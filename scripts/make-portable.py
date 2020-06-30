
def pack(version: str, base: str):
    import os
    import shutil

    pname = "nomacs-" + version
    binarydir = "/installer/nomacs.x64"
    dstdir = "/installer/" + pname
    licenses = "/ImageLounge/license"
    
    zippath = base + dstdir + ".zip"
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
    print(("[nomacs portable] %d files zipped to %s [%.2f MB] ") % (nf, dstdir + ".zip", zipsize))

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
    import os

    parser = argparse.ArgumentParser(
        description='packs nomacs portable.')

    parser.add_argument("repopath", type=str,
                        help="""full path to the nomacs repository""")
    parser.add_argument("version", type=str,
                        help="""current nomacs version""")


    args = parser.parse_args()

    if not os.path.exists(args.repopath):
        print("repository path does not exist: " + args.repopath)
        exit()

    pack(args.version, args.repopath)
