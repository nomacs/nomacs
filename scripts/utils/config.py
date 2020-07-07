import os

class Config(object):

    def __init__(self, params, name: str):
        self.__dict__.update(params)
        self.name = name

        self.additional_cmake_args = []

        self.defaults()
        self.check()

    def check(self):

        # check if all directories exist
        for key in self.__dict__:

            p = self.__dict__[key]

            if p:
                if key.endswith("path"):
                    if not os.path.exists(p):
                        print("[WARNING] %s path does not exist: %s" % (key, p))
                    else:
                        self.__dict__[key] = Config.normpath(p)

        if "builddir" in self.__dict__:
            self.builddir = Config.normpath(self.builddir)

    @staticmethod
    def normpath(path: str):
        tmp = os.path.abspath(path)
        # cmake has issues with windows path separators
        tmp = tmp.replace("\\", "/")
        return tmp

    def defaults(self):

        # "debug" builds debug and release
        if "buildconfig" not in self.__dict__ or not self.buildconfig:
            self.buildconfig = "debug"

        # call install?
        if "install" not in self.__dict__:
            self.install = True

        # if set to True, we'll run a dry run (only configure cmake)
        if "cmakeonly" not in self.__dict__:
            self.cmakeonly = False

        # if set, we can skip builds
        if "binaryfile" not in self.__dict__:
            self.binaryfile = ""

        # if true, we will always rebuild
        if "force" not in self.__dict__:
            self.force = False

        if "srcpath" not in self.__dict__ or not self.srcpath:
            self.srcpath = os.path.join(self.repopath, self.name)

        # this is not called 'path' because it won't exist until we call cmake
        if "builddir" not in self.__dict__ or not self.builddir:
            self.builddir = os.path.join(self.repopath, "build")
        else:
            self.builddir = os.path.abspath(self.builddir)

    def cmake_args(self):

        return ""

    def cmake_build_args(self):
    
        return ""

    def __str__(self):

        keyvals = [key + ": " + str(self.__dict__[key])
                   for key in self.__dict__]
        return "\n".join(keyvals)

    def is_built(self):

        if self.force:
            return False

        return os.path.isfile(self.binaryfile)
