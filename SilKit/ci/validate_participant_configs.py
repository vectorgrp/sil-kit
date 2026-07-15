#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2026 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

"""
Validate SIL Kit participant configuration example files (JSON/YAML) against
SilKit/source/config/ParticipantConfiguration.schema.json.
"""

from pathlib import Path

import argparse
import json
import sys

import jsonschema
import yaml

sys.path.insert(0, str(Path(__file__).resolve().parent))
from ci_utils import info, warn, die

SCRIPT_DIR = Path(__file__).resolve().parent
REPO_ROOT = SCRIPT_DIR.parents[1]
SCHEMA_PATH = REPO_ROOT / "SilKit" / "source" / "config" / "ParticipantConfiguration.schema.json"


def find_config_files(repo_root: Path):
    config_dir = repo_root / "SilKit" / "source" / "config"

    files = []
    files += sorted(p for p in config_dir.glob("*.json") if p != SCHEMA_PATH)
    files += sorted(config_dir.glob("*.yaml"))
    files += sorted((repo_root / "Demos").rglob("*.silkit.yaml"))
    return files


def load_config(path: Path):
    with path.open("r", encoding="utf-8") as f:
        if path.suffix == ".json":
            return json.load(f)
        return yaml.safe_load(f)


def main():
    parser = argparse.ArgumentParser(
        description="Validate SIL Kit participant configuration example files against the JSON schema")
    parser.add_argument(
        "--root", type=Path, default=REPO_ROOT,
        help="Repository root (defaults to the checkout containing this script)")
    args = parser.parse_args()

    schema_path = args.root / "SilKit" / "source" / "config" / "ParticipantConfiguration.schema.json"
    with schema_path.open("r", encoding="utf-8") as f:
        schema = json.load(f)

    validator_cls = jsonschema.validators.validator_for(schema)
    validator_cls.check_schema(schema)
    validator = validator_cls(schema)

    files = find_config_files(args.root)
    if not files:
        die(1, "No participant configuration example files found to validate!")

    info("Validating {} participant configuration file(s) against {}", len(files), schema_path)

    failed = 0
    for path in files:
        try:
            instance = load_config(path)
        except Exception as ex:
            warn("{}: failed to parse ({})", path, ex)
            failed += 1
            continue

        errors = sorted(validator.iter_errors(instance), key=lambda e: [str(p) for p in e.absolute_path])
        if errors:
            failed += 1
            for error in errors:
                location = "/".join(str(p) for p in error.absolute_path) or "<root>"
                warn("{}: {}: {}", path, location, error.message)
        else:
            info("{}: OK", path)

    if failed:
        die(64, "{} of {} configuration file(s) failed schema validation!", failed, len(files))

    info("All {} participant configuration file(s) are valid!", len(files))


if __name__ == "__main__":
    main()
