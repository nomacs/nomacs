
def build(config):
    import subprocess as sp

    if "all" not in config.project and config.name not in config.project:
        print(config.name + " not selected for building, skipping...")
        return

    if config.is_built():
        print(config.name + " exists, skipping...")
        return

    print(config.name + " ----------------------------------------------")

    cmakeconfig = "cmake " + " ".join(config.cmake_args())
    print(cmakeconfig)

    # configure
    try:
        sp.run(cmakeconfig, check=True)
    except sp.CalledProcessError:
        print("[ERROR] cmake config failed - aborting...")
        exit(1)

    if config.configure:
        print("\"configure\" specified -> done")
        return

    # build release
    cmakebuild = "cmake --build %s --config Release %s -- -m" % \
           (config.builddir, " ".join(config.cmake_build_args()))
    print(cmakebuild)
    sp.run(cmakebuild, check=True)

    # build debug?!
    if config.buildconfig.lower() == "debug":
        cmakebuild = "cmake --build %s --config Debug %s -- -m" % \
            (config.builddir, " ".join(config.cmake_build_args()))
        print(cmakebuild)
        sp.run(cmakebuild, check=True)

    # install
    if config.install:
        sp.run("cmake --build %s --config Release --target INSTALL -- -m" %
               (config.builddir), check=True)
