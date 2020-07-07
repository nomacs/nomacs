# Versioning nomacs
For nomacs, we use old-school software versioning with ``MAJOR.MINOR.PATCH.BUILD`` (i.e. 3.14.0.1449). We have elaborated our versioning system such that installer scripts, cmake and nomacs automatically have the same version.

## Increment the Version
To increment all versions, you need to open [CMakeLists.txt](../CMakeLists.txt) and change `NOMACS_VERSION_MAJOR` etc.