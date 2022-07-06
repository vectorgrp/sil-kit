#!/usr/bin/env python3
# ABI check a current build DLL against a published release
# usage:
# ./IntegrationBus/ci/abicheck.py  ../vib-delivery/Release_v3.0.4/IntegrationBus-3.0.4-gcc-Linux.zip _build_linux/Release/libIntegrationBus.so  -v
#
# Algorithm:
# Unzip the release package
# build the packaged Demos against release VIB dll
# substitute local VIB dll in the demos folder
# run demos for a couple of seconds

import urllib
import urllib.parse
import urllib.request
import os
import shutil
import logging
import argparse
import subprocess
import ssl
import sys
import tempfile
import zipfile
import re
import signal


logging.basicConfig(stream=sys.stdout)
logger = logging.getLogger('abicheck')
logger.setLevel(logging.INFO)
info = logger.info
error = logger.error
debug = logger.debug

# utilities
def die(*args):
    error(*args)
    sys.stdout.flush()
    sys.exit(1)

def run(cwd, cmd, timeout=None):
    try:
        flags = 0
        if os.name == 'nt':
            flags = subprocess.CREATE_NEW_PROCESS_GROUP | subprocess.CREATE_NEW_CONSOLE
        debug(f"Run: timeout={timeout}: {cmd}")
        p = subprocess.Popen(cmd, stdin=subprocess.PIPE,  shell=False, cwd=cwd,
                creationflags=flags)
        p.wait(timeout)
    except subprocess.TimeoutExpired as e:
        p.stdin.close()
        if os.name == 'nt':
            debug(f"Killing all subprocess of {p.pid}")
            subprocess.check_call(f"taskkill /PID {p.pid}  /T /F")
        else:
            debug(f"Sending SIGINT to {p.pid}")
            p.send_signal(signal.SIGINT)
        info("Subprocess reached timeout! sending Ctrl-C to IbLauncher")
    except subprocess.CalledProcessError as e:
        die(f"Subprocess \"{cmd}\" failed : {e!s}")
    p.wait()
    return p.returncode

def get_cmake_flags(filename):
    """ based on standardized CPack filename, return a cmake generator used"""
    gen =  {
        "gcc-Linux" : ["-DCMAKE_CXX_COMPILER=g++"],
        "clang-Linux": ["-DCMAKE_CXX_COMPILER=clang"],
        "VS2015-Win32": ["-GVisual Studio 15 2017", "-tv140"], 
        "VS2015-Win64": ["-GVisual Studio 15 2017 Win64", "-tv140"], 
        "VS2017-Win32": ["-GVisual Studio 15 2017"],
        "VS2017-Win64": ["-GVisual Studio 15 2017 Win64"], 
    }
    #NB: we implictly raise if match fails
    m = re.match(r'.*IntegrationBus-\d+\.\d+\.\d+-(.*)\.zip', filename)
    debug(f"filename={filename}")
    used_gen = gen[m[1]]
    debug(f"package arch: {m[1]}, generator: {used_gen}")
    return used_gen


def unpack(workdir, cpack):
    """ unzip cpack into workdir and return toplevel directory name"""
    # ZipInfo.is_dir is introduced in Python 3.6: compat hack
    debug(f"Unpack: workdir={workdir} zipfile={cpack}")
    def is_dir(zipinfo):
        return zipinfo.filename[-1] == '/'
    info(f"Unpacking {cpack}")
    with zipfile.ZipFile(cpack) as zf:
        count = 0
        #assuming the zipfile contains a folder named like itself
        top = os.path.basename(os.path.splitext(cpack)[0])
        debug(f"top={top}")
        for mem in zf.infolist():
            # strip toplevel folder from member filepaths
            el = mem.filename.split("/") #zip contains unix paths
            assert top == el[0] #sanity check
            fn = mem.filename
            out = os.path.join(workdir, os.path.normpath(fn))
            #fix up output folders
            parent = os.path.join(workdir, os.path.dirname(fn))
            if not os.path.exists(parent):
                os.makedirs(parent)
                debug(f"-> mkdir (parent) {parent}")
            if is_dir(mem) and not os.path.exists(out):
                os.makedirs(out)
                debug(f"-> mkdir {out}")
            else:
                if not os.path.exists(out):
                    extattr = mem.external_attr >> 16
                    debug(f"-> {out} (mode={oct(extattr)})")
                    with open(out, "wb") as fout:
                        fout.write(zf.read(mem))
                    os.chmod(out, extattr)
                    count += 1
        info(f"{top}: {count} files written.")
        #assert count != 0

    return os.path.join(workdir, top)

def get_version(filename):
    filename = os.path.basename(filename)
    m = re.match(r'.*IntegrationBus-(\d+\.\d+\.\d+)-.*\.zip', filename)
    return m[1]


def backup_and_replace_dll(version, bindir, dll):
    """ backup the existing dll and copy the dll under test to bindir """
    assert len(version) > 0 and version.find(" ") == -1
    assert os.path.exists(dll)
    #backup
    if os.name == "posix":
        bindir = os.path.join(bindir, "..", "lib")

    oldf = os.path.join(bindir, os.path.basename(dll))
    newf = os.path.join(bindir, f"BACKUP_{version}_{os.path.basename(dll)}")
    debug(f"Copying {oldf} ---> {newf}")
    if not os.path.exists(newf):
        os.rename(oldf, newf)
    #replace
    shutil.copyfile(dll, oldf)

def main():
    ap = argparse.ArgumentParser("ABI check", 
            description="""
            Extracts the packaged zip file and builds demos against the included IntegrationBus DLL.
            Then, the DLL is replaced with the one provided on the command line, and a demo is executed for 5 seconds.

            Note: make sure to use matching Release/Debug builds of the package and your DLL.
            
            """
    )
    ap.add_argument("PACKAGE_ZIP", type=str, help="the URL of the release package to download")
    ap.add_argument("DLL_PATH", type=str, 
        help="local Path to the IntegrationBus dll that shall be subsituted for building")
    ap.add_argument("-v", "--verbose", action="store_true", help="enable verbose output")
    args = ap.parse_args()
    if args.verbose:
        logger.setLevel(logging.DEBUG)

    info(f"Startup. URL: {args.PACKAGE_ZIP} DLL: {args.DLL_PATH}")

    pkg_url = args.PACKAGE_ZIP
    #check DLL 
    if not os.path.exists(args.DLL_PATH):
        die(f"Path \"{args.DLL_PATH}\" to IntegrationBus DLL does not exist")
    dll_path = os.path.realpath(args.DLL_PATH)
   
    #temporary workdir
    tempdir = os.path.join(os.getcwd(), "vib_abicheck_temp")
    workdir = os.path.splitext(os.path.basename(pkg_url))[0] #assume zip file contains folder with same name
    workdir = os.path.join(tempdir, workdir)
    try:
        os.mkdir(tempdir)
    except:
        pass

    info(f"Working in {tempdir}")
    
    #fetch package if necessary
    cmake_flags = get_cmake_flags(os.path.basename(pkg_url))
    debug(f"cmake_flags: {cmake_flags}")

    if not os.path.exists(workdir):
        unpackdir = unpack(tempdir, pkg_url)
        assert (unpackdir == workdir)
    else:
        info("Skipping unzip (already exists: f{workdir})")

    demodir = os.path.join(workdir, 'IntegrationBus-Demos')
    bindir = os.path.join(workdir, 'IntegrationBus', 'bin')

    debug(f"tempdir: {tempdir}")
    debug(f"workdir: {workdir}")
    debug(f"demodir: {demodir}")
    debug(f"bindir: {bindir}")

    run(demodir, ["cmake", "-B", "build_abicheck", "."] + cmake_flags)
    run(demodir, ["cmake", "--build", "build_abicheck", "--parallel", "3"])

    backup_and_replace_dll(get_version(pkg_url), bindir, dll_path)

    iblauncher=os.path.join(bindir, "IbLauncher.py")
    if os.name == "nt":
        iblauncher = ["python", iblauncher]
    else:
        iblauncher = [iblauncher]
    run(bindir, iblauncher +
            ["-c", "Installation",
                "../../IntegrationBus-Demos/Ethernet/IbConfig_DemoEthernet.json"]
        , timeout=5
    )


if __name__ == "__main__":
    main()
