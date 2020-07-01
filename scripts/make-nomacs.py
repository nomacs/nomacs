

class Config(object):
    
    def __init__(self, params):
        self.__dict__.update(params)

        self.defaults()
        self.check()

    def check(self):

        # check if all directories exist
        for key in self.__dict__:

            p = self.__dict__[key]

            if key.endswith("path") and \
               not os.path.exists(p):
                    print("[WARNING] %s path does not exist: %s" % (key, p))

    def defaults(self):

        # "debug" builds debug and release
        if "buildconfig" not in self.__dict__ or not self.buildconfig:
            self.buildconfig = "debug"

        # call install?
        if "install" not in self.__dict__ or not self.install:
            self.install = True

    def __str__(self):

        keyvals = [key + ": " + str(self.__dict__[key]) for key in self.__dict__]
        return "\n".join(keyvals)

class NmcConfig(Config):

    def __init__(self, params):
        super().__init__(params)

    def defaults(self):

        super().defaults()

        if not self.libpath:
            self.libpath = self.rootpath + "3rd-party/build"

        if "srcpath" not in self.__dict__ or not self.srcpath:
            self.srcpath = self.rootpath + "ImageLounge"

        # this is not called 'path' because it won't exist until we call cmake
        if "builddir" not in self.__dict__ or not self.builddir:
            self.builddir = self.rootpath + "build"


    def cmake_args(self):

        # tune cmake parameters here
        args = [
            "-DCMAKE_PREFIX_PATH=" + self.qtpath,
            "-DDEPENDENCY_PATH=" + self.libpath,
            "-DENABLE_TRANSLATIONS=ON",
            "-DENABLE_HEIF=ON",
            "-DENABLE_AVIF=ON",
            "-DENABLE_INCREMENTER=ON",
            "-B" + self.builddir,
            self.srcpath
        ]

        return args

def make(config: NmcConfig = Config):
    import subprocess as sp

    cmakeconfig = "cmake " + " ".join(config.cmake_args())

    print(cmakeconfig)

    # configure
    sp.run(cmakeconfig)

    # build release
    sp.run("cmake --build %s --config Release -- -m" % (config.builddir))

    # build debug?!
    if config.buildconfig.lower() == "debug":
        sp.run("cmake --build %s --config Debug -- -m" % (config.builddir))

    # install
    if config.install:
        sp.run("cmake --build %s --config Release --target INSTALL -- -m" % (config.builddir))

if __name__ == "__main__":
    import argparse
    import sys
    import os
    from utils import repopath

    parser = argparse.ArgumentParser(
        description='packs nomacs portable.')

    parser.add_argument("qtpath", type=str,
                        help="""path to your Qt folder""")
    parser.add_argument('--lib-path', dest='libpath', type=str, default="",
                        help='additional cmake arguments')

    # get our root path
    rootpath = repopath(sys.argv[0])

    # make args a dict
    params = vars(parser.parse_args())
    params["rootpath"] = rootpath

    c = NmcConfig(params)

    # uncomment for debugging
    # print(c)

    make(c)
