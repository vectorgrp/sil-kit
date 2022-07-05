#!/usr/bin/env python
import urllib
import urllib.request
import ssl
import sys
import os
import os.path
import shutil
import zipfile


ctx = ssl.create_default_context()
ctx.check_hostname = False
ctx.verify_mode = ssl.CERT_NONE

def log(fmt, *args, **kwargs):
    print(fmt.format(*args), **kwargs)
    sys.stdout.flush()

def fetch(url, outfile_path):
    """ download resource at URL url to outfile"""
    if os.path.exists(outfile_path):
        log(f"WARN: file {outfile_path} already exists! overwriting!")
    with urllib.request.urlopen(url, context=ctx) as req:
        with open(outfile_path, 'wb') as out:
            shutil.copyfileobj(req, out)

def die(msg):
    print(f"ERROR: {msg}")
    sys.exit(-1)

def extract(zip_name, path):
    log(f"Extracting {zip_name} to path \"{path}\"")
    with zipfile.ZipFile(zip_name) as zf:
        zf.extractall(path)

def main():
    artifactory = os.getenv("ARTIFACTORY")
    if artifactory is None:
        die("$ARTIFACTORY environment variable is not set!")
    repo = os.getenv("SILKIT_ARTIFACTORY_REPO")
    if repo is None:
        die("$SILKIT_ARTIFACTORY_REPO  environment variable is not set!")

    mingw_package_url = f"{repo}/ThirdParty/mingw-winlibs/winlibs-x86_64-posix-seh-gcc-11.2.0-mingw-w64-9.0.0-r5.zip"
    zipName = "mingw-w64.zip"
    fetch(f"{artifactory}/{mingw_package_url}", zipName )

    extract(zipName, ".")

if __name__ == "__main__":
    main()
