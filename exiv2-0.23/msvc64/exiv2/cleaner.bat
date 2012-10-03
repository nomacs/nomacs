del/s *.vcxproj *.ncb *.user *.filters *.sdf
for /r %d in (build,win32,x64) do rmdir/s/q %d
