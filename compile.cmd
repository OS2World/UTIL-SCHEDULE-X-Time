REM SET C_INCLUDE_PATH=c:/usr/include;%C_INCLUDE_PATH%
REM SET INCLUDE=C:\usr\include
REM SET EMXOMFLD_TYPE=WLINK
REM SET EMXOMFLD_LINKER=wl.exe
REM SET PATH=C:\usr\libexec\gcc\i686-pc-os2-emx\9;%PATH%
SET C_INCLUDE_PATH=c:/usr/include;%C_INCLUDE_PATH%
make 2>&1 |tee make.out
