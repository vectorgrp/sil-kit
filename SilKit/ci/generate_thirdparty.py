#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

"""Render the SIL Kit third party inventory into its published forms.

Software composition scanners cannot see this project's dependencies: five of them are git
submodules that carry nothing but a gitlink SHA, and rapidyaml is a vendored amalgamation with no
package metadata at all. The inventory is therefore declared by hand in
ThirdParty/third-party-components.json, and everything downstream is generated from it:

  spdx     SilKit.spdx.json               an SPDX 2.3 software bill of materials
  notices  ThirdParty/LICENSES.rst        the third party notice file
  docs     docs/licenses/thirdparty.rst   the same content for the documentation

The SBOM and the notice file are not interchangeable. The SBOM is an inventory and records license
identifiers; MIT, BSD, BSL-1.0, Apache-2.0 and OFL-1.1 all additionally require the license text
itself to travel with the distribution, which is what the notice file is for.

The script is deliberately restricted to the Python standard library: it runs from CMake during
ordinary developer builds on Windows, macOS, MinGW and the cross-compilation presets, where no
third party Python packages are installed.

See docs/development/sbom.rst.
"""

import argparse
import datetime
import difflib
import json
import os
import re
import subprocess
import sys
import uuid

from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from ci_utils import info, warn, die  # noqa: E402

TOOL_NAME = "silkit-generate-thirdparty"
TOOL_VERSION = "1.1"

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_METADATA = REPO_ROOT / "ThirdParty" / "third-party-components.json"
DEFAULT_OUTPUT = REPO_ROOT / "SilKit.spdx.json"
NOTICE_FILE = REPO_ROOT / "ThirdParty" / "LICENSES.rst"
DOCS_FILE = REPO_ROOT / "docs" / "licenses" / "thirdparty.rst"

GENERATED_BY = "generated from ThirdParty/third-party-components.json by SilKit/ci/{}".format(
    Path(__file__).name
)

# How a component reaches the user, for the 'Part of' column. Relationship types ending in _OF are
# build-time only: the tool produces the artifact without shipping any of its own code, so it needs
# no license notice.
ARTIFACT_LABELS = {
    "SilKit-library": "SIL Kit library",
    "sil-kit-registry": "sil-kit-registry",
    "SilKit-Documentation": "Documentation",
    "SilKit-Source": "Source distribution",
}

SILKIT_REPOSITORY = "https://github.com/vectorgrp/sil-kit"
SILKIT_SUPPLIER = "Organization: Vector Informatik GmbH"
SILKIT_LICENSE = "MIT"
SILKIT_COPYRIGHT = "Copyright (c) Vector Informatik GmbH"

# The SPDX document is embedded in reproducible builds, so nothing in it may vary between two
# builds of the same source tree. The namespace is therefore derived from the content rather than
# being a random UUID, and the timestamp falls back to SOURCE_DATE_EPOCH.
NAMESPACE_SEED = "https://github.com/vectorgrp/sil-kit/spdx"

# The CMake options that decide which release artifacts exist. The canonical SBOM committed to the
# repository describes a full release, so all of them default to on.
BUILD_OPTIONS = (
    "SILKIT_BUILD_UTILITIES",
    "SILKIT_BUILD_DOCS",
    "SILKIT_INSTALL_SOURCE",
    "SILKIT_BUILD_DASHBOARD",
    "SILKIT_USE_SYSTEM_LIBRARIES",
)

FULL_RELEASE = {
    "SILKIT_BUILD_UTILITIES": True,
    "SILKIT_BUILD_DOCS": True,
    "SILKIT_INSTALL_SOURCE": True,
    "SILKIT_BUILD_DASHBOARD": True,
    "SILKIT_USE_SYSTEM_LIBRARIES": False,
}


# ---------------------------------------------------------------------------------------------
# Metadata
# ---------------------------------------------------------------------------------------------


def load_metadata(path):
    try:
        with open(path, "r", encoding="utf-8") as f:
            metadata = json.load(f)
    except OSError as e:
        die(1, "Cannot read the third party metadata {}: {}", path, e)
    except json.JSONDecodeError as e:
        die(1, "{} is not valid JSON: {}", path, e)

    if metadata.get("schemaVersion") != 2:
        die(1, "Unsupported schemaVersion {} in {}", metadata.get("schemaVersion"), path)

    components = metadata.get("components")
    if not components:
        die(1, "{} declares no components", path)

    artifacts = metadata.get("artifacts")
    if not artifacts:
        die(1, "{} declares no artifacts", path)

    return artifacts, components


def enabled_artifacts(artifacts, options):
    """The release artifacts this build configuration actually produces."""
    return [a for a in artifacts if a["guard"] is None or options.get(a["guard"], False)]


def resolve(artifacts, components, options):
    """Work out which artifacts are built and which components reach them.

    Returns the enabled artifacts, the components to describe, and the (component, artifact,
    relationship) triples between them. A component is described when at least one of its 'partOf'
    entries resolves to an enabled artifact, or when it is bundled inside a component that is.
    """
    enabled = enabled_artifacts(artifacts, options)
    enabledIds = set(a["id"] for a in enabled)

    edges = []
    selectedIds = set()
    for component in components:
        for entry in component.get("partOf", []):
            if entry["artifact"] not in enabledIds:
                continue
            guard = entry.get("guard")
            if guard is not None and not options.get(guard, False):
                continue
            edges.append((component, entry["artifact"], entry["relationship"]))
            selectedIds.add(component["id"])

    # Bundled components ride along with their container, however that container got in.
    changed = True
    while changed:
        changed = False
        for component in components:
            container = component.get("containedBy")
            if container in selectedIds and component["id"] not in selectedIds:
                selectedIds.add(component["id"])
                changed = True

    selected = [c for c in components if c["id"] in selectedIds]
    return enabled, selected, edges


# ---------------------------------------------------------------------------------------------
# SPDX document
# ---------------------------------------------------------------------------------------------


def spdx_id(*parts):
    identifier = "-".join(str(p) for p in parts)
    # SPDX identifiers allow letters, digits, '.' and '-' only.
    return "SPDXRef-" + re.sub(r"[^A-Za-z0-9.\-]", "-", identifier)


def creation_timestamp(created):
    if created:
        return created

    sourceDateEpoch = os.environ.get("SOURCE_DATE_EPOCH")
    if sourceDateEpoch:
        try:
            stamp = datetime.datetime.fromtimestamp(int(sourceDateEpoch), datetime.timezone.utc)
            return stamp.strftime("%Y-%m-%dT%H:%M:%SZ")
        except ValueError:
            warn("Ignoring malformed SOURCE_DATE_EPOCH {}", sourceDateEpoch)

    return datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def document_namespace(version, configKey):
    seed = "{}/{}/{}".format(NAMESPACE_SEED, version, configKey)
    return "{}/SilKit-{}-{}".format(NAMESPACE_SEED, version, uuid.uuid5(uuid.NAMESPACE_URL, seed))


def external_refs(component):
    refs = []
    if component.get("purl"):
        refs.append(
            {
                "referenceCategory": "PACKAGE-MANAGER",
                "referenceType": "purl",
                "referenceLocator": component["purl"],
            }
        )
    if component.get("cpe23"):
        refs.append(
            {
                "referenceCategory": "SECURITY",
                "referenceType": "cpe23Type",
                "referenceLocator": component["cpe23"],
            }
        )
    return refs


def download_location(component):
    repository = component.get("repository")
    if not repository:
        return "NOASSERTION"
    if component.get("commit"):
        return "git+{}@{}".format(repository, component["commit"])
    return "git+{}".format(repository)


def component_package(component):
    package = {
        "SPDXID": spdx_id("Package", component["id"]),
        "name": component["name"],
        "versionInfo": component["version"],
        "supplier": component["supplier"],
        "originator": component["supplier"],
        "downloadLocation": download_location(component),
        "homepage": component.get("homepage", "NOASSERTION"),
        "filesAnalyzed": False,
        "licenseConcluded": component["licenseConcluded"],
        "licenseDeclared": component["licenseDeclared"],
        "copyrightText": component["copyrightText"],
    }

    refs = external_refs(component)
    if refs:
        package["externalRefs"] = refs

    comment = component.get("comment")
    if comment:
        package["comment"] = comment

    return package


def silkit_package(spdxid, name, version, description, downloadLocation, purl):
    package = {
        "SPDXID": spdxid,
        "name": name,
        "versionInfo": version,
        "supplier": SILKIT_SUPPLIER,
        "originator": SILKIT_SUPPLIER,
        "downloadLocation": downloadLocation,
        "homepage": SILKIT_REPOSITORY,
        "filesAnalyzed": False,
        "licenseConcluded": SILKIT_LICENSE,
        "licenseDeclared": SILKIT_LICENSE,
        "copyrightText": SILKIT_COPYRIGHT,
        "description": description,
    }
    if purl:
        package["externalRefs"] = [
            {
                "referenceCategory": "PACKAGE-MANAGER",
                "referenceType": "purl",
                "referenceLocator": purl,
            }
        ]
    return package


def relationship(element, relationshipType, related):
    return {
        "spdxElementId": element,
        "relationshipType": relationshipType,
        "relatedSpdxElement": related,
    }


def build_document(artifacts, components, version, gitHash, options, created):
    enabled, selected, edges = resolve(artifacts, components, options)

    configKey = ",".join(a["id"] for a in enabled)
    if options.get("SILKIT_USE_SYSTEM_LIBRARIES"):
        configKey += ";systemLibs"

    if gitHash and gitHash != "UNKNOWN":
        silkitDownload = "git+{}.git@{}".format(SILKIT_REPOSITORY, gitHash)
    else:
        silkitDownload = "git+{}.git@v{}".format(SILKIT_REPOSITORY, version)

    rootId = spdx_id("SilKit")

    packages = [
        silkit_package(
            rootId,
            "SilKit",
            version,
            "Vector SIL Kit release: the SIL Kit library, its utility tools, the documentation "
            "and the source distribution.",
            silkitDownload,
            "pkg:github/vectorgrp/sil-kit@v{}".format(version),
        )
    ]

    relationships = [relationship("SPDXRef-DOCUMENT", "DESCRIBES", rootId)]

    for artifact in enabled:
        artifactId = spdx_id("Artifact", artifact["id"])
        packages.append(
            silkit_package(
                artifactId, artifact["id"], version, artifact["description"], silkitDownload, None
            )
        )
        relationships.append(relationship(rootId, "CONTAINS", artifactId))

    for component in selected:
        packages.append(component_package(component))

    for component, artifactName, relationshipType in edges:
        artifactId = spdx_id("Artifact", artifactName)
        componentId = spdx_id("Package", component["id"])
        # SPDX relationship types ending in _OF or _BY read "component is a X of artifact", so the
        # component is the subject. CONTAINS and STATIC_LINK read the other way round.
        if relationshipType.endswith("_OF") or relationshipType.endswith("_BY"):
            relationships.append(relationship(componentId, relationshipType, artifactId))
        else:
            relationships.append(relationship(artifactId, relationshipType, componentId))

    # A component bundled inside another one (c4core inside the rapidyaml amalgamation, the
    # webfonts inside sphinx-rtd-theme) hangs off its container, not off the artifact.
    for component in selected:
        container = component.get("containedBy")
        if container:
            relationships.append(
                relationship(
                    spdx_id("Package", container),
                    "CONTAINS",
                    spdx_id("Package", component["id"]),
                )
            )

    document = {
        "spdxVersion": "SPDX-2.3",
        "dataLicense": "CC0-1.0",
        "SPDXID": "SPDXRef-DOCUMENT",
        "name": "SilKit-{}".format(version),
        "documentNamespace": document_namespace(version, configKey),
        "creationInfo": {
            "created": creation_timestamp(created),
            "creators": [
                SILKIT_SUPPLIER,
                "Tool: {}-{}".format(TOOL_NAME, TOOL_VERSION),
            ],
            "licenseListVersion": "3.21",
        },
        "comment": (
            "Generated by SilKit/ci/generate_thirdparty.py from "
            "ThirdParty/third-party-components.json. "
            "Third party components are declared by hand because they are vendored as git "
            "submodules and as a source amalgamation, which no software composition scanner can "
            "resolve. Artifacts covered: {}.".format(configKey)
        ),
        "packages": packages,
        "relationships": relationships,
    }

    return document


def serialize(document):
    return json.dumps(document, indent=2, ensure_ascii=False) + "\n"


# ---------------------------------------------------------------------------------------------
# Notice file and documentation
# ---------------------------------------------------------------------------------------------


def needs_notice(component):
    """Whether redistributing this component obliges us to reproduce its license text.

    Attribution attaches to distribution. A component that is only BUILD_TOOL_OF an artifact --
    breathe and myst-parser -- produces output without any of its own code shipping, so it is
    listed for completeness but carries no notice.
    """
    return any(
        entry["relationship"] in ("CONTAINS", "STATIC_LINK")
        for entry in component.get("partOf", [])
    ) or bool(component.get("containedBy"))


def part_of_label(component, components):
    """The 'Part of' cell: where in the release a reader actually meets this component."""
    container = component.get("containedBy")
    if container:
        byId = dict((c["id"], c) for c in components)
        return "bundled in {}".format(byId[container]["name"])

    labels = []
    for entry in component.get("partOf", []):
        label = ARTIFACT_LABELS.get(entry["artifact"], entry["artifact"])
        if entry["relationship"].endswith("_OF"):
            label += " (build tool)"
        if label not in labels:
            labels.append(label)
    return ", ".join(labels) if labels else "not redistributed"


def rst_table(components):
    """A list-table rather than a csv-table: no quoting or comma-escaping hazards."""
    lines = [
        ".. list-table::",
        "   :header-rows: 1",
        "   :widths: 22 12 24 42",
        "",
        "   * - Component",
        "     - Version",
        "     - License",
        "     - Part of",
    ]
    for component in components:
        homepage = component.get("homepage")
        name = component["name"]
        cell = "`{} <{}>`_".format(name, homepage) if homepage else name
        lines.append("   * - {}".format(cell))
        lines.append("     - {}".format(component["version"]))
        lines.append("     - {}".format(component["licenseDeclared"]))
        lines.append("     - {}".format(part_of_label(component, components)))
    lines.append("")
    return "\n".join(lines)


def license_text(component):
    path = REPO_ROOT / component["licenseTextFile"]
    try:
        return path.read_text(encoding="utf-8").rstrip("\n")
    except OSError as e:
        die(1, "Cannot read the license text {} for {}: {}", path, component["id"], e)


def rst_license_sections(components, underline):
    """One verbatim license text per redistributed component, as a literal block."""
    blocks = []
    for component in components:
        if not needs_notice(component):
            continue
        title = component["name"]
        blocks.append("{}\n{}\n".format(title, underline * max(len(title), 3)))
        blocks.append("::\n")
        # Indent by three spaces to make it a literal block. The text itself is untouched.
        for line in license_text(component).split("\n"):
            blocks.append(("   " + line).rstrip())
        blocks.append("")
    return "\n".join(blocks)


def render_notices(components):
    """ThirdParty/LICENSES.rst -- the notice file that ships with the source distribution."""
    header = [
        "SIL Kit Third Party Libraries",
        "=============================",
        "",
        ".. NOTE: This file is {}.".format(GENERATED_BY),
        "   Do not edit it by hand; edit the metadata or the texts in ThirdParty/licenses/",
        "   and run: python3 SilKit/ci/generate_thirdparty.py",
        "",
        "The SIL Kit uses the third party software components listed below, which are governed by",
        "their respective licenses. The full and unmodified license of each redistributed component",
        "is printed after the table.",
        "",
        "A machine-readable inventory of the same components is available as an SPDX 2.3 document",
        "in SilKit.spdx.json.",
        "",
        "",
    ]
    body = rst_table(components) + "\n" + rst_license_sections(components, "=")
    return "\n".join(header) + body + "\n"


def render_docs(components):
    """docs/licenses/thirdparty.rst -- included by docs/licenses/license.rst."""
    header = [
        ".. NOTE: This file is {}.".format(GENERATED_BY),
        "   Do not edit it by hand; edit the metadata or the texts in ThirdParty/licenses/",
        "   and run: python3 SilKit/ci/generate_thirdparty.py",
        "",
        "The |ProductName| uses the third party software components listed below. The full and",
        "unmodified license of each redistributed component is printed after the table.",
        "",
        "Components marked as a build tool are needed to produce an artifact but do not ship any of",
        "their own code, so no license text is reproduced for them.",
        "",
        "",
    ]
    body = rst_table(components) + "\n" + rst_license_sections(components, "~")
    return "\n".join(header) + body + "\n"


# ---------------------------------------------------------------------------------------------
# Consistency checks
# ---------------------------------------------------------------------------------------------


def gitlink_sha(path):
    """The submodule commit recorded in HEAD.

    Deliberately uses 'git ls-tree' rather than 'git submodule status': it works without the
    submodules being checked out, and it avoids the tag names reported by 'git describe', which
    for googletest and spdlog are several releases behind the actual pin.
    """
    try:
        out = subprocess.check_output(
            ["git", "ls-tree", "HEAD", path], cwd=str(REPO_ROOT), stderr=subprocess.DEVNULL
        )
    except (subprocess.CalledProcessError, OSError):
        return None

    entry = out.decode("utf-8", "replace").split()
    # <mode> <type> <object>\t<file>
    if len(entry) < 3 or entry[1] != "commit":
        return None
    return entry[2]


def pinned_versions(requirementsPath):
    """Map the distribution names pinned in a requirements file to their versions."""
    pins = {}
    try:
        text = (REPO_ROOT / requirementsPath).read_text(encoding="utf-8")
    except OSError:
        return None

    for line in text.splitlines():
        line = line.split("#", 1)[0].strip()
        match = re.match(r"^([A-Za-z0-9._-]+)\s*==\s*([^\s;]+)$", line)
        if match:
            pins[match.group(1).lower().replace("_", "-")] = match.group(2)
    return pins


def check_metadata(artifacts, components):
    """Verify the hand-maintained metadata against the tree. Returns a list of problems."""
    problems = []
    artifactIds = set(a["id"] for a in artifacts)
    componentIds = set(c["id"] for c in components)
    requirementCache = {}

    for component in components:
        for entry in component.get("partOf", []):
            if entry["artifact"] not in artifactIds:
                problems.append(
                    "{}: partOf references unknown artifact '{}'".format(
                        component["id"], entry["artifact"]
                    )
                )
        container = component.get("containedBy")
        if container and container not in componentIds:
            problems.append(
                "{}: containedBy references unknown component '{}'".format(
                    component["id"], container
                )
            )

        # Keep the documentation toolchain in step with the requirements file the release build
        # installs from, so bumping a pin there cannot silently invalidate the SBOM.
        pinnedIn = component.get("pinnedIn")
        if pinnedIn:
            if pinnedIn not in requirementCache:
                requirementCache[pinnedIn] = pinned_versions(pinnedIn)
            pins = requirementCache[pinnedIn]
            if pins is None:
                problems.append("{}: cannot read {}".format(component["id"], pinnedIn))
            else:
                key = component["name"].lower().replace("_", "-")
                if key not in pins:
                    problems.append(
                        "{}: '{}' is not pinned in {}".format(component["id"], component["name"],
                                                              pinnedIn)
                    )
                elif pins[key] != component["version"]:
                    problems.append(
                        "{}: metadata says {} but {} pins {}. Update the version in "
                        "ThirdParty/third-party-components.json.".format(
                            component["id"], component["version"], pinnedIn, pins[key]
                        )
                    )


    for component in components:
        cid = component["id"]

        if component["vendoring"] == "submodule":
            expected = component.get("commit")
            actual = gitlink_sha(component["path"])
            if actual is None:
                warn("Cannot read the gitlink for {}; skipping its commit check", component["path"])
            elif actual != expected:
                problems.append(
                    "{}: metadata pins commit {} but the tree records {}. Update 'version' and "
                    "'commit' in ThirdParty/third-party-components.json.".format(
                        cid, expected, actual
                    )
                )

        amalgamation = component.get("amalgamationSource")
        if amalgamation:
            path = REPO_ROOT / amalgamation
            if not path.exists():
                problems.append(
                    "{}: amalgamationSource {} does not exist. The vendored copy was most likely "
                    "updated without updating 'version'.".format(cid, amalgamation)
                )

        licenseFile = component.get("licenseFile")
        if licenseFile and not (REPO_ROOT / licenseFile).exists():
            # Submodules may simply not be checked out; only complain for tracked files.
            if component["vendoring"] != "submodule":
                problems.append("{}: licenseFile {} does not exist".format(cid, licenseFile))

        # The notice files are generated, so they cannot drift from the metadata. What can go
        # wrong is a redistributed component without a license text to reproduce.
        licenseTextFile = component.get("licenseTextFile")
        if needs_notice(component):
            if not licenseTextFile:
                problems.append(
                    "{}: is redistributed but has no licenseTextFile. Add the upstream license "
                    "text under ThirdParty/licenses/ and reference it.".format(cid)
                )
            elif not (REPO_ROOT / licenseTextFile).exists():
                problems.append(
                    "{}: licenseTextFile {} does not exist".format(cid, licenseTextFile)
                )
        elif licenseTextFile:
            problems.append(
                "{}: has a licenseTextFile but is not redistributed; no notice is required "
                "for a build-time-only component.".format(cid)
            )

    return problems


# ---------------------------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------------------------


REGENERATE_HINT = "python3 SilKit/ci/generate_thirdparty.py"


def canonical_outputs(artifacts, components, version, spdxOutput):
    """The three generated files, always for the full release configuration.

    The SPDX document reuses the creation timestamp already recorded in the committed file, so that
    an unchanged SBOM does not go stale simply because time passed.
    """
    created = None
    if spdxOutput.exists():
        try:
            created = json.loads(spdxOutput.read_text(encoding="utf-8"))["creationInfo"]["created"]
        except (json.JSONDecodeError, KeyError, TypeError, OSError):
            created = None

    return [
        ("spdx", spdxOutput,
         serialize(build_document(artifacts, components, version, None, FULL_RELEASE, created))),
        ("notices", NOTICE_FILE, render_notices(components)),
        ("docs", DOCS_FILE, render_docs(components)),
    ]


def do_check(artifacts, components, output, args):
    problems = check_metadata(artifacts, components)

    for name, path, expected in canonical_outputs(artifacts, components, args.version, output):
        if not path.exists():
            problems.append(
                "{} does not exist. Generate it with: {}".format(path, REGENERATE_HINT)
            )
            continue

        existing = path.read_text(encoding="utf-8")
        if existing == expected:
            continue

        sys.stdout.writelines(
            difflib.unified_diff(
                existing.splitlines(keepends=True),
                expected.splitlines(keepends=True),
                fromfile="{} (committed)".format(path),
                tofile="{} (expected)".format(path),
            )
        )
        problems.append(
            "{} ({}) is out of date. Regenerate it with: {}".format(path, name, REGENERATE_HINT)
        )

    if problems:
        for problem in problems:
            warn("{}", problem)
        die(1, "Third party check failed with {} problem(s)", len(problems))

    info("SBOM, notice file and documentation are up to date and consistent with the tree")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--metadata", default=str(DEFAULT_METADATA),
                        help="third party component metadata (default: %(default)s)")
    parser.add_argument("--output", default=str(DEFAULT_OUTPUT),
                        help="where to write the SPDX document (default: %(default)s)")
    parser.add_argument("--version", default=None,
                        help="the SIL Kit version; read from SilKitVersion.cmake if omitted")
    parser.add_argument("--git-hash", default=None,
                        help="the commit the artifacts were built from. Omit for the canonical "
                             "SBOM committed to the repository, which must not change on every "
                             "commit")
    # The canonical SBOM describes a full release, so every artifact-producing option defaults to
    # on and is turned off explicitly for a narrower build.
    parser.add_argument("--without-utilities", dest="SILKIT_BUILD_UTILITIES",
                        action="store_false", help="the build has SILKIT_BUILD_UTILITIES=OFF")
    parser.add_argument("--without-docs", dest="SILKIT_BUILD_DOCS",
                        action="store_false", help="the build has SILKIT_BUILD_DOCS=OFF")
    parser.add_argument("--without-source", dest="SILKIT_INSTALL_SOURCE",
                        action="store_false", help="the build has SILKIT_INSTALL_SOURCE=OFF")
    parser.add_argument("--without-dashboard", dest="SILKIT_BUILD_DASHBOARD",
                        action="store_false", help="the build has SILKIT_BUILD_DASHBOARD=OFF")
    parser.add_argument("--use-system-libraries", dest="SILKIT_USE_SYSTEM_LIBRARIES",
                        action="store_true", help="the build has SILKIT_USE_SYSTEM_LIBRARIES=ON")
    parser.add_argument("--emit", choices=("spdx", "notices", "docs", "all"), default="all",
                        help="which outputs to write (default: %(default)s). The notice file and "
                             "the documentation always describe a full release, so the build "
                             "configuration options apply to the SBOM only")
    parser.add_argument("--check", action="store_true",
                        help="verify the generated files and the metadata instead of writing; "
                             "exits non-zero when any of them is stale")
    args = parser.parse_args()

    if args.version is None:
        args.version = read_version()

    artifacts, components = load_metadata(args.metadata)
    output = Path(args.output)

    if args.check:
        return do_check(artifacts, components, output, args)

    options = dict((name, getattr(args, name)) for name in BUILD_OPTIONS)

    if options["SILKIT_USE_SYSTEM_LIBRARIES"]:
        warn(
            "SILKIT_USE_SYSTEM_LIBRARIES is ON: the versions in this SBOM are the ones vendored "
            "in ThirdParty/, not the system libraries actually linked."
        )

    if args.emit in ("spdx", "all"):
        document = build_document(artifacts, components, args.version, args.git_hash, options, None)
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(serialize(document), encoding="utf-8")
        info("Wrote {} ({} packages)", output, len(document["packages"]))

    # The notice file and the documentation are not per-build: they must list everything a release
    # redistributes, whatever this particular build happens to enable.
    if args.emit in ("notices", "all"):
        NOTICE_FILE.write_text(render_notices(components), encoding="utf-8")
        info("Wrote {}", NOTICE_FILE)

    if args.emit in ("docs", "all"):
        DOCS_FILE.write_text(render_docs(components), encoding="utf-8")
        info("Wrote {}", DOCS_FILE)

    return 0


def read_version():
    """Read the version from SilKitVersion.cmake, the single source of truth for it."""
    versionCmake = REPO_ROOT / "SilKit" / "cmake" / "SilKitVersion.cmake"
    try:
        text = versionCmake.read_text(encoding="utf-8")
    except OSError as e:
        die(1, "Cannot read {}: {}", versionCmake, e)

    parts = []
    for name in ("MAJOR", "MINOR", "PATCH"):
        match = re.search(r"set\(SILKIT_VERSION_" + name + r"\s+(\d+)\)", text)
        if not match:
            die(1, "Cannot find SILKIT_VERSION_{} in {}", name, versionCmake)
        parts.append(match.group(1))

    version = ".".join(parts)

    suffix = re.search(r'set\(SILKIT_VERSION_SUFFIX\s+"([^"]*)"\)', text)
    if suffix and suffix.group(1):
        version += "-" + suffix.group(1)

    return version


if __name__ == "__main__":
    sys.exit(main())
