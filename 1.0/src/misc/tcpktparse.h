/*
  Copyright 2015 Concurrent Computer Corporation

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef TCPKTPARSE_H
#define TCPKTPARSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "helper.h"

/* Layer 2 Macro */
#define TRANSC_MPLS               0x8847
#define TRANSC_IPV4               0x0800
#define TRANSC_IPV6               0x86DD

/* pkt protocol definitions */
#define TRANSC_PROTO_TCP          6
#define TRANSC_PROTO_UDP          17

/* TCP Flags */
#define TCTCPFLG_NONE             0x00
#define TCTCPFLG_FIN              0x01
#define TCTCPFLG_SYN              0x02
#define TCTCPFLG_RST              0x04
#define TCTCPFLG_PSH              0x08
#define TCTCPFLG_ACK              0x10
#define TCTCPFLG_URG              0x20


/*
 * Macros to convert pointer to a an array of bytes to a U32 or U16.
 */
#define ccur_nptrtohs(_ptr)                                                 \
    ((((U16)(_ptr)[0] << 8) & 0xFF00) | ((U16)(_ptr)[1] & 0x00FF))
#define ccur_nptrtohl(_ptr)                                                 \
    ((((U32)(_ptr)[0] << 24) & 0xFF000000) |                               \
     (((U32)(_ptr)[1] << 16) & 0x00FF0000) |                               \
     (((U32)(_ptr)[2] << 8)  & 0x0000FF00) | ((U32)(_ptr)[3] & 0x000000FF))

/*
  Note that as offsets *can* be negative,
  please do not change them to unsigned
*/
struct _tc_phshdr_sz_s
{
    S16                 nEthOffset;
    S16                 nVlanOffset;
    S16                 nL3Offset;
    S16                 nL4Offset;
    S16                 nPayloadOffset;
};
typedef struct _tc_phshdr_sz_s
               tc_phshdr_sz_t;

/* L2 */
struct _tc_phshdr_l2_s
{
    U8                  aSrcMACAddress[6];
    U8                  aDstMACAddress[6];
};
typedef struct _tc_phshdr_l2_s
               tc_phshdr_l2_t;

/* L3 */
enum _tc_iphdr_ipddrtype_e
{
    tcIpaddrTypeInvalid = 0,
    tcIpaddrTypeUnspec = 1,
    tcIpaddrTypeIPv4 = 4,
    tcIpaddrTypeIPv6 = 6

};
typedef enum _tc_iphdr_ipddrtype_e
             tc_iphdr_ipddrtype_e;

/*
 * IP address data structure.
 */
struct _tc_iphdr_ipaddr_s
{
    union _tc_iphdr_ip_u
    {
        struct _ccur_ip_v6_s { U8 octet[16]; } v6;
        struct _ccur_ip_v4_s { U8 octet[4];  } v4;
    } ip;
    tc_iphdr_ipddrtype_e eType;
};
typedef struct _tc_iphdr_ipaddr_s
               tc_iphdr_ipaddr_t;

struct _tc_iphdr_l3_s
{
    U16                     nEthType;
    tc_iphdr_ipaddr_t       tSrcIP;
    U32                     nSrcIPHash;
    tc_iphdr_ipaddr_t       tDstIP;
    U32                     nDstIPHash;
};
typedef struct _tc_iphdr_l3_s
               tc_iphdr_l3_t;

/* L4 */
enum _tc_tcphdr_l4pyldtype_e
{
    tcL4PyldTypeNone=0,
    tcL4PyldTypeHttp,
};
typedef enum _tc_tcphdr_l4pyldtype_e
             tc_tcphdr_l4pyldtype_e;

struct _tc_tcphdr_l4_s
{
    S32             nPyldLen;
    U16             nSrcPort;
    U16             nDstPort;
    U32             nTcpSeq;
    U32             nTcpAck;
    U8              nTcpFlags;
    U8*             pPyld;
    U8              pPyldBuf[1536];
};
typedef struct _tc_tcphdr_l4_s
               tc_tcphdr_l4_t;

/* Pkt descriptor */
struct _tc_pktdesc_s
{
    U8*                             pMsgStrt;       /* Pkt start */
    S32                             nCaplen;        /* Capture length */
    U32                             nWireLen;       /* Wire length */
    U64                             pktHash;        /* if any */
    tc_phshdr_sz_t                  tOffsets;
#if TRANSC_STRMINJTIME
    tc_gd_time_t                    tRxTime;
#endif
    tc_phshdr_l2_t                  l2Hdr;
    tc_iphdr_l3_t                   ipHdr;
    tc_tcphdr_l4_t                  tcpHdr;
    U16                             ethType;
};
typedef struct _tc_pktdesc_s
               tc_pktdesc_t;

CCUR_PROTECTED(tresult_t)
tcPktParse(
        tc_pktdesc_t*               pPktDesc);

CCUR_PROTECTED(tresult_t)
tcPktParseTCP(
        U8*                       pTcpHdrLen,
        tc_pktdesc_t*             pPktDesc,
        tc_phshdr_sz_t*           pPktDescSz
    );

#ifdef __cplusplus
}
#endif
#endif /* TCPKTPARSE_H */
