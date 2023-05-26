#!/usr/bin/env python3
# This tool packages the final deliverables for SIL Kit releases for a given target (VS2017, Linux etc.)
# the simplified cpack usage results in separate Debug and Release zip files.
# this tool is used to combine Debug and Release distribution zipfiles and make some
# additional changes: adjusting paths, adding directories/files etc.
####
## Usage:
## call this script with the CPack zipfiles as arguments.
## optionally give the sourcetree root as argument.
## type package.py --help for more infos
##
from __future__ import print_function
import sys, argparse, os
import zipfile
import re
import shutil
import glob
import fnmatch
import hashlib

#################################################
# fixups for packages
# this is inherent to the cmake packaging and/or other binary distributions
# keep this up to date and in sync
#################################################
# fixup(work, topdir) -> bool
fixups=[
]
def applyFixups(workdir, topdir):
    global fixups
    log("Applying fixups")
    for fix in fixups:
        ok = fix(workdir, topdir)
        log("fixup {}: {}",fix.__name__, "OK" if ok else "ERROR")
#################################################
#sanity checks and utils
#################################################
build_types=["Debug", "Release"]
archs=["Win32", "Win64", "Linux", "ubuntu"]

DEBUG=0
KEEP_TOP_DIRS=False

def log(fmt, *args):
    print(fmt.format(*args))

def debug(fmt, *args):
    if DEBUG > 0:
        log("-- "+fmt, *args)

def die(status, fmt, *args):
    log("ERROR: " + fmt, *args)
    sys.exit(status)

#expected cpack filename: Project-major.minor.build-TOOL-ARCH-BUILD_TYPE.zip
#Example: SilKit-3.99.29-ubuntu-20.04-x86_64-clang-Debug.zip
expfn = r'^(?P<name>\w+)-(?P<version>\d+\.\d+\.\d+)-(?P<platform>\w+(?:-[\w.]+)?)-(?P<arch>[\w_]+)-(?P<tool>\w+)-(?P<buildtype>\w+)\.zip$'
rc = re.compile(expfn)
def extractDistInfos(filename):
    """ split the filename into its component like version, tool, arch, build type etc.
    """
    fn = os.path.basename(filename)
    m = rc.match(fn)
    if m:
        return m.groupdict()
    return None
def isCompatibleCpack(cpack1, cpack2):  #type: (str, str) -> bool:
    v1 = extractDistInfos(cpack1)
    v2 = extractDistInfos(cpack2)
    log("{} distinfos: {}", cpack1, v1)
    log("{} distinfos: {}", cpack2, v2)
    assert v1 != None and v2 != None
    assert len(v1) == len(v2)
    ok = True
    for key in v1:
        if key != "buildtype":
            if v2[key] != v1[key]:
                log("ERROR: cpack files not matching: {}: {} != {}", key, v1[key], v2[key])
            ok &= v2[key] == v1[key]
    assert v1["buildtype"] in build_types
    assert v2["buildtype"] in build_types
    return ok

def get_prefix(first, second):
    """ os.path.commonpath is only available in python 3.5+.
    we only need the string prefix -- the paths always exist in our case.
    """
    l1, l2 = len(first), len(second)
    if l1 > l2:
        assert first.startswith(second)
        return first[0:l2]
    else:
        assert second.startswith(first)
        return second[0:l1]

def copyWithExcludes(src, dest, exlist=[]):
    """ copy src to workdir/dest, but exclude all patterns in exlist.

    """
    if not os.path.exists(src):
        raise Exception("{} does not exist!".format(src))
    if not os.path.exists(dest):
        os.makedirs(dest)
    def ignored(fn):
        for pat in exlist:
            if fnmatch.fnmatch(fn, pat):
                return True
        return False
    for base, dirs, fs in os.walk(src):
        for f in fs:
            fpath = os.path.join(base, f)
            if ignored(fpath):
                continue
            prefix = get_prefix(src, fpath)
            apath = fpath[len(prefix):]
            while apath.startswith(os.path.sep):
                apath=apath[1:]
            apath = os.path.join(dest,apath)
            adir = os.path.dirname(apath)
            if not os.path.exists(adir):
                os.makedirs(adir)
                debug("staging mkdir {}".format(adir))
            debug("staging copy {} -> {}".format(fpath, apath))
            shutil.copy2(fpath, apath)

def hashFile(fname):
    alg = hashlib.sha256()
    with open(fname, 'rb') as f:
        while True:
            dat = f.read(1<<21)
            if len(dat) == 0:
                break
            alg.update(dat)
    return alg.hexdigest()
#################################################
# main 
#################################################
def parseArgs():
    abspath = os.path.abspath
    parser = argparse.ArgumentParser(description='SIL Kit distribution packaging tool. It allows bundling and merging multiple CPack generated zip files into a single release zip file')
    parser.add_argument('--projectroot', metavar='PROJECT_ROOT', type=str,
            help="the toplevel project directory containing the source")
    parser.add_argument('zipfiles', metavar='SilKit-<CONFIG>.zip', type=str,
            help="""Zip files which contain the SIL Kit binaries and are packaged by CPack.
            The distribution will contain all merged zipfiles. 
            The pattern of <name>-<sem_version>-<compiler>-<arch>-<buildtype> is significant for computing the output name.
            For Example: SilKit-1.0.0-VS2015-Win32-Debug.zip SilKit-1.0.0-VS2015-Win32-Release.zip""",
            nargs="+")
    parser.add_argument('--work-dir', type=str, help="Specify the work directory", default="_package")
    parser.add_argument('-d','--debug', action="store_true", help="enable debugging output")
    parser.add_argument('--keep-top-directory', action="store_true", help="""keep a top level directory for each zip file.
            That is, merge the CPack zip files into the same root directory and create a distribution zip file.""")
    args = parser.parse_args()
    cpackfiles = [abspath(x) for x in args.zipfiles]
    workdir=abspath(args.work_dir)
    if args.debug:
        global DEBUG
        DEBUG=1
    if args.keep_top_directory:
        global KEEP_TOP_DIRS
        KEEP_TOP_DIRS=True
    files =[]
    for cpack in cpackfiles:
        # check if glob pattern
        if cpack.count("*") > 0 :
            tmp = glob.glob(cpack)
            if len(tmp) == 1:
                files.append(tmp[0])
                continue
            else:
                die(1, "zip pattern \"{}\" match error: {}", cpack, tmp)

        if not os.path.exists(cpack):
            die(1, "zip file does not exist: {}", cpack)
        else:
            files.append(cpack)
    if len(files) != len(build_types):
        die(2, "sanity check failed: more cpack files than supported build types"
        "given as argument (expected: {}".format(build_types))
    if not isCompatibleCpack(files[0], files[1]):
        die(3, "incompatible cpack files (based on name schema) detected!")


    return files, args.projectroot, workdir

def makeDistribution(workdir, projectroot, deploy_dirs, excludes):
    """ create an easy to use package containing support 
    cmake files, source code and binaries.
    deploy_dirs is a list of directories that should be copied verbatim, sans the exclude patterns in the 'excludes' list
    """
    if projectroot:
        projectroot = os.path.abspath(projectroot)
    else:
        log("WARN: no project root given. skipping source distribution step")
        return
    log("Copying distribution files to work dir")
    for nd in deploy_dirs:
        src=os.path.join(projectroot, nd)
        dest=os.path.join(workdir, deploy_dirs[nd])
        copyWithExcludes(src, dest, excludes)

def unpack(workdir, cpackfiles):
    """ unzip all the cpackfiles into the workdir """
    # ZipInfo.is_dir is introduced in Python 3.6: compat hack
    global KEEP_TOP_DIRS
    def is_dir(zipinfo):
        return zipinfo.filename[-1] == '/'
    for cpack in cpackfiles:
        log("Unpacking {}", cpack)
        with zipfile.ZipFile(cpack) as zf:
            count = 0
            #assuming the zipfile contains a folder named like itself
            top = os.path.basename(os.path.splitext(cpack)[0])
            debug("top={}", top)
            for mem in zf.infolist():
                # strip toplevel folder from member filepaths
                el = mem.filename.split("/") #zip contains unix paths
                if KEEP_TOP_DIRS:
                    fn = mem.filename
                else:
                    assert top == el[0] #sanity check
                    fn = "/".join(el[1:])

                if len(el) > 2 and not el[1].startswith("SilKit-"):
                    fn = "SilKit/" + fn

                out = os.path.join(workdir, os.path.normpath(fn))
                if is_dir(mem) and not os.path.exists(out):
                    os.makedirs(out)
                    debug("-> mkdir {}", out)
                else:
                    if not os.path.exists(out):
                        extattr = mem.external_attr >> 16
                        debug("-> {} (mode={})", out, oct(extattr))
                        with open(out, "wb") as fout:
                            fout.write(zf.read(mem))
                        os.chmod(out, extattr)
                        count += 1
            log("{}: {} files written.", top, count)
            #assert count != 0


def setupWorkdir(workdir):
    """ make sure the workdir is clean """
    log("cleaning work dir")
    if os.path.exists(workdir):
        shutil.rmtree(workdir)
    os.mkdir(workdir)


def packDistribution(workdir, distinfos):
    out = "{}-{}-{}-{}-{}".format(
        distinfos["name"],
        distinfos["version"],
        distinfos["platform"],
        distinfos["arch"],
        distinfos["tool"]
        )
    log("Packing distribution in file {}.zip".format(out))
    count = 0
    with zipfile.ZipFile("{}.zip".format(out), mode="w", compression=zipfile.ZIP_DEFLATED) as zf:
        for base, dirs, fs in os.walk(workdir):
            for d in dirs:
                #make sure empty directories are kept
                dpath = os.path.join(base, d)
                if os.listdir(dpath) == []:
                    prefix = get_prefix(workdir, dpath)
                    apath = dpath[len(prefix):]
                    while apath.startswith(os.path.sep):
                        apath=apath[1:]
                    tgt = os.path.join(out, apath) + os.path.sep
                    zi =zipfile.ZipInfo(tgt)
                    zf.writestr(zi, "")
                    debug("zip mkdir {}".format(tgt))

            for f in fs:
                fpath = os.path.join(base, f)
                prefix = get_prefix(workdir, fpath)
                apath = fpath[len(prefix):]
                while apath.startswith(os.path.sep):
                    apath=apath[1:]
                tgt=os.path.join(out, apath)
                debug("zip add {}".format(tgt))
                zf.write(fpath, arcname=tgt)
                count += 1
    cksum = hashFile('{}.zip'.format(out))
    log("Done. read {} files. SHA256={}", count,cksum)

def main():
    cpackfiles, projectroot, workdir = parseArgs()
    log("using CPack zip files: {}", ", ".join(cpackfiles))
    log("using source directory: {}", projectroot)
    log("using working directory: {}", workdir)

    setupWorkdir(workdir)
    unpack(workdir, cpackfiles)
    excludes=[
        "*.*.sw*",
        "*~",
        "*.vcxproj*",
        "*.sln",
        ".editorconfig",
        "*makefile.in",
        "*/ci/*",
        ".git",
    ]
    deploy_dirs={
        #Source   archive-name
        #"Demos" : "Demos",
        #"SilKit" : "Source",
        #"ThirdParty/json11" : "ThirdParty/json11",
        #"ThirdParty/googletest-1.8.0" : "ThirdParty/googletest-1.8.0",
        #"ThirdParty/fmt-5.3.0" : "ThirdParty/fmt-5.3.0",
    }
    makeDistribution(workdir, projectroot, deploy_dirs, excludes)
    applyFixups(workdir, projectroot)
    dinfos = extractDistInfos(cpackfiles[0])
    packDistribution(workdir, dinfos)

if __name__=="__main__":
    main()

