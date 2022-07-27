#ifndef WSAWaitForMultipleEvents4K_SOURCE_INCLUDED
#define WSAWaitForMultipleEvents4K_SOURCE_INCLUDED
/* WSAWaitForMultipleEvents4K.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    WSAWaitForMultipleEvents() on a event list which may contain more than
 *    WSA_MAXIMUM_WAIT_EVENTS = 64 events.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2013/Jan/22: Carsten Dehning, Initial release
 *    $Id: WSAWaitForMultipleEvents4K.c 3742 2015-12-30 07:20:28Z dehning $
 *
 *****************************************************************************************
 */

typedef struct _WAITINFO
{
   const WSAEVENT *lphEvents;
   DWORD           cEvents;
   DWORD           dwTimeout;
   BOOL            fWaitAll;
   BOOL            fAlertable;
} WAITINFO;

static
DWORD WINAPI _wait_thread(WAITINFO *wi)
{
   return WSAWaitForMultipleEvents
          (
               wi->cEvents,
               wi->lphEvents,
               wi->fWaitAll,
               wi->dwTimeout,
               wi->fAlertable
          );
}


C_FUNC_PREFIX
DWORD WSAWaitForMultipleEvents4K
(
  __in  const DWORD     cEvents,
  __in  const WSAEVENT *lphEvents,
  __in  const BOOL      fWaitAll,
  __in  const DWORD     dwTimeout,
  __in  const BOOL      fAlertable
)
{
   const WSAEVENT *pevents;
   HANDLE          hthreadv[MAXIMUM_WAIT_OBJECTS];
   WAITINFO        waitinfov[MAXIMUM_WAIT_OBJECTS];
   DWORD           nevents,ngroups,ret,err;


   if (!lphEvents)
      goto EXIT_INVALID_DATA;

   //
   // Test for the trival job
   //
   if (cEvents <= WSA_MAXIMUM_WAIT_EVENTS)
   {
      if (!cEvents)
         goto EXIT_INVALID_DATA;
      return WSAWaitForMultipleEvents(cEvents,lphEvents,fWaitAll,dwTimeout,fAlertable);
   }

   //
   // Quick check on more than WSA_MAXIMUM_WAIT_EVENTS:
   // Check each event group of max. size WSA_MAXIMUM_WAIT_EVENTS with timeout=0.
   //
   nevents = cEvents;
   pevents = lphEvents;
   for(ngroups=0; nevents>0; ngroups++)
   {
      const DWORD n = (nevents < WSA_MAXIMUM_WAIT_EVENTS) ? nevents : WSA_MAXIMUM_WAIT_EVENTS;
      switch(ret=WSAWaitForMultipleEvents(n,pevents,fWaitAll,0,fAlertable))
      {
         case WSA_WAIT_TIMEOUT:
            break;

         case WSA_WAIT_IO_COMPLETION:
         case WSA_WAIT_FAILED:
            return ret;

         default:
            return (ret >= WSA_WAIT_EVENT_0 && ret < (WSA_WAIT_EVENT_0+n))
               ? ret + ngroups*WSA_MAXIMUM_WAIT_EVENTS
               : WSA_WAIT_FAILED;
      }
      pevents += n;
      nevents -= n;
   }

   if (!dwTimeout) // Great, no threads required
      return WSA_WAIT_TIMEOUT;

   //
   // The hard work begins here:
   // Max. is still limited to MAXIMUM_WAIT_OBJECTS threads
   //
   if (ngroups > MAXIMUM_WAIT_OBJECTS)
      goto EXIT_INVALID_DATA;

   nevents = cEvents;
   pevents = lphEvents;
   for(ngroups=0; nevents>0; ngroups++)
   {
      WAITINFO   *wi = waitinfov + ngroups;
      const DWORD n  = (nevents < WSA_MAXIMUM_WAIT_EVENTS) ? nevents : WSA_MAXIMUM_WAIT_EVENTS;
      wi->lphEvents  = pevents;
      wi->cEvents    = n;
      wi->fWaitAll   = fWaitAll;
      wi->dwTimeout  = dwTimeout;
      wi->fAlertable = fAlertable;
      hthreadv[ngroups] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)_wait_thread,wi,0,NULL);

      pevents += n;
      nevents -= n;
   }

   //
   // Now wait for all of the waiting threads
   //
   switch(ret=WaitForMultipleObjects(ngroups,hthreadv,fWaitAll,dwTimeout))
   {
      case WAIT_TIMEOUT:
         ret = WSA_WAIT_TIMEOUT;
         break;

      case WAIT_FAILED:
         ret = WSA_WAIT_FAILED;
         break;

      default:
      {
         const DWORD n = ret - WAIT_OBJECT_0;
         if (GetExitCodeThread(hthreadv[n],&ret))
         {
            if (ret >= WSA_WAIT_EVENT_0 && ret < (WSA_WAIT_EVENT_0+WSA_MAXIMUM_WAIT_EVENTS))
            {
               ret += n*WSA_MAXIMUM_WAIT_EVENTS;
            }
         }
         break;
      }
   }

   //
   // Close all threads, but forget any error with terminate & close
   // and keep the original error code.
   //
   err = GetLastError();
   while (ngroups-- > 0)
   {
      TerminateThread(hthreadv[ngroups],WSA_WAIT_TIMEOUT);
      CloseHandle(hthreadv[ngroups]);
   }
   SetLastError(err);
   return ret;


EXIT_INVALID_DATA:
   WSASetLastError(ERROR_INVALID_DATA);
   return WSA_WAIT_FAILED;
}

#endif
