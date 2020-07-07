# Versioning nomacs
For nomacs, we use old-school software versioning with ``MAJOR.MINOR.PATCH.BUILD`` (i.e. 3.14.0.1449). We have elaborated our versioning system such that installer scripts, cmake and nomacs automatically have the same version.

## Increment the Version
To increment all versions, you need to open [CMakeLists.txt](../CMakeLists.txt) and change `NOMACS_VERSION_MINOR`. Then, run:
```bash
python scripts/make.py 'qtpath/bin'
```

## Scripts
While [versionincrement.py](../../scripts/versionincrement.py) increments the current (globally used) nomacs version by updating [version.cache](../../scripts/utils/version.cache):
```bash
python scripts/versionincrement.py 3.14.0
```
[versionupdate.py](../../scripts/versionupdate.py) will manipulate the version string of input files:
```bash
python scripts/versionupdate.py build/DkVersion.h
```
To prevent unnecessary git conflicts, we copy versioned files (`-versioned` is in .gitignore):
```bash
python scripts/versionupdate.py installer/nomacs-setup.wxs --copy
```

## CMake
Typically you don't need to touch any of the scripts above, because CMake does everything if `-DENABLE_INCREMENTER=ON` is set.  
Currently we manipulate these files:
- ./installer/nomacs-setup.wxs -> ./installer/nomacs-setup-versioned.wxs
- ./installer/nomacs-setup.iss -> ./installer/nomacs-setup-versioned.iss
- ./build/DkVersion.h
