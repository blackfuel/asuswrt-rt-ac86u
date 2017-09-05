/*
* <:copyright-BRCM:2007-2010:proprietary:standard
* 
*    Copyright (c) 2007-2010 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>

*/

#include "cms.h"
#include "cms_util.h"
#include "cms_msg.h"

#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <net/if.h>
#include <signal.h>

#define ND_OPT_RDNSS  25 /* RFC 6106 */
#define ND_OPT_DNSSL 31 /* RFC 6106 */

SINT32 raMonitorFd;
static int sigterm_received = 0;
void * msgHandle=NULL;

// brcm
#ifdef __linux__
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,0,0)) && !(defined(__USE_GNU) && (defined __UCLIBC_HAS_IPV6__ || !defined __UCLIBC_STRICT_HEADERS__))
/* from linux/net/ipv6.h */
struct in6_pktinfo {
     struct in6_addr ipi6_addr;
     int ipi6_ifindex;
};
#endif
#endif

         
static CmsRet parsepio(struct nd_opt_prefix_info *pio, UINT8 len, 
                       RAStatus6MsgBody *ramsg)
{
   if (inet_ntop(AF_INET6, &pio->nd_opt_pi_prefix, ramsg->pio_prefix,
                  CMS_IPADDR_LENGTH) == NULL)
   {
      return CMSRET_INVALID_PARAM_VALUE;
   }

   ramsg->pio_prefixLen = pio->nd_opt_pi_prefix_len;
   ramsg->pio_L_flag = pio->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_ONLINK;
   ramsg->pio_A_flag = pio->nd_opt_pi_flags_reserved & ND_OPT_PI_FLAG_AUTO;
   ramsg->pio_plt = ntohl(pio->nd_opt_pi_preferred_time);
   ramsg->pio_vlt = ntohl(pio->nd_opt_pi_valid_time);

   return CMSRET_SUCCESS;
}

static CmsRet parserdnss(const UINT8 *dns, UINT8 len, RAStatus6MsgBody *ramsg)
{
   /*
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |     Type      |     Length    |           Reserved            |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                           Lifetime                            |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                                                               |
     :            Addresses of IPv6 Recursive DNS Servers            :
     |                                                               |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */
   int num;
   char *msgptr;
   const UINT8 *pktptr;

   pktptr = dns;
   cmsLog_debug("rdnss: len<%d>", len);

   /* number of addresses in the message is (len-1)/2 */
   num = (len -1)/2;
   if (num < 1)
   {
      cmsLog_debug("there must be at least one DNS server in dnss msg");
      return CMSRET_INVALID_PARAM_VALUE;
   }

   pktptr += 8;
   msgptr = ramsg->dns_servers;
   /* TODO: we only support at most two dns servers */
   if (num > 2)
   {
      cmsLog_debug("more than 2 dns servers in dnss msg");
      num = 2;
   }

   do
   {
      if (inet_ntop(AF_INET6, pktptr, msgptr, CMS_IPADDR_LENGTH) == NULL)
      {
         return CMSRET_INVALID_PARAM_VALUE;
      }
      
      num--;
      pktptr += sizeof (struct in6_addr);
      msgptr += strlen(msgptr);

      if (num == 1)
      {
         *(msgptr++) = ',';
      }

   } while(num);

   ramsg->dns_lifetime = *((uint32_t *)dns + 1);

   return CMSRET_SUCCESS;
}

#if 0 //TODO
static CmsRet parserdnssl(const UINT8 *domain, UINT8 len, RAStatus6MsgBody *ramsg)
{
   /*
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |     Type      |     Length    |           Reserved            |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                           Lifetime                            |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     |                                                               |
     :                Domain Names of DNS Search List                :
     |                                                               |
     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */
   int num;
   char *msgptr;
   const UINT8 *pktptr;

   pktptr = domain;
   cmsLog_debug("dnssl: len<%d>", len);

   if (len < 2)
   {
      cmsLog_debug("there must be at least one domain in dnssl msg");
      return CMSRET_INVALID_PARAM_VALUE;
   }

   pktptr += 8;
   msgptr = ramsg->domainName;
   /* TODO: we only support one domain name up to 32 characters */




   return CMSRET_SUCCESS;
}
#endif

CmsRet initRAMonitorFd()
{
   struct icmp6_filter filter;
   int val;

   if ( (raMonitorFd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0 )
   {
       cmsLog_error("Could not open socket for RA monitor");
       return CMSRET_INTERNAL_ERROR;
   }

   /*
    * Ask for ancillary data with each ICMPv6 packet so we can get
    * the incoming interface name.
    */
   val = 1;
   if ( setsockopt(raMonitorFd, IPPROTO_IPV6, IPV6_RECVPKTINFO, &val,
                   sizeof(val)) < 0 )
   {
       cmsLog_error("could not set option to get ifname");
       return CMSRET_INTERNAL_ERROR;
   }

   /* fetch router advertisement only */
   ICMP6_FILTER_SETBLOCKALL(&filter);
   ICMP6_FILTER_SETPASS(ND_ROUTER_ADVERT, &filter);

   if ( setsockopt(raMonitorFd, IPPROTO_ICMPV6, ICMP6_FILTER, &filter,
                   sizeof(filter)) < 0 )
   {
       cmsLog_error("could not set filter for RA");
       return CMSRET_INTERNAL_ERROR;
   }

   return CMSRET_SUCCESS;
}


void cleanupRAMonitorFd()
{
   if ( raMonitorFd != CMS_INVALID_FD )
   {
      close(raMonitorFd);
   }
}


CmsRet processRAMonitor(RAStatus6MsgBody *ramsg)
{
   char buf[BUFLEN_1024], ifName[BUFLEN_32];
   int buflen;
   char ancbuf[CMSG_SPACE(sizeof (struct in6_pktinfo)) ];
   struct iovec iov;
   struct msghdr msg;
   struct cmsghdr *cmsgp;
   struct in6_pktinfo *pktinfo = NULL;
   struct sockaddr_in6 src;
   char gwAddr[CMS_IPADDR_LENGTH];          
   struct nd_router_advert *ra;
   UINT8 *data_p;
   int len, m_flag, o_flag, router_lifetime;
   CmsRet ret = CMSRET_SUCCESS;
    
   iov.iov_len = sizeof(buf);
   iov.iov_base = (void *)buf;

   memset(&msg, 0, sizeof(msg));
   msg.msg_name = (void *)&src;
   msg.msg_namelen = sizeof(src);
   msg.msg_iov = &iov;
   msg.msg_iovlen = 1;
   msg.msg_control = (void *)ancbuf;
   msg.msg_controllen = sizeof(ancbuf);

   if ( (buflen = recvmsg(raMonitorFd, &msg, 0)) < 0 )
   {
      cmsLog_error("read error on raw socket");
      return CMSRET_INTERNAL_ERROR;
   }

   for (cmsgp = CMSG_FIRSTHDR(&msg); cmsgp != NULL;
        cmsgp = CMSG_NXTHDR(&msg, cmsgp))
   {
      if (cmsgp->cmsg_len == 0)
      {
         cmsLog_error("ancillary data with zero length");
         return CMSRET_INTERNAL_ERROR;
      }

      if ((cmsgp->cmsg_level == IPPROTO_IPV6) && 
          (cmsgp->cmsg_type == IPV6_PKTINFO))
      {
         pktinfo = (struct in6_pktinfo *)CMSG_DATA(cmsgp);
      }
   }

   if ( pktinfo != NULL )
   {
      if ( if_indextoname(pktinfo->ipi6_ifindex, ifName) == NULL )
      {
         cmsLog_error("couldn't find interface name: index %d", 
                      pktinfo->ipi6_ifindex);
         return CMSRET_INTERNAL_ERROR;
      }
   }
   else
   {
      cmsLog_error("couldn't get ancillary data for packet");
      return CMSRET_INTERNAL_ERROR;
   }

   if (inet_ntop(AF_INET6, &src.sin6_addr, gwAddr, CMS_IPADDR_LENGTH)==NULL)
   {
      cmsLog_error("invalid IPv6 address??");
      return CMSRET_INTERNAL_ERROR;
   }

   /* parse the RA for detailed info */
   data_p = (UINT8 *)buf;
   len = buflen;
    
   ra = (struct nd_router_advert *)data_p;

   if (ra->nd_ra_type != ND_ROUTER_ADVERT)
   {
      cmsLog_debug("Not RA");
      return CMSRET_INTERNAL_ERROR;
   }

   m_flag = ra->nd_ra_flags_reserved & ND_RA_FLAG_MANAGED;
   o_flag = ra->nd_ra_flags_reserved & ND_RA_FLAG_OTHER;
   router_lifetime = ntohs(ra->nd_ra_router_lifetime);

   cmsLog_debug("RA with M<%d> O<%d> lifetime<%d> from<%s> at <%s>", 
                m_flag, o_flag, router_lifetime, gwAddr, ifName);
    
   /* fetch prefix info */
   data_p += sizeof (struct nd_router_advert);
   len -= sizeof (struct nd_router_advert);

   /* RFC 4861: options will be always at 64-bit boundaries */
   while (len >= 8)
   {
      UINT8 optlen;
      UINT8 pio_cnt = 0;

      optlen = data_p[1];
      if ( optlen == 0 )
      {
         cmsLog_debug("option with zero length");
         break;
      }

      switch (data_p[0])
      {
         case ND_OPT_PREFIX_INFORMATION:
            if (pio_cnt != 0)
            {
               cmsLog_debug("Do not support 2nd PIO in RA");
               break;
            }

            if ((ret = parsepio((struct nd_opt_prefix_info *)data_p, 
                           optlen, ramsg)) != CMSRET_SUCCESS)
            {
               cmsLog_error("Cannot parse prefix option");
               return ret;
            }
            else
            {
               pio_cnt++;
            }

            break;

         case ND_OPT_RDNSS:
            if (router_lifetime)
            {
               if ((ret = parserdnss(data_p, 
                              optlen, ramsg)) != CMSRET_SUCCESS)
               {
                  cmsLog_error("Cannot parse rdnss option");
                  return ret;
               }
            }
            break;

#if 0 //TODO
         case ND_OPT_DNSSL:
            if (router_lifetime)
            {
               if ((ret = parserdnssl(data_p, 
                              optlen, ramsg)) != CMSRET_SUCCESS)
               {
                  cmsLog_error("Cannot parse rdnss option");
                  return ret;
               }
            }
            break;
#endif

         default:
               cmsLog_debug("option type<%d>", data_p[0]);
               break;
      }

      data_p += (optlen*8);
      len -= (optlen*8);
   }

   cmsLog_debug("PIO with L<%d> A<%d> plt<%u> vlt<%u> prefix<%s/%u>", 
                ramsg->pio_L_flag, ramsg->pio_A_flag, ramsg->pio_plt, 
                ramsg->pio_vlt, ramsg->pio_prefix, ramsg->pio_prefixLen);

   cmsUtl_strncpy(ramsg->router, gwAddr, BUFLEN_40);
   ramsg->router_lifetime = router_lifetime;
   ramsg->router_M_flags = m_flag;
   ramsg->router_O_flags = o_flag;
   cmsUtl_strncpy(ramsg->ifName, ifName, sizeof(ramsg->ifName));

   return ret;
}

void sendRAInfoEventMessage(RAStatus6MsgBody *raInfo)
{
   char buf[sizeof(CmsMsgHeader) + sizeof(RAStatus6MsgBody)]={0};
   CmsMsgHeader *msg=(CmsMsgHeader *) buf;
   RAStatus6MsgBody *raStatus6MsgBody = (RAStatus6MsgBody *) (msg+1);
   CmsRet ret;

   msg->type = CMS_MSG_RASTATUS6_INFO;
   msg->src = MAKE_SPECIFIC_EID(getpid(), EID_RASTATUS6);
   msg->dst = EID_SSK;
   msg->flags_event = 1;
   msg->dataLength = sizeof(RAStatus6MsgBody);

   memcpy(raStatus6MsgBody, raInfo, sizeof(RAStatus6MsgBody));

   if ((ret = cmsMsg_send(msgHandle, msg)) != CMSRET_SUCCESS)
   {
      cmsLog_error("could not send out CMS_MSG_RASTATUS6_INFO, ret=%d", ret);
   }
   else
   {
      cmsLog_notice("sent out CMS_MSG_RASTATUS6_INFO");
   }

   return;
}

void sigterm_handler()
{
   sigterm_received++;
}

SINT32 main(SINT32 argc, char *argv[])
{
   CmsRet ret;
   SINT32 n;
   fd_set readFdsMaster, readFds;
   RAStatus6MsgBody raStatus6MsgBody;

   if ((ret = cmsMsg_init(EID_RASTATUS6, &msgHandle)) != CMSRET_SUCCESS)
   {
      cmsLog_error("msg initialization failed, ret=%d", ret);
      return -1;
   }

   raMonitorFd = CMS_INVALID_FD;

   if ((initRAMonitorFd() != CMSRET_SUCCESS) || (raMonitorFd == CMS_INVALID_FD))
   {
      cmsLog_error("initRAMonitorFd failed");
      cleanupRAMonitorFd();
      return -1;
   }

   signal(SIGHUP, SIG_IGN);
   signal(SIGTERM, sigterm_handler);
   signal(SIGPIPE, SIG_IGN);
   signal(SIGINT, SIG_IGN);

   FD_ZERO(&readFdsMaster);
   FD_SET(raMonitorFd, &readFdsMaster);

   while (1)
   {
      readFds = readFdsMaster;
      n = select(raMonitorFd+1, &readFds, NULL, NULL, NULL);
      
      if (n > 0)
      {
         if (FD_ISSET(raMonitorFd, &readFds))
         {
            memset(&raStatus6MsgBody, 0, sizeof(RAStatus6MsgBody));
            ret = processRAMonitor(&raStatus6MsgBody);

            if (ret == CMSRET_SUCCESS)
            {
               /* 
                * - Send message to advertise RA info 
                * - TODO: Timeout according to router lifetime for each ifName?
                */
               sendRAInfoEventMessage(&raStatus6MsgBody);
//               tv. = raStatus6MsgBody.router_lifetime;
            }
            else
            {
               cmsLog_notice("processRAMonitor error!");
            }
         }
      }
      else if (n == 0)
      {
         /* timeout case */
      }

      if (sigterm_received)
      {
         break;
      }
   }

   cleanupRAMonitorFd();
   exit(0);
}
