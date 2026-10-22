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
    # Reading in text mode turns every line ending into "\n". f.newlines records which ones the
    # file really used: None if it has no line break, a string for one kind, a tuple if mixed.
    with open(path, encoding="utf-8") as f:
        text = f.read()
        newlines = f.newlines

    if newlines is None:
        newlines = ()
    elif isinstance(newlines, str):
        newlines = (newlines,)

    if "\r\n" in newlines:
        eol = "\r\n"
    else:
        eol = "\n"
    return text, eol


def write(path, text, eol, dry_run):
    print(f"{'[dry-run] ' if dry_run else ''}write {path.relative_to(ROOT)}")
    if not dry_run:
        with open(path, "w", encoding="utf-8", newline=eol) as f:
            f.write(text)


def main():
    parser = argparse.ArgumentParser(description="Bump the SIL Kit version and rotate the changelog.")
    parser.add_argument("version", help="new version, e.g. 5.0.9")
    parser.add_argument("--date", type=datetime.date.fromisoformat,
                        help="release date of the current version, YYYY-MM-DD (default: the date already in "
                             "latest.md, or today if it is UNRELEASED)")
    parser.add_argument("-n", "--dry-run", action="store_true", help="only print the files that would be written")
    args = parser.parse_args()

    match = re.fullmatch(r"(\d+)\.(\d+)\.(\d+)", args.version)
    if not match:
        die("version must be MAJOR.MINOR.PATCH")
    new = tuple(int(x) for x in match.groups())

    header, header_eol = read(HEADER)
    current = []
    for component in COMPONENTS:
        match = re.search(rf"^#define SILKIT_VERSION_{component} (\d+)$", header, re.M)
        if not match:
            die(f"no SILKIT_VERSION_{component} in {HEADER.relative_to(ROOT)}")
        current.append(int(match.group(1)))
    current = tuple(current)
    if new <= current:
        die(f"{args.version} is not newer than {'.'.join(map(str, current))}")

    current_name = ".".join(map(str, current))
    new_name = ".".join(map(str, new))

    archive = VERSIONS_DIR / (current_name + ".md")
    if archive.exists():
        die(f"{archive.relative_to(ROOT)} already exists")

    latest, latest_eol = read(LATEST)
    heading = re.search(r"^# \[[^\]]*\] - (UNRELEASED|\d{4}-\d{2}-\d{2})$", latest, re.M)
    if not heading:
        die(f"no '# [x.y.z] - UNRELEASED' or '# [x.y.z] - YYYY-MM-DD' heading in {LATEST.relative_to(ROOT)}")
    date = args.date
    if date is None:
        date = datetime.date.today() if heading.group(1) == "UNRELEASED" else heading.group(1)
    latest = f"{latest[:heading.start()]}# [{current_name}] - {date}{latest[heading.end():]}"

    overview, overview_eol = read(OVERVIEW)
    anchor = re.search(r"^( *)versions/latest\.md$", overview, re.M)
    if not anchor:
        die(f"no 'versions/latest.md' entry in {OVERVIEW.relative_to(ROOT)}")
    overview = f"{overview[:anchor.end()]}\n{anchor.group(1)}versions/{current_name}.md{overview[anchor.end():]}"

    for component, value in zip(COMPONENTS, new):
        header = re.sub(rf"^(#define SILKIT_VERSION_{component} )\d+$", rf"\g<1>{value}", header, flags=re.M)

    write(HEADER, header, header_eol, args.dry_run)
    write(archive, latest, latest_eol, args.dry_run)
    write(LATEST, f"# [{new_name}] - UNRELEASED\n\n> This changelog entry is still empty.\n", latest_eol, args.dry_run)
    write(OVERVIEW, overview, overview_eol, args.dry_run)


if __name__ == "__main__":
    main()
