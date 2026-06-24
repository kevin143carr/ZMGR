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


ZMGR_dependencies =  \
 zmgr.obj

ZMFT_dependencies =  \
 memmgr.obj \
 fwdmgr.obj \
 dldmgr.obj \
 procman.obj \
 upldmgr.obj \
 transfer.obj \
 configcl.obj \
 serbrdg.obj \
 zmft.obj \
 execpgm.obj \
 secrets.obj \
 dosser.lib

ZMSER_dependencies =  \
 serbrdg.obj \
 zmser.obj \
 dosser.lib

ZMDIAG_dependencies =  \
 serial.obj \
 zmdiag.obj \
 dosser.lib

ZMCFG_dependencies =  \
 zmcfg.obj

#		*Explicit Rules*
all: zmgr.exe zmft.exe zmser.exe zmdiag.exe zmcfg.exe

zmgr.exe: build.cfg $(ZMGR_dependencies)
  $(TLINK) /v/x/c/P-/L$(LIBPATH) @&&|
c0l.obj+
zmgr.obj
zmgr
		# no map file
emu.lib+
mathl.lib+
cl.lib
|

zmft.exe: build.cfg $(ZMFT_dependencies)
  $(TLINK) /v/x/c/P-/L$(LIBPATH) @&&|
c0l.obj+
memmgr.obj+
fwdmgr.obj+
dldmgr.obj+
procman.obj+
upldmgr.obj+
transfer.obj+
configcl.obj+
serbrdg.obj+
zmft.obj+
execpgm.obj+
secrets.obj
zmft
		# no map file
emu.lib+
mathl.lib+
dosser.lib+
cl.lib
|

zmser.exe: build.cfg $(ZMSER_dependencies)
  $(TLINK) /v/x/c/P-/L$(LIBPATH) @&&|
c0l.obj+
serbrdg.obj+
zmser.obj
zmser
		# no map file
emu.lib+
mathl.lib+
dosser.lib+
cl.lib
|

zmdiag.exe: build.cfg $(ZMDIAG_dependencies)
  $(TLINK) /v/x/c/P-/L$(LIBPATH) @&&|
c0l.obj+
serial.obj+
zmdiag.obj
zmdiag
		# no map file
emu.lib+
mathl.lib+
dosser.lib+
cl.lib
|

zmcfg.exe: build.cfg $(ZMCFG_dependencies)
  $(TLINK) /v/x/c/P-/L$(LIBPATH) @&&|
c0l.obj+
zmcfg.obj
zmcfg
		# no map file
emu.lib+
mathl.lib+
cl.lib
|

dosser.lib: build.cfg dosser.obj
  if exist dosser.lib del dosser.lib
  $(TLIB) dosser.lib +dosser.obj


#		*Individual File Dependencies*
memmgr.obj: build.cfg memmgr.cpp

fwdmgr.obj: build.cfg fwdmgr.cpp 

dldmgr.obj: build.cfg dldmgr.cpp 

procman.obj: build.cfg procman.cpp 

upldmgr.obj: build.cfg upldmgr.cpp 

transfer.obj: build.cfg transfer.cpp 

configcl.obj: build.cfg configcl.cpp

serial.obj: build.cfg serial.cpp

serbrdg.obj: build.cfg serbrdg.cpp

dosser.obj: build.cfg dosser.c dosser.h

zmgr.obj: build.cfg zmgr.cpp

zmft.obj: build.cfg zmft.cpp

zmser.obj: build.cfg zmser.cpp

zmdiag.obj: build.cfg zmdiag.cpp

zmcfg.obj: build.cfg zmcfg.cpp

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


