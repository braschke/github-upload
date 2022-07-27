#define IS_UNICODE         0
#define INCLUDE_STATIC     1

#define XMSG_USE_PTR       1
#define XMSG_USE_CIO       1
#define XMSG_USE_ATEXIT    1
#define XMSG_USE_ASSERT    1
#define XMSG_USE_PREFIX    1
#define XMSG_USE_TIME      1

#define XMEM_USE           1
#define XMEM_USE_PTR       1
#define XMEM_USE_TRACE     1
#define XMEM_USE_REALLOC   1
#define XMEM_USE_VALLOC    1
#define XMEM_USE_DUP       1

#define XENV_USE              1
#define XENV_USE_PTR          1
#define XENV_USE_SET          1
#define XENV_USE_GETINT       1
#define XENV_USE_GETLONG      1
#define XENV_USE_GETFLOAT     1
#define XENV_USE_GETDOUBLE    1
#define XENV_USE_GETBOOL      1
#define XENV_USE_DEFINED      1
#define XENV_USE_ISTRUE       1
#define XENV_USE_SCANS        1
#define XENV_USE_LOAD         1
#define XENV_USE_PRINT        1

#define RSOCK_USE_CLIENT   1
#define RSOCK_USE_LISTEN   1

#define SSOCK_USE_CLIENT   1
#define SSOCK_USE_LISTEN   1
#define SSOCK_USE_SWAP     1

#define SBMCLIENT_FOR_STARCD 0

#include "stdheader.h"

#include <locale.h>
#include <conio.h>

#include "xmem.h"
#include "xmsg.h"

#include "xmsg.c"
#include "xmem.c"
#include "winshm.c"

#include "getusername.c"
#include "getdatestring.c"
#include "getcpuseconds.c"
#include "getcpuid.c"
#include "getramusage.c"
#include "strlower.c"
#include "strupper.c"
#include "strcxchg.c"
#include "cleanpath.c"
#include "getexepath.c"
#include "bytes2hex.c"

#include "xenv.h"
#include "xenv.c"

#include "cuda_utl.c"

#include "getsigname.c"

#include "strunquote.c"
#include "strunquote1.c"
#include "hlsrgbcolorconvert.c"
#include "getx11color.c"

#include "fload.c"
#include "fimage.c"
#include "fputcn.c"
//#include "wstrcstr.c"

#include "ipool.c"
#include "octbox.c"

#if IS_MSWIN
   #include "MessageBeepEx.c"

   #include "chrcpy_MBToUTF8.c"
   #include "strcnv_UTF8ToMB.c"
   #include "strscpy_ACPToUTF8.c"

   #include "SetConsoleIcon.c"
   #include "GetSystemBootTime.c"
   #include "Get64BitTickCount.c"
   #include "SetProcessPrivileg.c"

   #include "RegGetKeyValue.c"
   #include "RegGetOpenCommand.c"
   #include "FindApplication.c"

   #include "GetProcessPEBAddress.c"
   #include "GetProcessUnicodeString.c"
   #include "GetProcessCommandLine.c"
   #include "GetEnvironmentBlockVariable.c"
   #include "DupEnvironmentBlock.c"

   #include "GetWindowsVersion.c"
   #include "GetWindowsInfoString.c"

   #include "GetPathAsVect.c"
   #include "GetPathextAsVect.c"
   #include "SetHomeEnv.c"
   #include "GetCmdExe.c"

   #include "GetLnkFileTarget.c"
   #include "LookupAccountSidName.c"
   #include "GetWellKnownSid.c"
   #include "GetPermFromAcl.c"
   #include "GetReparsePointTarget.c"
   #include "GetTruePathName.c"
   #include "GetPathInfo.c"

   #include "StreamWrite.c"
   #include "StreamRead.c"

   #include "SockSend.c"
   #include "SockRecv.c"
   #include "SockRecvOOB.c"
   #include "SockSendUTF8.c"

   #include "ServiceOpenByName.c"
   #include "ServiceInstallMyself.c"
   #include "ServiceRemoveByName.c"
   #include "ServiceStartByName.c"
   #include "ServiceStopByName.c"

   #include "gettimeofday.c"
#endif

#include "getbasename.c"
#include "millisleep.c"
#include "getmstime.c"

#include "isshmeta.c"
#include "getendianess.c"
#include "memswapb.c"
#include "memswapv.c"

#include "move_float2double.c"
#include "move_double2float.c"
#include "move_int2long.c"
#include "move_long2int.c"

#include "conv_float2double.c"
#include "conv_double2float.c"
#include "conv_int2long.c"
#include "conv_long2int.c"

#include "strcountchar.c"
#include "memcount_char.c"
#include "memcount_int.c"
#include "memcount_long.c"

#include "memindex_char.c"
#include "memindex_int.c"
#include "memindex_long.c"
#include "memindex_vptr.c"

#include "memrev_int.c"
#include "memrev_vptr.c"

#include "memset_int.c"
#include "memset_long.c"
#include "memset_float.c"
#include "memset_double.c"

#include "vsum_int.c"
#include "vsum_long.c"
#include "vsum_float.c"
#include "vsum_double.c"

#include "memsum2p_float.c"
#include "memsum2p_double.c"

#include "vlintr_float.c"
#include "vlintr_double.c"

#include "memrot.c"
#include "numlen.c"

#include "esolve2_float.c"
#include "esolve2_double.c"

#include "esolve3_float.c"
#include "esolve3_double.c"

#include "strbtrim.c"
#include "str2long.c"
#include "str2double.c"
#include "str2float.c"
#include "str2frac.c"
#include "str2int.c"
#include "str2bool.c"
#include "istr2cstr.c"

#include "strlshift.c"
#include "strrshift.c"

#include "getpathname.c"
#include "getbasename.c"
#include "mkrootname.c"
#include "mkdirname.c"
#include "getextname.c"
#include "gethomedir.c"
#include "getshell.c"
#include "strxerror.c"
#include "getworkdir.c"
#include "pathrepair.c"

#include "mkdirectory.c"
#include "getdynamicfunction.c"

#include "lsearchv.c"
#include "strsjoinl.c"
#include "strscatl.c"
#include "strscat.c"
#include "strscpy.c"
#include "strslen.c"
#include "strscpy_f2c.c"
#include "strscpy_c2f.c"
#include "strcent.c"
#include "strichr.c"
#include "strexp.c"
#include "stripext.c"
#include "strfixk.c"
#include "strfixs.c"
#include "strins.c"
#include "strmatch.c"
#include "strnotch.c"
#include "strnotgr.c"
#include "strtograph.c"
#include "strtoword.c"
#include "strtoprop.c"
#include "chartostr.c"
#include "strsplit.c"
#include "stristr.c"

#include "strclist.c"
#include "striclist.c"
#include "strtoshell.c"

#include "ulong2adp.c"
#include "ulong2adm.c"
#include "ulong2vptr.c"
#include "vptr2ulong.c"



#include "getlocalhostname.c"
#include "usockaddr_pton.c"
#include "usockaddr_ntop.c"
#include "getnetdeviceaddr.c"
#include "getmacaddrv.c"
#include "getifnamev.c"

#include "s3d_angle.c"
#include "s3f_angle.c"
#include "s3r_angle.c"
#include "v3d_angle.c"
#include "v3f_angle.c"
#include "v3r_angle.c"

#include "s3d_anglew.c"
#include "s3f_anglew.c"
#include "s3r_anglew.c"
#include "v3d_anglew.c"
#include "v3f_anglew.c"
#include "v3r_anglew.c"

#include "s3d_sangle.c"
#include "s3f_sangle.c"
#include "s3r_sangle.c"
#include "v3d_sangle.c"
#include "v3f_sangle.c"
#include "v3r_sangle.c"

#include "s3d_norm.c"
#include "s3f_norm.c"
#include "s3r_norm.c"
#include "v3d_norm.c"
#include "v3f_norm.c"
#include "v3r_norm.c"

#include "v3d_ngalign.c"
#include "v3f_ngalign.c"
#include "v3r_ngalign.c"

#include "s3d_spat.c"
#include "s3f_spat.c"
#include "s3r_spat.c"

#include "s3r_cosine.c"
#include "s3r_cosinew.c"

#include "s3d_length.c"
#include "s3f_length.c"
#include "s3r_length.c"

#include "s3d_cross.c"
#include "s3f_cross.c"
#include "s3r_cross.c"



#include "fisdevice.c"
#include "fisdirectory.c"
#include "fisreadable.c"
#include "fxmodtime.c"
#include "fxdelete.c"
#include "fxopen.c"
#include "fxgets.c"
#include "flinecount.c"
#include "fnextline.c"
#include "fnextchar.c"
#include "fprintf_intv.c"
#include "fprintf_dblv.c"
#include "fscanf_intv.c"
#include "folines.c"
#include "fwaitdeleted.c"
#include "fwaitexists.c"
#include "scanpath.c"
#include "setworkdir.c"

#include "splitporthost.c"
#include "rawsocket.c"
#include "streamsocket.c"

#include "strvect.c"

#include "pownd.c"
#include "pownf.c"
#include "pow075.c"


#include "gmem.c"

#if 0
#include "tmp_include.h"
#endif

static char *ge(const char *name){ if (name); return (char *)"UNDEFINIERT"; }
static void _pfull(const char *s, int len) {if(!len) return;fputs("FULL:",stdout); fputs(s,stdout);}
static void _pline(const char *s, int len) {if(!len) return;fputs("LINE:",stdout); puts(s);}

int main(void)
{
   FILE *fp = NULL;
   TCHAR *p1, *p2, tstr[2048];
   char str[2048];
   const TCHAR *tokv[128];
   long l1,l2;
   int  i,i1,i2;
   double d1,d2;
/*   qreal qarr[128]; */
   size_t size;
   SOCK_t *sock = NULL;
   int (*CCI_Def_partition)(int meshId, int partId );
   void *funcp;
   int tokc;

   setbuf(stdout,NULL);
   setbuf(stderr,NULL);


#if IS_MSWIN
   GetWindowsInfoString(str,countof(str));
   printf("Welcome to Microsoft Windows %s.\n",str);
   SetHomeEnv(NULL,0);
   printf("sizeof(UNICODE_STRING)  =%u\n",(unsigned)sizeof(UNICODE_STRING));
   printf("sizeof(BOOLEAN) =%u\n",(unsigned)sizeof(BOOLEAN));
   printf("sizeof(PVOID)   =%u\n",(unsigned)sizeof(PVOID));
   printf("sizeof(DWORD)   =%u\n",(unsigned)sizeof(DWORD));
   printf("sizeof(ULONG)   =%u\n",(unsigned)sizeof(ULONG));
   printf("sizeof(LONG)    =%u\n",(unsigned)sizeof(LONG));
   printf("sizeof(USHORT)  =%u\n",(unsigned)sizeof(USHORT));
   printf("sizeof(UINT16)  =%u\n",(unsigned)sizeof(UINT16));
   printf("sizeof(HANDLE)  =%u\n",(unsigned)sizeof(HANDLE));
   printf("sizeof(SIZE_T)  =%u\n",(unsigned)sizeof(SIZE_T));
#endif
   printf("sizeof(long)    =%u\n",(unsigned)sizeof(long));

#if defined(__GNUC__)
   printf("__GNUC__ is defined\n");
#endif

#if defined(__i386__)
   printf("__i386__ is defined\n");
#endif

#if defined(__i486__)
   printf("__i486__ is defined\n");
#endif

#if defined(__i586__)
   printf("__i586__ is defined\n");
#endif

#if defined(__i686__)
   printf("__i686__ is defined\n");
#endif

#if defined(__ia64__)
   printf("__ia64__ is defined\n");
#endif

#if defined(__x86_64__)
   printf("__x86_64__ is defined\n");
#endif

#if HAVE_CPUID
   printf("HAVE_CPUID is 1\n");
#endif

#if USE_SSE
   printf("USE_SSE is 1\n");
#endif

#ifdef IS_LINUX
   printf("IS_LINUX is defined\n");
#endif

#ifdef IS_MACOSX
   printf("IS_MACOSX is defined\n");
#endif

#ifdef IS_OSFALPHA
   printf("IS_OSFALPHA is defined\n");
#endif

#ifdef IS_HPUX11
   printf("IS_HPUX11 is defined\n");
#endif

#ifdef IS_IRIX65
   printf("IS_IRIX65 is defined\n");
#endif

#ifdef IS_SUNOS
   printf("IS_SUNOS is defined\n");
#endif

#ifdef IS_AIX
   printf("IS_AIX is defined\n");
#endif


   MEMZERO(str,sizeof(str));
   memset(str,'X',128);
   strcpy(str,"HALLO-");
   printf("str=<%s>\n",str);


   STRCPY(tstr,TEXT("//server/share/path/to/file"));
   _tprintf(TEXT("pathname:%s\n"),tstr);
   _tprintf(TEXT("   getpathname:%s\n"),getpathname(tstr));
   _tprintf(TEXT("   mkrootname :%s\n"),mkrootname(tstr));

   STRCPY(tstr,TEXT("A:/server/share/path/to/file"));
   _tprintf(TEXT("pathname:%s\n"),tstr);
   _tprintf(TEXT("   getpathname:%s\n"),getpathname(tstr));
   _tprintf(TEXT("   mkrootname :%s\n"),mkrootname(tstr));


   STRCPY(tstr,TEXT("ZZ_test_dir/long/path/to/a/file/deep/down/in/a/dir/tree/"));
   i = mkdirectory(tstr,0744);
   _tprintf(TEXT("mkdirectory(%s)=%d: %s\n"),tstr,i,strxerror(i));

   printf("SSE-LEVEL=%d\n",getcpuSSE());
   setlocale(LC_ALL,"");
   i1 = TEXT('�');
   i2 = TOUPPER(i1);
   printf("H�tte �ber �: upper %c = %c\n",i1,i2);

   printf("memory: %s\n",ulong2adm(getramusage(),NULL,0));

   p1 = stristr(TEXT("Cars carsten carste carstten carstexx  Carsten Dehning"),TEXT("carsteN"));
   if (p1)
      _tprintf(TEXT("stristr test TRUE(%s).\n"),p1);
   else
      _tprintf(TEXT("stristr test FALSE.\n"));

   STRCPY(tstr,TEXT("hallo|test>ich\\ bin  2>&1 >carsten 3>&4"));
   tokc = strsplit(tstr,128, tokv,TEXT(" \t\n|<&>"),STRSPLIT_FLAG_KEEPEMPTY);
   for (i=0;i<tokc;i++)
      _tprintf(TEXT("token %02d: \"%s\"\n"),i,tokv[i]);
#if 0
   exit(EXIT_SUCCESS);
#endif

#if 0
   XENV_init(ge);
#endif
#if XMSG_USE_PTR
//   XMSG_SETFUNCT(_pfull,_pfull,_pfull,_pline,_pline,_pline);
   XMSG_SETFUNCT(NULL,NULL,NULL,_pline,_pline,_pline);
#endif

   XMSG_ATEXIT(NULL);

   GMEM_getall(10);
#if XMSG_USE_PREFIX
   XMSG_SETPREFIX("[MpCCI %d-%d SETUP] ",9,99);
#endif

   XMSG_SETLEVEL(3);

   XMSG_INFO0("JOIN-Test: Teil 1 ...");
   XMSG_INFO0(" Teil 2 ...");
   XMSG_INFO0(" Teil 3 ...");
   XMSG_INFO0(" Teil 4 ...");
   XMSG_INFO0(" Teil 5 ...\n... und dann noch Teil 6");
   XMSG_INFO0(" mit ende\n");

   XMSG_INFO0("LINE-Test:\n Teil 1 ...\n Teil 2 ...\n Teil 3 ...\n Teil 4 ...\n Teil 5 ...");
   XMSG_INFO0("\nENDE");
   XMSG_INFO0("\n");

   _XMEM_trace(1);
#if 0
#if IS_MSWIN
   getdynamicfunction("NtQueryInformationProcess::ntdll",1);
   getdynamicfunction("NtQueryInformationProcess::ntdll",1);
   CCI_Def_partition = getdynamicfunction("CCI_Def_partition::libmpcci-vc60-md-32",1);
#else
   CCI_Def_partition = getdynamicfunction("CCI_Def_partition::libmpcci-32",1);
#endif
   printf("P=%p\n",CCI_Def_partition);
   CCI_Def_partition(1,1);
#endif

   isshmeta('|');
   FTOUCH(TEXT("hallo"));
#if 0
   setworkdir(TEXT("hallo"));
#endif
   FEXISTS(TEXT("hallo"));
   fisdirectory(TEXT("hallo"));
   FISREADABLE(TEXT("hallo"));

   fxmodtime(TEXT("hallo"),NULL);

#if 0
   MILLISLEEP(1);
   SLEEPSECOND(1);
   SLEEPDOUBLE(1.0);
#endif

   numlen(10,10);
   str2frac("hallo",&l1,&l1);
   str2long("hallo",&l1);
   str2int("hallo",&i1);
   str2double("hallo",&d1);

   printf("socket-init\n");
   RSOCK_init();

   XENV_print(NULL,-1);
   p1 = (TCHAR *)XENV_get("abk");
   printf("XENV_get(abk): \"%s\"\n",(p1) ? p1 : "NULL");
#if XMSG_USE_PREFIX
   XMSG_SETPREFIX("[Sonstige] ");
#endif

   XENV_getint("MPCCI_DEBUG",&i);
   XENV_get("MPCCI_SERVER");

   printf("localhostname: %s\n", getlocalhostname(NULL));

   printf("waitdeleted\n")     ;  fwaitdeleted(TEXT("hallo"),-1);
   printf("waitexists\n")      ;  fwaitexists("test.c",-1);
   getbasename(TEXT("hallo"));
   mkdirname(TEXT("hallo"));
   getextname(TEXT("hallo"));
   gethomedir();
   getshell();
#if 0
   strxerror(10);
#endif
   getusername();
   getdatestring();
   getcpuseconds();
   getworkdir();

   pathrepair("hallo");
   scanpath(TEXT("hallo"),TEXT(".exe"),tstr,countof(tstr));
#if 0
   SSOCK_rpcall(sock,38,"f iiil *q T  : *d? i ",3.0f,1,1,1,(long)1, (size_t)128,qarr,"xhallo",100,NULL,&i1,&i2);
#endif

#if 0
   for(i=0; i<128;i++)
   {
      SOCKET sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      printf("Created socket %d.\n",i+1);
      if (IS_INVALID_SOCKET(sock)) printf("%d call of socket() failed.\n",i+1);
   }
#endif

   sock = SSOCK_connect("47111@daimler.scai.fhg.de",0,999,10000,NULL);
   strcpy(str, "2 RANK -1 IDSTR ABAQUS");

   size = strlen(str)+1;
   SSOCK_putraw(sock,str,size);
   SSOCK_putraw(sock,NULL,1024-size);
   SSOCK_getraw(sock,str,2048);
   printf("%s\n",str);
   printf("%s\n",str+1024);
   SSOCK_close(&sock);
   i1 = sscanf(str, "SOCKET %d %n", &i2, &i );
   printf("i1=%d\n",i1);
   XMSG_ASSERT(i1==1);

   XMSG_INFO2("Contacting my server %d@%s ...\n",i2,"daimler.scai.fhg.de");
   sock = SSOCK_connect("daimler.scai.fhg.de",i2,1000,10000,NULL);
   SSOCK_close(&sock);


   XENV_load(".hallorc");
   XENV_load(".hallorc");
   XENV_load(".hallorc");
   XENV_load(".hallorc");
   XENV_load(".hallorc");
   XENV_print(NULL,-1);
   return 0;
}
