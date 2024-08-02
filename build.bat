@echo off
del *.exe
del *.obj
echo building zmgr
make -B -f zmgr.mak
echo finished building zmgr.exe