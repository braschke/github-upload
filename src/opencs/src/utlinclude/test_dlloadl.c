#define _CRT_SECURE_NO_DEPRECATE
#define INCLUDE_STATIC     1

#define XMEM_USE           1
#define XMEM_USE_PTR       1
#define XMEM_USE_TRACE     0

#define XMSG_USE_PTR       1
#define XMSG_USE_CIO       1
#define XMSG_USE_ASSERT    1

#include "stdheader.h"

#include <stdlib.h>
#include <time.h>

#include "xmsg.h"

#include "xmsg.c"
#include "xmem.h"
#include "xmem.c"

#include "getdynamicfunction.c"

int main(int argc, char *argv[])
{
   void (*pFunction)(void);

   XMSG_SETLEVEL(3);

   if (argc > 1)
      pFunction = (void (*)(void))getdynamicfunction(argv[1],1);
   else
      fprintf(stderr,"Usage: %s funcname::libname\n",argv[0]);
   return 0;
}
