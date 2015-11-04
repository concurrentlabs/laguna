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

#ifndef TCMTXMSG_H
#define TCMTXMSG_H

#ifdef __cplusplus
extern "C" {
#endif


/**********  Start Mtx PP -> Health  */
/* Ingress */
struct _tc_mtxtbl_ingress_s
{
    CHAR    strIntfName[32];
    CHAR    strRedirAddr[64];
    BOOL    bIntfRdy; /* Link */
};
typedef struct _tc_mtxtbl_ingress_s
               tc_mtxtbl_ingress_t;

struct _tc_mtxtbl_ipptohlth_s
{
    tc_mtxtbl_ingress_t tIntfITbl[TRANSC_INTERFACE_MAX];
    U16                 nTotal;
    U16                 nOpen;
};
typedef struct _tc_mtxtbl_ipptohlth_s
               tc_mtxtbl_ipptohlth_t;

/********** End Mtx PP -> Health  */

/**********  Start Mtx PktGen -> Health  */
/* Egress */
struct _tc_mtxtbl_egress_s
{
    CHAR                strIntfName[32];
    BOOL                bIntfRdy; /* Link */
};
typedef struct _tc_mtxtbl_egress_s
               tc_mtxtbl_egress_t;

struct _tc_mtxtbl_epgentohlth_s
{
    tc_mtxtbl_egress_t  tIntfETbl[TRANSC_INTERFACE_MAX];
    U16                 nTotal;
    U16                 nOpen;
};
typedef struct _tc_mtxtbl_epgentohlth_s
               tc_mtxtbl_epgentohlth_t;

struct _tc_mtxtbl_stspgentohlth_s
{
    BOOL                bRedirCapReached;
    BOOL                bActiveMode;
};
typedef struct _tc_mtxtbl_stspgentohlth_s
               tc_mtxtbl_stspgentohlth_t;

/* Map */
/*>>> active monitored redirection addresses */
struct _tc_health_monactv_s
{
    CHAR                        strRedirAddr[64];
    BOOL                        bIsRedirUp;
    BOOL                        bOldRedirIsUp;
};
typedef struct _tc_health_monactv_s
               tc_health_monactv_t;

struct _tc_mtxtbl_map_pgentohlth_s
{
    CHAR                    strMonIntfName[32];
    U16                     nEgressIdx;
    U16                     bEgressIdxSet;
    BOOL                    bLinked; /* nIngressdx and strMonIntfName values linked */
    tc_health_monactv_t*    pMonActv;
};
typedef struct _tc_mtxtbl_map_pgentohlth_s
               tc_mtxtbl_map_pgentohlth_t;

struct _tc_mtxtbl_mpgentohlth_s
{
    tc_mtxtbl_map_pgentohlth_t tIntfMapTbl[TRANSC_INTERFACE_MAX];
    U16                       nTotal;
};
typedef struct _tc_mtxtbl_mpgentohlth_s
               tc_mtxtbl_mpgentohlth_t;
/**********  End Mtx PktGen -> Health */

/********** Start Mtx Health -> PktGen */
struct _tc_mtxtbl_mhlthtopktgen_s
{
    BOOL                         tIntfMapActvTbl[TRANSC_INTERFACE_MAX];
    U16                          nTotal;
};
typedef struct _tc_mtxtbl_mhlthtopktgen_s
               tc_mtxtbl_mhlthtopktgen_t;
/********** End Mtx Health -> PktGen  */

/********** Start Mtx Health -> Mib thd */
struct _tc_mtxtbl_mhlthtomib_s
{
    tc_tr_sts_e             eTrSts;
    CHAR                         tIntfMapActvTbl[TRANSC_INTERFACE_MAX][128];
    U16                          nTotal;
};
typedef struct _tc_mtxtbl_mhlthtomib_s
               tc_mtxtbl_mhlthtomib_t;
/********** End Mtx Health -> Mib thd  */

#ifdef __cplusplus
}
#endif
#endif /* TCMTXMSG_H */
