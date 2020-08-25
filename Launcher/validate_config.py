#!/usr/bin/env python3
import jsonschema
import json
import sys
import os.path

def loadJson(filePath):
    with open(filePath, "r") as f:
        return json.load(f)

def validate_config(schema, cfg):
    print(f"--> Validating {cfg} ...", end="")
    cfgdata = loadJson(cfg)
    try:
        jsonschema.validate(cfgdata, schema)
    except BaseException as  e:
        print("Fail")
        print(f"    Error in {cfg} :")
        for l in str(e).split("\n"):
            print(f"    {l}")
        return False
   
    print("OK")
    return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: Launcher/validate_config.py file1 [file2...]")
        sys.exit(1)

    schemaPath = os.path.join(os.path.dirname(__file__), "iblauncher/data/IbConfig.schema.json")
    schema=loadJson(schemaPath)

    for cfgfile in sys.argv[1:]:
        validate_config(schema, cfgfile)
