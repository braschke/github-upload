#pragma once
#ifndef getmacaddrv_SOURCE_INCLUDED
#define getmacaddrv_SOURCE_INCLUDED
/* getmacaddrv.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns a NULL terminated vector of MAC addresses available on this machine
 *       mac_addrv[0  ] -> points to MAC string 1
 *       mac_addrv[1  ] -> points to MAC string 2 or NULL
 *       ...
 *       mac_addrv[n-1] -> points to MAC string n or NULL
 *       mac_addrv[n  ] -> always NULL
 *       MAC string 1  (18 * sizeof(TCHAR)
 *       MAC string 2  (18 * sizeof(TCHAR)
 *       ....
 *
 *    Memory is allocated in a single block: just call FREE(mac_addrv) after usage.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2011/Mar/02: Carsten Dehning, Initial release
 *    $Id: getmacaddrv.c 4428 2016-05-14 08:57:21Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"
#include "xmsg.h"
#include "xmem.h"

#if INCLUDE_STATIC
   #include "bytes2hex.c"
#endif

/*
 * Allocate the address buffer  TCHAR *mac_addr[n+1] buffers at once.
 * The head of the buffer contains n+1 pointers to the MAC strings, which in fact are
 * stored at the second part of the buffer as a series of 0-terminated strings.
 */
#define ALLOC_ADDRV(_n)\
{\
   char *head;\
   size_t msize;\
   msize     = (_n+1)*sizeof(TCHAR *) + /* _n+1 address pointers, the last is NULL */\
               (_n  )*18*sizeof(TCHAR); /* _n MAC strings, each 6*2(bytes) + 5(hyphen) + 1x'\0' bytes long */\
   head      = (char *)MALLOC(msize);  MEMZERO(head,msize);\
   mac_addrv = SCAST_INTO(TCHAR **,head); /* the address vector */\
   head     += (_n+1)*sizeof(TCHAR *);    /* the head of the strings buffer behind the mac_addrv[n+1] */\
   mac_addrv[0] = mac_head = SCAST_INTO(TCHAR *,head);\
}

/****************************************************************************************/

#if IS_MSWIN

   #include <iphlpapi.h>
   #pragma comment(lib,"iphlpapi")
   #include "xmem.h"

   #if INCLUDE_STATIC
      #include "getipadapters.c"
   #endif

   C_FUNC_PREFIX TCHAR **getmacaddrv(const int hyphen, const int upcase)
   {
      const IP_ADAPTER_ADDRESSES *aa;
      TCHAR  **mac_addrv;
      TCHAR   *mac_head;
      unsigned nfound = 0;


      /* Allocate a max. of 10 MAC address'es */
      ALLOC_ADDRV(10)
      for(aa=getipadapters(0); aa && nfound<10; aa=aa->Next)
      {
         switch(aa->IfType)
         {
            case IF_TYPE_ETHERNET_CSMACD:
            case IF_TYPE_IEEE80211:
            case IF_TYPE_TUNNEL:
               if ( (aa->PhysicalAddress)[0] != 0  /* IP="0.0.0.0" indicates a deactivated adapter */
                  && aa->PhysicalAddressLength == 6)
               {
                  unsigned i;

                  bytes2hex(aa->PhysicalAddress,6,upcase,hyphen,mac_head);

                  /*
                   * Under MSWin we may detect devices with the identical MAC.
                   * Avoid doubled address strings in the list.
                   */
                  for(i=0; i<nfound; i++)
                  {
                     if (!_tcscmp(mac_addrv[i],mac_head))
                        break;
                  }

                  if (i>=nfound)
                  {
                     /* Store the new MAC address string */
                     mac_addrv[nfound++] = mac_head;
                     mac_head += 3*6;
                  }
               }
               break;

            default: /* Not an  ethernet device */
               break;
         }
      }

      if (nfound) return mac_addrv;
      FREE(mac_addrv);
      return NULL;
   }

/****************************************************************************************/

#elif defined(IS_LINUX)

   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <sys/ioctl.h>
   #include <net/if.h>

   #if INCLUDE_STATIC
      #include "getipadapters.c"
   #endif

   C_FUNC_PREFIX TCHAR **getmacaddrv(const int hyphen, const int upcase)
   {
      const struct ifaddrs *ifa;
      TCHAR     **mac_addrv;
      TCHAR      *mac_head;
      const char *last_ifname = "";
      unsigned    nfound = 0;
      int         sock;


      if ((sock=socket(AF_INET6,SOCK_DGRAM,IPPROTO_IP)) < 0 &&
          (sock=socket(AF_INET ,SOCK_DGRAM,IPPROTO_IP)) < 0)
      {
         XMSG_FATAL1
         (
            "getmacaddrv: Can\'t create socket: %s.\n",
            strerror(errno)
         );
         return NULL;
      }

      /* Allocate a max. of 10 MAC address'es */
      ALLOC_ADDRV(10)
      for(ifa=getipadapters(0); ifa && nfound<10; ifa=ifa->ifa_next)
      {
         if (ifa->ifa_addr
               && STRHASLEN(ifa->ifa_name)
               && (ifa->ifa_flags &IFF_LOOPBACK) == 0
               && strcmp(last_ifname,ifa->ifa_name))
         {
            struct ifreq ifr;
            MEMZERO(&ifr,sizeof(ifr));
            strcpy(ifr.ifr_name,ifa->ifa_name);
            if (!ioctl(sock,SIOCGIFHWADDR,&ifr))
            {
               mac_addrv[nfound++] = mac_head;
               bytes2hex(ifr.ifr_addr.sa_data,6,upcase,hyphen,mac_head);
               /*printf("MAC(%s): '%s'\n",ifa->ifa_name,mac_head);*/
               mac_head += 3*6;
               last_ifname = ifa->ifa_name;
            }
         }
      }

      close(sock);
      if (nfound) return mac_addrv;
      FREE(mac_addrv);
      return NULL;
   }

/****************************************************************************************/

#elif defined(IS_SUNOS)

   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <sys/ioctl.h>
   #include <net/if.h>
   #include <sys/sockio.h>
   C_FUNC_PREFIX TCHAR **getmacaddrv(const int hyphen, const int upcase)
   {
      struct ifreq ifr;
      TCHAR **mac_addrv;
      TCHAR  *mac_head;
      int     sock,ret;


      if ((sock=socket(AF_INET6,SOCK_DGRAM,IPPROTO_IP)) < 0 &&
          (sock=socket(AF_INET ,SOCK_DGRAM,IPPROTO_IP)) < 0)
      {
         XMSG_FATAL1
         (
            "getmacaddrv: Can\'t create socket: %s.\n",
            strerror(errno)
         );
         return NULL;
      }

      MEMZERO(&ifr,sizeof(ifr));
      strcpy(ifr.ifr_name,"ce0");
      ret = ioctl(sock,SIOCGIFADDR,&ifr);
      close(sock);
      if (ret) /* *FAIL* */
         return NULL;

      ALLOC_ADDRV(1)
      bytes2hex(ifr.ifr_addr.sa_data,6,upcase,hyphen,mac_head);
      return mac_addrv;
   }

/****************************************************************************************/

#elif defined(IS_AIX)

   #include <sys/ndd_var.h>
   #include <sys/kinfo.h>
   C_FUNC_PREFIX TCHAR **getmacaddrv(const int hyphen, const int upcase)
   {
      struct kinfo_ndd nddp[10];
      unsigned char *addr;
      TCHAR **mac_addrv;
      TCHAR  *mac_head;
      int     size = (int)sizeof(nddp);


      if (getkerninfo(KINFO_NDD,&nddp,&size,0) < 0)
      {
         XMSG_FATAL1
         (
            "getmacaddrv: Can\'t get kerninfo: %s.\n",
            strerror(errno)
         );
         return NULL;
      }

      ALLOC_ADDRV(1)
      bytes2hex(nddp[0].ndd_addr,6,upcase,hyphen,mac_head);
      return mac_addrv;
   }

/****************************************************************************************/

#elif defined(IS_HPUX11)

   /*
    * Use the Data Link Provider Interface
    */
   #include <stropts.h>
   #include <fcntl.h>
   #include<sys/types.h>
   #include<sys/dlpi.h>

   static int _get_dlpi_mac(int fd, int ppa, char addrbuf[6])
   {

      struct strbuf  ctl;
      struct strbuf  data;
      int            fl;
      unsigned char  buf[4096];


      {
         dl_ok_ack_t    *doa = (dl_ok_ack_t *)buf;
         dl_attach_req_t dar;

         dar.dl_primitive  =  DL_ATTACH_REQ;
         dar.dl_ppa        =  ppa;
         ctl.buf           =  (char *)&dar;
         ctl.maxlen        =
         ctl.len           =  sizeof(dar);
         if (putmsg(fd,&ctl,0,0) < 0)
         {
            perror("putmsg(DL_ATTACH_REQ)");
            return -1;
         }

         ctl.buf     =  (char*)&buf;
         ctl.maxlen  =
         ctl.len     =  sizeof(buf);

         data.len    =  0;
         data.maxlen =  sizeof(buf);
         data.buf    =  (char*)&buf;

         fl          =  RS_HIPRI;
         if (getmsg(fd,&ctl,&data,&fl) < 0)
         {
            perror("getmsg");
            return -1;
         }


         if (doa->dl_primitive != DL_OK_ACK)
         {
            #if 0
               dl_error_ack_t *dea = (dl_error_ack_t *)buf;

               if (dea->dl_primitive == DL_ERROR_ACK)
                  printf
                  (
                     "error: ppa=%d dl_error=%d dl_unix_errno=%d\n",
                     ppa,
                     dea->dl_errno,
                     dea->dl_unix_errno
                  );
               else
                  printf
                  (
                     "error: ppa=%d primitive=%x\n",
                     ppa,
                     dea->dl_primitive
                  );
            #endif

            return -1;
         }
      }

      {
         dl_info_ack_t *dia = (dl_info_ack_t *)buf;
         dl_info_req_t  dir;

         dir.dl_primitive  =  DL_INFO_REQ;
         ctl.buf           =  (char *)&dir;
         ctl.maxlen        =
         ctl.len           =  sizeof(dir);
         if (putmsg(fd,&ctl,0,0) < 0)
         {
            perror("putmsg(DL_INFO_REQ)");
            return -1;
         }

         ctl.buf     =  (char*)&buf;
         ctl.maxlen  =
         ctl.len     =  sizeof(buf);
         data.len    =
         data.maxlen =  0;
         data.buf    =  0;
         fl          =  0;
         if (getmsg(fd,&ctl,&data,&fl) < 0)
         {
            perror("getmsg");
            return -1;
         }

         if(dia->dl_primitive != DL_INFO_ACK)
         {
            printf("error! primitive=%d\n",dia->dl_primitive);
            return -1;
         }

         if (dia->dl_addr_length != 6)
            return -1;

         memcpy(addrbuf,buf+dia->dl_addr_offset,6);
      }

      return 0;
   }

   C_FUNC_PREFIX TCHAR **getmacaddrv(const int hyphen, const int upcase)
   {
      TCHAR **mac_addrv;
      TCHAR  *mac_head;
      int     fd,nfound,ppa;


      if ((fd=open("/dev/dlpi",O_RDWR)) < 0)
      {
         XMSG_FATAL2
         (
            "getmacaddrv: Can\'t open device \"%s\": %s.\n",
            "/dev/dlpi",
            strerror(errno)
         );
         return NULL;
      }

      /* allocate a max. of 10 MAC address'es */
      ALLOC_ADDRV(10)
      for(nfound=ppa=0; ppa<10; ppa++)
      {
         char addrbuf[6];
         if (!_get_dlpi_mac(fd,ppa,addrbuf))
         {
            mac_addrv[nfound++] = mac_head;
            bytes2hex(addrbuf,6,upcase,hyphen,mac_head);
            mac_head += 3*6;
         }
      }
      close(fd);
      if (nfound) return mac_addrv;
      FREE(mac_addrv);
      return NULL;
   }

/****************************************************************************************/

#elif defined(IS_OSFALPHA)

   C_FUNC_PREFIX TCHAR **getmacaddrv(conbst int hyphen, const int upcase)
   {
      if ((hyphen||upcase) || !(hyphen||upcase)) /* keep compiler happy */
      XMSG_FATAL0("getmacaddrv: Not implemented.\n");
      return NULL;
   }

/****************************************************************************************/

#elif defined(IS_IRIX65)

   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <sys/ioctl.h>
   #include <net/if.h>
   C_FUNC_PREFIX TCHAR **getmacaddrv(const int hyphen, const int upcase)
   {
      struct ifreq ifr;
      TCHAR **mac_addrv;
      TCHAR  *mac_head;
      int     sock,ret;


      if ((sock=socket(AF_INET6,SOCK_DGRAM,IPPROTO_IP)) < 0 &&
          (sock=socket(AF_INET ,SOCK_DGRAM,IPPROTO_IP)) < 0)
      {
         XMSG_FATAL1
         (
            "getmacaddrv: Can\'t create socket: %s.\n",
            strerror(errno)
         );
         return NULL;
      }

      MEMZERO(&ifr,sizeof(ifr));
      strcpy(ifr.ifr_name,"ef0");
      ret = ioctl(sock,SIOCGIFADDR,&ifr);
      close(sock);
      if (ret) /* *FAIL* */
         return NULL;

      ALLOC_ADDRV(1)
      bytes2hex(ifr.ifr_addr.sa_data,6,upcase,hyphen,mac_head);
      return mac_addrv;
   }

/****************************************************************************************/

#elif defined(IS_MACOSX) /* BSD type MAC OS X */

   #include <sys/types.h>
   #include <sys/socket.h>
   #include <net/if_dl.h>
   #include <ifaddrs.h>

   #if INCLUDE_STATIC
      #include "getipadapters.c"
   #endif

   C_FUNC_PREFIX TCHAR **getmacaddrv(const int hyphen, const int upcase)
   {
      const struct ifaddrs *ifa;
      TCHAR  **mac_addrv;
      TCHAR   *mac_head;
      unsigned nfound = 0;


      /* allocate a max. of 10 MAC addresses */
      ALLOC_ADDRV(10)
      for (ifa=getipadapters(0); ifa && nfound<10; ifa=ifa->ifa_next)
      {
         if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK)
         {
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)ifa->ifa_addr;
            if (sdl->sdl_alen == 6)
            {
               mac_addrv[nfound++] = mac_head;
               bytes2hex(LLADDR(sdl),6,upcase,hyphen,mac_head);
               mac_head += 3*6;
            }
         }
      }
      if (nfound) return mac_addrv;
      FREE(mac_addrv);
      return NULL;
   }

/****************************************************************************************/

#else

   #error getmacaddrv: Unknown compile platform

#endif

/****************************************************************************************/

#undef ALLOC_ADDRV


#if 0


/*
 * http://cplus.kompf.de/artikel/macaddr.html
 *
 * mac_addr_dlpi.c
 *
 * Return the MAC (ie, ethernet hardware) address by using the dlpi api.
 *
 * compile with: gcc -c -D "OS" mac_addr_dlpi.c
 * with "OS" is one of AIX, SunOS, HPUX
 */

/***********************************************************************/
/* this section defines a list of the dlpi capable devices
 * this depends on the operating system
 */

#undef DLPI_DEV

#ifdef HPUX
static char *dlpi_dev[] = {"/dev/dlpi", ""};
#define DLPI_DEV
#endif

#ifdef AIX
static char *dlpi_dev[] = {"/dev/dlpi/et", "/dev/dlpi/en", "/dev/dlpi/tr", "/dev/dlpi/fddi", ""};
#define DLPI_DEV
/* AIX: remember to set up /etc/pse.conf or /etc/dlpi.conf */
#endif

#ifdef SunOS
static char *dlpi_dev[] = {"/dev/eri", "/dev/hme", "/dev/ie", "/dev/le", ""};
#define DLPI_DEV
#endif

#ifndef DLPI_DEV
static char *dlpi_dev[] = {"/dev/dlpi", ""};
/* unknown OS - hope that this will work ??? */
#define DLPI_DEV
#endif

/***********************************************************************/
/*
 * implementation
 */

#define INSAP 22
#define OUTSAP 24

#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stropts.h>
#include <sys/poll.h>
#include <sys/dlpi.h>

#define bcopy(source, destination, length) memcpy(destination, source, length)

#define AREA_SZ 5000 /*�=�* buffer length in bytes *�=�*/
static u_long ctl_area[AREA_SZ];
static u_long dat_area[AREA_SZ];
static struct strbuf ctl = {AREA_SZ, 0, (char *)ctl_area};
static struct strbuf dat = {AREA_SZ, 0, (char *)dat_area};
#define GOT_CTRL 1
#define GOT_DATA 2
#define GOT_BOTH 3
#define GOT_INTR 4
#define GOT_ERR 128

/*�=�* get a message from a stream; return type of message *�=�*/
static int get_msg(int fd)
{
    int flags = 0;
    int res, ret;
    ctl_area[0] = 0;
    dat_area[0] = 0;
    ret = 0;
    res = getmsg(fd, &ctl, &dat, &flags);
    if(res < 0) {
        if(errno == EINTR) {
            return(GOT_INTR);
        } else {
            return(GOT_ERR);
        }
    }
    if(ctl.len > 0) {
        ret |= GOT_CTRL;
    }
    if(dat.len > 0) {
        ret |= GOT_DATA;
    }
    return(ret);
}

/*�=�* verify that dl_primitive in ctl_area = prim *�=�*/
static int check_ctrl(int prim)
{
    dl_error_ack_t *err_ack = (dl_error_ack_t *)ctl_area;
    if(err_ack->dl_primitive != prim) {
        return GOT_ERR;
    }
    return 0;
}

/*�=�* put a control message on a stream *�=�*/
static int put_ctrl(int fd, int len, int pri)
{
    ctl.len = len;
    if(putmsg(fd, &ctl, 0, pri) < 0) {
        return GOT_ERR;
    }
    return  0;
}

/*�=�* put a control + data message on a stream *�=�*/
static int put_both(int fd, int clen, int dlen, int pri)
{
    ctl.len = clen;
    dat.len = dlen;
    if(putmsg(fd, &ctl, &dat, pri) < 0) {
        return GOT_ERR;
    }
    return  0;
}

/*�=�* open file descriptor and attach *�=�*/
static int dl_open(const char *dev, int ppa, int *fd)
{
    dl_attach_req_t *attach_req = (dl_attach_req_t *)ctl_area;
    if((*fd = open(dev, O_RDWR)) == -1) {
        return GOT_ERR;
    }
    attach_req->dl_primitive = DL_ATTACH_REQ;
    attach_req->dl_ppa = ppa;
    put_ctrl(*fd, sizeof(dl_attach_req_t), 0);
    get_msg(*fd);
    return check_ctrl(DL_OK_ACK);
}

/*�=�* send DL_BIND_REQ *�=�*/
static int dl_bind(int fd, int sap, u_char *addr)
{
    dl_bind_req_t *bind_req = (dl_bind_req_t *)ctl_area;
    dl_bind_ack_t *bind_ack = (dl_bind_ack_t *)ctl_area;
    bind_req->dl_primitive = DL_BIND_REQ;
    bind_req->dl_sap = sap;
    bind_req->dl_max_conind = 1;
    bind_req->dl_service_mode = DL_CLDLS;
    bind_req->dl_conn_mgmt = 0;
    bind_req->dl_xidtest_flg = 0;
    put_ctrl(fd, sizeof(dl_bind_req_t), 0);
    get_msg(fd);
    if (GOT_ERR == check_ctrl(DL_BIND_ACK)) {
        return GOT_ERR;
    }
    bcopy((u_char *)bind_ack + bind_ack->dl_addr_offset, addr,
        bind_ack->dl_addr_length);
    return 0;
}

/***********************************************************************/
/*
 * interface:
 * function mac_addr_dlpi - get the mac address of the "first" interface
 *
 * parameter: addr: an array of six bytes, has to be allocated by the caller
 *
 * return: 0 if OK, -1 if the address could not be determined
 *
 */

long mac_addr_dlpi ( u_char  *addr)
{
    int fd;
    int ppa;
    u_char mac_addr[25];
    int i;

    char **dev;

    for (dev = dlpi_dev; **dev != '?'; ++dev) {
        for (ppa=0; ppa<10; ++ppa) {
            if (GOT_ERR != dl_open(*dev, ppa, &fd)) {
                if (GOT_ERR != dl_bind(fd, INSAP, mac_addr)) {
                    bcopy( mac_addr, addr, 6);
                    return 0;
                }
            }
            close(fd);
        }
    }
    return -1;
}

#endif


#if 0

vai DLPI for HPUX11

#include<sys/types.h>
#include<sys/dlpi.h>
#include <stropts.h>
#include <fcntl.h>

main(c,v)
int c;
char **v;
{
   int f;
   dl_attach_req_t   dar;
   dl_info_req_t     dir;
   struct strbuf     ctl;
   struct strbuf     data;
   unsigned char     buf[4096];
   dl_ok_ack_t      *doa = (dl_ok_ack_t   *)buf;
   dl_info_ack_t    *dia = (dl_info_ack_t *)buf;
   dl_error_ack_t   *dea = (dl_info_ack_t *)buf;
   int r;
   int fl=0;
   int prim;

   f=open("/dev/dlpi",O_RDWR);


   dar.dl_primitive=DL_ATTACH_REQ;
   dar.dl_ppa=atoi(v[1]);;
   ctl.buf=(char*)&dar;
   ctl.maxlen=ctl.len=sizeof(dar);
   putmsg(f,&ctl,0,0);

   ctl.buf=(char*)&buf;
   ctl.maxlen=ctl.len=sizeof(buf);
   data.len=0;
   data.maxlen=sizeof(buf);
   data.buf=&buf;
   fl=RS_HIPRI;
   r=getmsg(f,&ctl,&data,&fl);
   if(r<0) perror("getmsg");

   if((prim=doa->dl_primitive)!=DL_OK_ACK)
   {
      if (dea->dl_primitive==DL_ERROR_ACK)
         printf("erreur: ppa=%d dl_error=%d dl_unix_errno=%d\n",atoi(v[1]),dea->dl_errno,dea->dl_unix_errno);
      else
         printf("erreur: ppa=%d primitive=%x\n",atoi(v[1]),dea->dl_primitive);
      exit(1);
   }

   dir.dl_primitive=DL_INFO_REQ;
   ctl.buf=(char*)&dir;
   ctl.maxlen=ctl.len=sizeof(dir);
   putmsg(f,&ctl,0,0);

   ctl.buf=(char*)&buf;
   ctl.maxlen=ctl.len=sizeof(buf);
   data.len=data.maxlen=0;
   data.buf=0;
   fl=0;
   r=getmsg(f,&ctl,&data,&fl);
   if(r<0) perror("getmsg");

   dia=buf;

   if(dia->dl_primitive==DL_INFO_ACK)
   {
      int i;
      int l;
      int o;
      char sep='=';
      unsigned char*c;
      l=dia->dl_addr_length;
      o=dia->dl_addr_offset;
      printf("interface %d mac addr",atoi(v[1]));
      c=buf+o;
      for(i=0;i<l;i++)
      {
         printf("%c%02x",sep,c[i]);
         sep=':';
      }
      printf("\n");
   }
   else
      printf("error! primitive=%d\n",dia->dl_primitive);
}

#endif

#endif
