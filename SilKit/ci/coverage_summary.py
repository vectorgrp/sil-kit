#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

"""
Print a Markdown table of line coverage per component, comparing a unit-test-only
run against a run of unit and integration tests. Inputs are gcovr --json-summary files.
"""

from collections import defaultdict

import argparse
import json

TWO_LEVEL_DIRS = ("core", "services", "experimental", "Utilities")


def component_of(filename: str) -> str:
    parts = filename.split("/")
    if parts[:2] == ["SilKit", "source"]:
        parts = parts[2:]
    elif parts[:2] == ["SilKit", "include"]:
        return "include"
    if len(parts) > 2 and parts[0] in TWO_LEVEL_DIRS:
        return "/".join(parts[:2])
    return parts[0] if len(parts) > 1 else "(top level)"


def lines_per_component(summary_path: str):
    with open(summary_path, "r", encoding="utf-8") as f:
        summary = json.load(f)
    totals = defaultdict(lambda: [0, 0])
    for entry in summary["files"]:
        counts = totals[component_of(entry["filename"])]
        counts[0] += entry["line_covered"]
        counts[1] += entry["line_total"]
    return totals


def percent(covered: int, total: int) -> str:
    return f"{100.0 * covered / total:.1f}" if total else "-"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("unit_summary", help="gcovr --json-summary of the unit test run")
    parser.add_argument("full_summary", help="gcovr --json-summary of the unit and integration test run")
    args = parser.parse_args()

    unit = lines_per_component(args.unit_summary)
    full = lines_per_component(args.full_summary)

    print("## Line coverage per component")
    print()
    print("| Component | Lines | Unit tests % | Unit + integration tests % |")
    print("|---|---:|---:|---:|")
    unit_covered = full_covered = all_lines = 0
    for component in sorted(full):
        covered, total = full[component]
        covered_by_unit = unit[component][0] if component in unit else 0
        print(f"| {component} | {total} | {percent(covered_by_unit, total)} | {percent(covered, total)} |")
        unit_covered += covered_by_unit
        full_covered += covered
        all_lines += total
    print(f"| **Total** | {all_lines} | {percent(unit_covered, all_lines)} | {percent(full_covered, all_lines)} |")


if __name__ == "__main__":
    main()
