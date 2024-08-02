.AUTODEPEND

#		*Translator Definitions*
CC = bcc +ZMGR.CFG
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
zmgr.exe: zmgr.cfg $(EXE_dependencies)
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
memmgr.obj: zmgr.cfg memmgr.cpp 

fwdmgr.obj: zmgr.cfg fwdmgr.cpp 

dldmgr.obj: zmgr.cfg dldmgr.cpp 

procman.obj: zmgr.cfg procman.cpp 

upldmgr.obj: zmgr.cfg upldmgr.cpp 

transfer.obj: zmgr.cfg transfer.cpp 

configcl.obj: zmgr.cfg configcl.cpp 

zmgr.obj: zmgr.cfg zmgr.cpp 

execpgm.obj: zmgr.cfg execpgm.cpp 

secrets.obj: zmgr.cfg secrets.cpp 

#		*Compiler Configuration File*
zmgr.cfg: zmgr.mak
  copy &&|
-ml
-v
-vi-
-wpro
-weas
-wpre
-I$(INCLUDEPATH)
-L$(LIBPATH)
| zmgr.cfg


