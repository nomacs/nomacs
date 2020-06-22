# Update Dependencies for nomacs

## Sync the nomacs fork
First see where we are:
```bash
git remote show origin
```
I use quazip as sample here. Setup the upstream:
```bash
git remote add upstream https://github.com/stachenov/quazip.git
```
Then update:
```bash
git fetch upstream
```
now checkout the latest stable:
```bash
git fetch --all --tags
git log
```
find the tag of the latest stable (let's assume it's ``v0.9.1``):
```bash
git checkout master
git merge tags/v9.0.1
```
solve all merge conflicts and commit. Delete the project's build folder (i.e. 3rd-party/build/quazip), then:
```bash
3rd-party/make.bat "path/to/your/qt/bin/dir"
```
If the build succeeds push your changes. (don't forget to push the changes to the submodule & nomacs)