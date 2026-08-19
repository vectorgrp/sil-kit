#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

"""Generate an SPDX 2.3 SBOM for SIL Kit.

Software composition scanners cannot see this project's dependencies: five of them are git
submodules that carry nothing but a gitlink SHA, and rapidyaml is a vendored amalgamation with no
package metadata at all. The inventory is therefore declared by hand in
ThirdParty/third-party-components.json and rendered into SPDX by this script.

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

TOOL_NAME = "silkit-generate-sbom"
TOOL_VERSION = "1.0"

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_METADATA = REPO_ROOT / "ThirdParty" / "third-party-components.json"
DEFAULT_OUTPUT = REPO_ROOT / "SilKit.spdx.json"
NOTICE_FILE = REPO_ROOT / "ThirdParty" / "LICENSES.rst"

SILKIT_REPOSITORY = "https://github.com/vectorgrp/sil-kit"
SILKIT_SUPPLIER = "Organization: Vector Informatik GmbH"
SILKIT_LICENSE = "MIT"
SILKIT_COPYRIGHT = "Copyright (c) Vector Informatik GmbH"

# The SPDX document is embedded in reproducible builds, so nothing in it may vary between two
# builds of the same source tree. The namespace is therefore derived from the content rather than
# being a random UUID, and the timestamp falls back to SOURCE_DATE_EPOCH.
NAMESPACE_SEED = "https://github.com/vectorgrp/sil-kit/spdx"

# Artifacts a component can be part of. Keep in sync with 'shipsIn' in the metadata file.
ARTIFACT_LIBRARY = "SilKit"
ARTIFACT_REGISTRY = "sil-kit-registry"


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

    if metadata.get("schemaVersion") != 1:
        die(1, "Unsupported schemaVersion {} in {}", metadata.get("schemaVersion"), path)

    components = metadata.get("components")
    if not components:
        die(1, "{} declares no components", path)

    return components


def selected_components(components, withDashboard, withTests):
    """The components that end up in a released artifact for this build configuration."""
    enabled = {"SILKIT_BUILD_DASHBOARD": withDashboard, "SILKIT_BUILD_TESTS": withTests}
    selected = []
    for component in components:
        # Test-only dependencies never reach a released artifact.
        if not component["shipsIn"]:
            continue
        guard = component.get("cmakeGuard")
        if guard is not None and not enabled.get(guard, False):
            continue
        selected.append(component)
    return selected


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


def build_document(components, version, gitHash, withDashboard, withTests, useSystemLibraries,
                   created):
    shipped = selected_components(components, withDashboard, withTests)

    configKey = "dashboard={};systemLibs={}".format(int(bool(withDashboard)),
                                                    int(bool(useSystemLibraries)))

    if gitHash and gitHash != "UNKNOWN":
        silkitDownload = "git+{}.git@{}".format(SILKIT_REPOSITORY, gitHash)
    else:
        silkitDownload = "git+{}.git@v{}".format(SILKIT_REPOSITORY, version)

    rootId = spdx_id("SilKit")
    libraryId = spdx_id("Artifact", "SilKit-library")
    registryId = spdx_id("Artifact", "sil-kit-registry")

    packages = [
        silkit_package(
            rootId,
            "SilKit",
            version,
            "Vector SIL Kit distribution: the SIL Kit library and its utility tools.",
            silkitDownload,
            "pkg:github/vectorgrp/sil-kit@v{}".format(version),
        ),
        silkit_package(
            libraryId,
            "SilKit-library",
            version,
            "The SIL Kit shared library (SilKit.dll / libSilKit.so).",
            silkitDownload,
            None,
        ),
    ]

    relationships = [
        relationship("SPDXRef-DOCUMENT", "DESCRIBES", rootId),
        relationship(rootId, "CONTAINS", libraryId),
    ]

    if withDashboard:
        packages.append(
            silkit_package(
                registryId,
                "sil-kit-registry",
                version,
                "The SIL Kit registry utility, which carries the dashboard client.",
                silkitDownload,
                None,
            )
        )
        relationships.append(relationship(rootId, "CONTAINS", registryId))

    artifactIds = {ARTIFACT_LIBRARY: libraryId, ARTIFACT_REGISTRY: registryId}

    for component in shipped:
        packages.append(component_package(component))
        componentId = spdx_id("Package", component["id"])

        containedBy = component.get("containedBy")
        if containedBy:
            # A component bundled inside another one (c4core inside the rapidyaml amalgamation)
            # hangs off its container, not off the artifact.
            relationships.append(
                relationship(spdx_id("Package", containedBy), "CONTAINS", componentId)
            )
            continue

        for artifact in component["shipsIn"]:
            artifactId = artifactIds.get(artifact)
            if artifactId is None:
                die(1, "Component '{}' ships in unknown artifact '{}'", component["id"], artifact)
            relationships.append(relationship(artifactId, "STATIC_LINK", componentId))

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
            "Generated by SilKit/ci/generate_sbom.py from ThirdParty/third-party-components.json. "
            "Third party components are declared by hand because they are vendored as git "
            "submodules and as a source amalgamation, which no software composition scanner can "
            "resolve. Build configuration: {}.".format(configKey)
        ),
        "packages": packages,
        "relationships": relationships,
    }

    return document


def serialize(document):
    return json.dumps(document, indent=2, ensure_ascii=False) + "\n"


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


def check_metadata(components):
    """Verify the hand-maintained metadata against the tree. Returns a list of problems."""
    problems = []

    try:
        notices = NOTICE_FILE.read_text(encoding="utf-8", errors="replace")
    except OSError:
        notices = None
        problems.append("cannot read {}".format(NOTICE_FILE))

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

        noticeName = component.get("noticeName")
        if notices is not None and noticeName and noticeName not in notices:
            problems.append(
                "{}: '{}' is not mentioned in ThirdParty/LICENSES.rst. Every component must also "
                "appear in the third party notice file.".format(cid, noticeName)
            )

    return problems


# ---------------------------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------------------------


def do_check(components, output, args):
    problems = check_metadata(components)

    if not output.exists():
        problems.append(
            "{} does not exist. Generate it with: python3 SilKit/ci/generate_sbom.py".format(output)
        )
        for problem in problems:
            warn("{}", problem)
        die(1, "SBOM check failed with {} problem(s)", len(problems))

    existing = output.read_text(encoding="utf-8")

    # Compare content only: reuse the recorded creation timestamp so that an unchanged SBOM does
    # not go stale simply because time passed.
    created = None
    try:
        created = json.loads(existing)["creationInfo"]["created"]
    except (json.JSONDecodeError, KeyError, TypeError):
        problems.append("{} is not a readable SPDX document".format(output))

    expected = serialize(
        build_document(
            components,
            args.version,
            None,
            withDashboard=True,
            withTests=False,
            useSystemLibraries=False,
            created=created,
        )
    )

    if expected != existing:
        diff = difflib.unified_diff(
            existing.splitlines(keepends=True),
            expected.splitlines(keepends=True),
            fromfile=str(output) + " (committed)",
            tofile=str(output) + " (expected)",
        )
        sys.stdout.writelines(diff)
        problems.append(
            "{} is out of date. Regenerate it with: python3 SilKit/ci/generate_sbom.py".format(
                output
            )
        )

    if problems:
        for problem in problems:
            warn("{}", problem)
        die(1, "SBOM check failed with {} problem(s)", len(problems))

    info("SBOM is up to date and consistent with the tree")
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
    # The canonical SBOM committed to the repository is the default configuration, so the
    # dependency-affecting options default to their CMake defaults and are turned off explicitly.
    parser.add_argument("--without-dashboard", dest="with_dashboard", action="store_false",
                        help="the build has SILKIT_BUILD_DASHBOARD=OFF")
    parser.add_argument("--with-tests", action="store_true",
                        help="the build has SILKIT_BUILD_TESTS=ON")
    parser.add_argument("--use-system-libraries", action="store_true",
                        help="the build has SILKIT_USE_SYSTEM_LIBRARIES=ON")
    parser.add_argument("--check", action="store_true",
                        help="verify the committed SBOM and the metadata instead of writing; "
                             "exits non-zero when either is stale")
    args = parser.parse_args()

    if args.version is None:
        args.version = read_version()

    components = load_metadata(args.metadata)
    output = Path(args.output)

    if args.check:
        return do_check(components, output, args)

    if args.use_system_libraries:
        warn(
            "SILKIT_USE_SYSTEM_LIBRARIES is ON: the versions in this SBOM are the ones vendored "
            "in ThirdParty/, not the system libraries actually linked."
        )

    document = build_document(
        components,
        args.version,
        args.git_hash,
        withDashboard=args.with_dashboard,
        withTests=args.with_tests,
        useSystemLibraries=args.use_system_libraries,
        created=None,
    )

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(serialize(document), encoding="utf-8")
    info("Wrote {} ({} packages)", output, len(document["packages"]))
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
