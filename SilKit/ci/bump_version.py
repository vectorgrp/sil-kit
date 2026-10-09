#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

# Bumps the SIL Kit version: patches SilKitVersionMacros.h, archives
# docs/changelog/versions/latest.md as <current-version>.md, lists it in
# docs/changelog/overview.rst and resets latest.md.
# See docs/development/release.md.

import argparse
import datetime
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
HEADER = ROOT / "SilKit/include/silkit/capi/SilKitVersionMacros.h"
VERSIONS_DIR = ROOT / "docs/changelog/versions"
LATEST = VERSIONS_DIR / "latest.md"
OVERVIEW = ROOT / "docs/changelog/overview.rst"
COMPONENTS = ("MAJOR", "MINOR", "PATCH")


def die(message):
    sys.exit("error: " + message)


def read(path):
    with open(path, encoding="utf-8", newline="") as f:
        text = f.read()
    return text.replace("\r\n", "\n"), "\r\n" if "\r\n" in text else "\n"


def write(path, text, eol, dry_run):
    print("{}write {}".format("[dry-run] " if dry_run else "", path.relative_to(ROOT)))
    if not dry_run:
        with open(path, "w", encoding="utf-8", newline="") as f:
            f.write(text.replace("\n", eol))


def main():
    parser = argparse.ArgumentParser(description="Bump the SIL Kit version and rotate the changelog.")
    parser.add_argument("version", help="new version, e.g. 5.0.9")
    parser.add_argument("--date", type=datetime.date.fromisoformat, default=datetime.date.today(),
                        help="release date of the current version, YYYY-MM-DD (default: today)")
    parser.add_argument("-n", "--dry-run", action="store_true", help="only print the files that would be written")
    args = parser.parse_args()

    match = re.fullmatch(r"(\d+)\.(\d+)\.(\d+)", args.version)
    if not match:
        die("version must be MAJOR.MINOR.PATCH")
    new = tuple(int(x) for x in match.groups())

    header, header_eol = read(HEADER)
    current = []
    for component in COMPONENTS:
        match = re.search(r"^#define SILKIT_VERSION_{} (\d+)$".format(component), header, re.M)
        if not match:
            die("no SILKIT_VERSION_{} in {}".format(component, HEADER.relative_to(ROOT)))
        current.append(int(match.group(1)))
    current = tuple(current)
    if new <= current:
        die("{} is not newer than {}".format(args.version, ".".join(map(str, current))))

    current_name = ".".join(map(str, current))
    new_name = ".".join(map(str, new))

    archive = VERSIONS_DIR / (current_name + ".md")
    if archive.exists():
        die("{} already exists".format(archive.relative_to(ROOT)))

    latest, latest_eol = read(LATEST)
    latest, count = re.subn(r"^# \[[^\]]*\] - .*$", "# [{}] - {}".format(current_name, args.date), latest,
                            count=1, flags=re.M)
    if count == 0:
        die("no '# [x.y.z] - UNRELEASED' heading in {}".format(LATEST.relative_to(ROOT)))

    overview, overview_eol = read(OVERVIEW)
    anchor = re.search(r"^( *)versions/latest\.md$", overview, re.M)
    if not anchor:
        die("no 'versions/latest.md' entry in {}".format(OVERVIEW.relative_to(ROOT)))
    overview = "{}\n{}versions/{}.md{}".format(overview[:anchor.end()], anchor.group(1), current_name,
                                               overview[anchor.end():])

    for component, value in zip(COMPONENTS, new):
        header = re.sub(r"^(#define SILKIT_VERSION_{} )\d+$".format(component), r"\g<1>{}".format(value), header,
                        flags=re.M)

    write(HEADER, header, header_eol, args.dry_run)
    write(archive, latest, latest_eol, args.dry_run)
    write(LATEST, "# [{}] - UNRELEASED\n\n> This changelog entry is still empty.\n".format(new_name), latest_eol,
          args.dry_run)
    write(OVERVIEW, overview, overview_eol, args.dry_run)


if __name__ == "__main__":
    main()
