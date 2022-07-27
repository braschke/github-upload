
set SOURCE=test.c
rem set SOURCE=strvect.c
rem set SOURCE=strhash.c
rem set SOURCE=strsjoinl.c
set DEFINES=/DINCLUDE_STATIC=1

cl /TC /Zi /W4 -I../../include -I../mswin/winutl -I../mswin/tcsutl -I../mswin/charconv -I../mswin/conutl  -I../mswin/sockutl %DEFINES% /DFORTRAN_NAMING_UCU /DFORTRAN_STRING_CLCL %SOURCE% -I"C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v3.2/include"
