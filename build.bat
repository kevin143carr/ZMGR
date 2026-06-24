@echo off
del *.exe
del *.obj
del BUILD.LOG
echo building zmgr > BUILD.LOG
make -B -f zmgr.mak >> BUILD.LOG
echo finished building zmgr.exe >> BUILD.LOG
type BUILD.LOG
