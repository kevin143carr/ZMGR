.AUTODEPEND

#		*Translator Definitions*
CC = bcc +BUILD.CFG
TASM = TASM
TLIB = tlib
TLINK = tlink
LIBPATH = C:\BC\LIB
INCLUDEPATH = C:\BC\INCLUDE


#		*Implicit Rules*
.c.obj:
  $(CC) -c {$< }

.cpp.obj:
  $(CC) -c {$< }

#		*List Macros*


EXE_dependencies =  \
 memmgr.obj \
 fwdmgr.obj \
 dldmgr.obj \
 procman.obj \
 upldmgr.obj \
 transfer.obj \
 configcl.obj \
 zmgr.obj \
 execpgm.obj \
 secrets.obj

#		*Explicit Rules*
zmgr.exe: build.cfg $(EXE_dependencies)
  $(TLINK) /v/x/c/P-/L$(LIBPATH) @&&|
c0l.obj+
memmgr.obj+
fwdmgr.obj+
dldmgr.obj+
procman.obj+
upldmgr.obj+
transfer.obj+
configcl.obj+
zmgr.obj+
execpgm.obj+
secrets.obj
zmgr
		# no map file
emu.lib+
mathl.lib+
cl.lib
|


#		*Individual File Dependencies*
memmgr.obj: build.cfg memmgr.cpp 

fwdmgr.obj: build.cfg fwdmgr.cpp 

dldmgr.obj: build.cfg dldmgr.cpp 

procman.obj: build.cfg procman.cpp 

upldmgr.obj: build.cfg upldmgr.cpp 

transfer.obj: build.cfg transfer.cpp 

configcl.obj: build.cfg configcl.cpp 

zmgr.obj: build.cfg zmgr.cpp 

execpgm.obj: build.cfg execpgm.cpp 

secrets.obj: build.cfg secrets.cpp 

#		*Compiler Configuration File*
build.cfg: zmgr.mak
  copy &&|
-ml
-v
-vi-
-wpro
-weas
-wpre
-I$(INCLUDEPATH)
-L$(LIBPATH)
| build.cfg


