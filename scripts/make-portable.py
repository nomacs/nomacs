
def pack(base: str):
    import os
    import shutil
    from utils.fun import version

    version = version()

    pname = "nomacs-" + version
    binarydir = "/installer/nomacs.x64"
    dstdir = "/installer/" + pname
    licenses = "/ImageLounge/license"
    
    zipname = "nomacs-portable-win.zip"
    zippath = base + "/installer/" + zipname
    dst = base + dstdir

    # collect all files
    copytree(base + binarydir, dst)
    copytree(base + licenses, dst + "/license")

    # create empty settings.ini (nomacs will then recognize that it's portable)
    open(dst + "/settings.ini", 'w').close()

    # zip
    nf = zip(dst, zippath, pname)

    # clean up
    shutil.rmtree(dst)

    zipsize = os.stat(zippath).st_size / (1024*1024) # -> MB
    print(("[nomacs portable] %d files zipped to %s [%.2f MB] ") % (nf, zipname, zipsize))

def zip(src: str, dst: str, zipbase: str):
    import zipfile

    numfiles = 0

    with zipfile.ZipFile(dst, 'w', zipfile.ZIP_DEFLATED) as zh:

        # zip everything
        for root, dirs, files in os.walk(src):
            base = root.replace(src, zipbase)

            for file in files:
                zh.write(os.path.join(root, file), os.path.join(base, file))
                numfiles += 1

    return numfiles

def copytree(src: str, dst: str, ext: str = "", symlinks: bool = False, ignore: bool = None):
    import shutil
    import os

    if not os.path.exists(dst):
        os.mkdir(dst)

    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)

        if os.path.isdir(s):
            copytree(s, d, ext, symlinks, ignore)
        elif ext:
            if s.endswith(ext):
                shutil.copyfile(s, d)
        else:
            shutil.copyfile(s, d)


if __name__ == "__main__":
    import argparse
    import sys, os
    from utils.fun import repopath

    parser = argparse.ArgumentParser(
        description='packs nomacs portable.')

    rp = repopath(sys.argv[0])

    args = parser.parse_args()

    if not os.path.exists(rp):
        print("repository path does not exist: " + rp)
        exit()

    pack(rp)
