# Releasing 🍸

## Windows
- Update Version Number (in `CMakeLists.txt` *and* `nomacs.rc`)
- Tag commit - Title: version (e.g. 3.2)
- Add changelog to Release on https://github.com/nomacs/nomacs/releases
- Batch build
  - nomacs x64
  - nomacs x86
  - nomacs-plugins x64
  - nomacs-plugins x86
- Run _INSTALL -> Project Only -> `Build only INSTALL`_ for every solution

Now all dll's & exe should be up-to-date. Do not forget to update the install/package/data folders if you changed/updated a library (e.g. Qt).
Open `update-installer.bat` in an editor and ucomment this line (updates all versions in the `package.xml` files):

```REM cscript /nologo ./package-incrementer-batch.vbs ./packages package.xml package-incrementer.vbs```

- clean plugin folders in nomacs.x64 and nomacs.x86
- double-click `update-installer.bat`
- zip the repository folder & name it `repository.zip`
- upload the repository folder to http://download.nomacs.org/htdocs/repository
- upload `nomacs-setup.exe`
- upload `repository.zip`

### nomacs portable
- copy the packages/nomacs.x64/data/nomacs.x64 folder to projects/nomacs/releases and name it `nomacs-3.x.x`
- copy all files from portable-files to this new folder
- copy the `src/settings.css` to this folder and name it `nomacs.qss`
- zip the `nomacs-3.x.x folder`
- back-up the installer folder to the release folder
- upload the `nomacs-3.x.x.zip` to http://download.nomacs.org/htdocs/portable
- update the index.html accordingly
- update http://download.nomacs.org/htdocs/versions/index.html
