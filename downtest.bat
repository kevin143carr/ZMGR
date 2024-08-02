rescan
call uptest.bat
del c:\upload\*.FIL
del c:\download\keep\*.*
del c:\download\*.*
copy *.obj c:\upload
zmgr -ff MELBA
zmgr -ff TIMEMACH
copy c:\upload\*.* c:\download
del c:\upload\*.*

