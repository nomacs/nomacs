class Config(object):

    def __init__(self, params, name: str):
        self.__dict__.update(params)
        self.name = name

        self.defaults()
        self.check()

    def check(self):
        import os

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

        # if set to True, we'll run a dry run (only configure cmake)
        if "cmakeonly" not in self.__dict__ or not self.cmakeonly:
            self.cmakeonly = False

        # config name
        if "name" not in self.__dict__ or not self.name:
            self.name = "please name me"

    def cmake_args(self):

        return ""

    def __str__(self):

        keyvals = [key + ": " + str(self.__dict__[key])
                   for key in self.__dict__]
        return "\n".join(keyvals)

def repopath(arg0: str):
    import os

    relp = os.path.dirname(arg0)
    absp = os.path.abspath(relp)

    rp = absp.replace('scripts', '')

    return rp


def make(config: Config):
    import subprocess as sp

    cmakeconfig = "cmake " + " ".join(config.cmake_args())

    print(cmakeconfig)

    # configure
    sp.run(cmakeconfig)

    if config.cmakeonly:
        print("\"cmakeonly\" specified -> done")
        return

    # build release
    sp.run("cmake --build %s --config Release -- -m" % (config.builddir))

    # build debug?!
    if config.buildconfig.lower() == "debug":
        sp.run("cmake --build %s --config Debug -- -m" % (config.builddir))

    # install
    if config.install:
        sp.run("cmake --build %s --config Release --target INSTALL -- -m" %
               (config.builddir))
