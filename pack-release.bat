@echo off
set OUTDIR=release
set PKGROOT=%OUTDIR%\gremov
set OUTFILE=%PKGROOT%.7z

del /f /q %OUTFILE%
rmdir /s /q %PKGROOT%
mkdir %PKGROOT%
copy /y *.exe %PKGROOT%\
copy /y removal_overrides.xml %PKGROOT%\
copy /y LICENSE %PKGROOT%\
copy /y README.md %PKGROOT%\

rem touch -m -r %OUTDIR%\sdp.exe %PKGROOT%
7z.exe a -t7z -mx -myx -mtr- -stl %OUTFILE% .\%PKGROOT%\
