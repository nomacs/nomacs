import os
from utils.config import Config

class NomacsConfig(Config):

    def __init__(self, params):
        super().__init__(params, "nomacs")

    def defaults(self):

        if not self.libpath:
            self.libpath = self.repopath + "/3rd-party/build"

        if "srcpath" not in self.__dict__ or not self.srcpath:
            self.srcpath = self.repopath + "/ImageLounge"

        # this is not called 'path' because it won't exist until we call cmake
        if "builddir" not in self.__dict__ or not self.builddir:
            self.builddir = self.repopath + "/build"

        super().defaults()

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


class ExpatConfig(Config):

    def __init__(self, params):
        super().__init__(params, "expat")

        self.install = False

    def defaults(self):

        # this is not called 'path' because it won't exist until we call cmake
        if "builddir" not in self.__dict__ or not self.builddir:
            self.builddir = os.path.join(self.repopath, "build", self.name)

        super().defaults()

        self.binaryfile = os.path.join(self.builddir, "Release", "expat.dll")

    def cmake_args(self):

        # tune cmake parameters here
        args = [
            "--clean-first",
            "-DBUILD_examples=OFF",
            "-DBUILD_tests=OFF",
            "-B" + self.builddir,
            self.srcpath
        ]

        return args


class ExifConfig(Config):

    def __init__(self, params):
        super().__init__(params, "exiv2")

        self.install = False

    def defaults(self):

        if "libpath" not in self.__dict__ or not self.libpath:
            self.libpath = os.path.join(self.repopath, "build")

        # this is not called 'path' because it won't exist until we call cmake
        if "builddir" not in self.__dict__ or not self.builddir:
            self.builddir = os.path.join(self.repopath, "build", self.name)

        super().defaults()

        self.binaryfile = os.path.join(
            self.builddir, "Release", "bin", "exiv2.dll")

    def cmake_args(self):

        # tune cmake parameters here
        args = self.additional_cmake_args + [
            "--clean-first",
            "-DEXPAT_BUILD_PATH=" + self.libpath + "/expat",
            "-DEXPAT_INCLUDE_DIR=" + self.repopath + "/expat/lib",
            "-B" + self.builddir,
            self.srcpath
        ]

        return args


class LibrawConfig(Config):

    def __init__(self, params):
        super().__init__(params, "libraw")

        self.install = False

    def defaults(self):

        # this is not called 'path' because it won't exist until we call cmake
        if "builddir" not in self.__dict__ or not self.builddir:
            self.builddir = os.path.join(self.repopath, "build", self.name)

        super().defaults()

        self.binaryfile = os.path.join(self.builddir, "Release", "raw.dll")

    def cmake_args(self):

        # tune cmake parameters here
        args = self.additional_cmake_args + [
            "--clean-first",
            "-DENABLE_EXAMPLES=OFF",
            "-B" + self.builddir,
            self.srcpath
        ]

        return args


class QuazipConfig(Config):

    def __init__(self, params):
        super().__init__(params, "quazip")

        self.install = False

    def defaults(self):

        # this is not called 'path' because it won't exist until we call cmake
        if "builddir" not in self.__dict__ or not self.builddir:
            self.builddir = os.path.join(self.repopath, "build", self.name)

        if "libpath" not in self.__dict__ or not self.libpath:
            self.libpath = os.path.join(self.repopath, "build")

        super().defaults()

        self.binaryfile = os.path.join(self.builddir, "Release", "quazip5.dll")

    def cmake_args(self):

        # tune cmake parameters here
        args = self.additional_cmake_args + [
            "--clean-first",
            "-DCMAKE_PREFIX_PATH=" + self.qtpath,
            "-B" + self.builddir,
            self.srcpath
        ]

        return args

    def cmake_build_args(self):

        # only build shared lib
        args = [
            "--target quazip5"
        ]

        return args

class OpenCVConfig(Config):

    def __init__(self, params):
        super().__init__(params, "opencv")

        self.install = False

    def defaults(self):

        if "srcpath" not in self.__dict__ or not self.srcpath:
            self.srcpath = os.path.join(self.repopath, self.name)

        # this is not called 'path' because it won't exist until we call cmake
        if "builddir" not in self.__dict__ or not self.builddir:
            self.builddir = os.path.join(self.repopath, "build", self.name)

        super().defaults()

        # FIXME: release is generated by cmake - so this is not enough to ask for...
        self.binaryfile = os.path.join(
            self.builddir, "bin", "Release", "opencv_core430.dll")

    def cmake_zlib(self):

        args = [
            "-DZLIB_INCLUDE_DIR=" + self.srcpath + "/3rdparty/zlib",
            "-DZLIB_BUILD_PATH=" + self.builddir + "/3rdparty",
        ]

        return args

    def cmake_args(self):

        # tune cmake parameters here
        args = [
            "--clean-first",
            "-DBUILD_PERF_TESTS=OFF",
            "-DBUILD_TESTS=OFF",
            "-DBUILD_opencv_java=OFF",
            "-DBUILD_opencv_java_bindings_generator=OFF",
            "-DBUILD_opencv_python=OFF",
            "-DBUILD_opencv_apps=OFF",
            "-DBUILD_opencv_dnn=OFF",
            "-DBUILD_opencv_calib3d=OFF",
            "-DBUILD_opencv_highgui=OFF",
            "-DBUILD_opencv_photo=OFF",
            "-DBUILD_opencv_python2=OFF",
            "-DBUILD_opencv_python3=OFF",
            "-DBUILD_opencv_python_tests=OFF",
            "-DBUILD_opencv_python_bindings_generator=OFF",
            "-DBUILD_opencv_stitiching=OFF",
            "-DBUILD_opencv_video=OFF",
            "-DBUILD_opencv_videoio=OFF",
            "-B" + self.builddir,
            self.srcpath
        ]

        return args


class FormatsConfig(Config):

    def __init__(self, params, name: str):
        super().__init__(params, name)

    def defaults(self):

        if "libpath" not in self.__dict__ or not self.libpath:
            self.libpath = os.path.join(self.repopath, "build")

        # only build release for imageformats        
        self.buildconfig = "release"

        super().defaults()



    def cmake_args(self):

        # tune cmake parameters here
        args = [
            "--clean-first",
            "-DCMAKE_PREFIX_PATH=" +
            os.path.join(self.libpath, "libde265") + ";" +
            os.path.join(self.libpath, "libheif") + ";" +
            self.qtpath,
            "-B" + self.builddir,
            self.srcpath
        ]

        return args


def make_libs(params):
    from utils.build import build

    params['install'] = False

    opencv = OpenCVConfig(params)
    # some script debugging options:
    # opencv.force = True
    print(opencv)
    build(opencv)

    # Exiv2
    expat = ExpatConfig(params)
    build(expat)

    exiv2 = ExifConfig(params)
    exiv2.additional_cmake_args = opencv.cmake_zlib()
    build(exiv2)

    # libraw
    libraw = LibrawConfig(params)
    build(libraw)

    # quazip
    quazip = QuazipConfig(params)
    quazip.additional_cmake_args = opencv.cmake_zlib()
    build(quazip)

    # uncomment for debugging
    # print(expat)

def make_imageformats(params):
    
    params['install'] = False

    # config libde265 which we need for libheif
    libde265 = FormatsConfig(params, "libde265")
    libde265.builddir = os.path.join(libde265.builddir, libde265.name)
    libde265.binaryfile = os.path.join(libde265.builddir, libde265.name, "Release", libde265.name + ".dll")
    build(libde265)
    
    # config libheif
    libheif = FormatsConfig(params, "libheif")
    libheif.builddir = os.path.join(libheif.builddir, libheif.name)
    libheif.binaryfile = os.path.join(libheif.builddir, "libheif", "Release", "heif.dll")
    build(libheif)
    
    # configure image formats
    params["srcpath"] = params["repopath"]
    params['install'] = True
    imageformats = FormatsConfig(params, "imageformats")
    build(imageformats)

    # uncomment for debugging
    # print(libde265)
    # print(libheif)
    # print(imageformats)

def configure_libs(p, config):
    # from makelibs import make as ml
    # from makeimageformats import make as mi

    p['repopath'] = config.repopath + "/3rd-party"

    make_libs(p)

    p['libpath']  = config.libpath + "/imageformats"
    p['repopath'] = config.repopath + "/3rd-party/imageformats"
    p['builddir'] = config.repopath + "/3rd-party/build/imageformats"

    make_imageformats(p)


if __name__ == "__main__":
    import argparse
    import sys
    
    from utils.config import repopath
    from utils.build import build

    parser = argparse.ArgumentParser(
        description='packs nomacs portable.')

    parser.add_argument("qtpath", type=str,
                        help="""path to your Qt folder""")
    parser.add_argument('--lib-path', dest='libpath', type=str, default="",
                        help='additional cmake arguments')
    parser.add_argument('--repo-path', dest='repopath', type=str, default="",
                        help='path to the nomacs repository')
    parser.add_argument('--build-dir', dest='builddir', type=str, default="",
                        help='Specify the build directory')
    parser.add_argument('--build-config', dest='buildconfig', type=str, default="",
                        help='build configuration [debug|release]')
    parser.add_argument('--project', dest='project', type=str, default="all",
                        help='comma separated name of the project(s) to be built (\'all\' will build everything)')
    parser.add_argument('--force', action='store_true',
                    help='forces building the project')
    parser.add_argument('--configure', action='store_true',
                        help='if set, projects are only configured rather than built')


    # make args a dict
    params = vars(parser.parse_args())

    # get the repository path
    if not params['repopath']:
        params['repopath'] = repopath(sys.argv[0])

    params['project'] = params['project'].split(',')

    c = NomacsConfig(params)

    configure_libs(params, c)

    # uncomment for debugging
    print(c)

    build(c)
