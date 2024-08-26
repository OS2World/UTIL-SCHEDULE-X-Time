#--------------------
# Xtime make file 
#--------------------

xtime.obj : xtime.c xtime
     cl -c -G2sw -W3 -Zpi xtime.c

xtime.exe : xtime.obj xtime.def
     link /CODEVIEW xtime, /align:16, NUL, os2, xtime
