#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
   printf("_MSC_VER: %d, 0%04X\n",_MSC_VER,_MSC_VER);
   return 0;
}
