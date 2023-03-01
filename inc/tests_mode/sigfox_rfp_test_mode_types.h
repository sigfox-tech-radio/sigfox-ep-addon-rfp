/*!*****************************************************************
 * \file    sigfox_rfp_test_mode_types.h
 * \brief   RFP test mode types
 *******************************************************************
 * \copyright
 *
 * Copyright (c) 2022, UnaBiz SAS
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1 Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  2 Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  3 Neither the name of UnaBiz SAS nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************/

#ifndef __SIGFOX_RFP_TEST_MODE_TYPES_H__
#define __SIGFOX_RFP_TEST_MODE_TYPES_H__
#ifdef USE_SIGFOX_EP_FLAGS_H
#include "sigfox_ep_flags.h"
#endif
#include "sigfox_types.h"
#include "sigfox_ep_addon_rfp_api.h"
#ifdef CERTIFICATION

typedef struct {
    const SIGFOX_rc_t *rc;
#ifndef UL_BIT_RATE_BPS
    SIGFOX_ul_bit_rate_t ul_bit_rate;
#endif
#ifndef TX_POWER_DBM_EIRP
    sfx_s8 tx_power_dbm_eirp;
#endif
#ifdef ASYNCHRONOUS
    void (*process_cb)(void);
    void (*cplt_cb)(void);
#endif
} SIGFOX_RFP_test_mode_t;

typedef struct {
    SIGFOX_EP_ADDON_RFP_API_status_t (*init_fn)(SIGFOX_RFP_test_mode_t *test_mode_callbacks);
    SIGFOX_EP_ADDON_RFP_API_status_t (*process_fn)(void);
    SIGFOX_EP_ADDON_RFP_API_progress_status_t (*get_progress_status_fn)(void);
} SIGFOX_RFP_test_mode_fn_t;

extern const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_A_fn;
extern const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_B_fn;
extern const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_C_fn;
#ifdef BIDIRECTIONAL
extern const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_E_fn;
extern const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_F_fn;
extern const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_D_fn;
#endif
#if (defined RC3C) || (defined RC5)
extern const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_G_fn;
#endif
extern const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_J_fn;
#ifdef PUBLIC_KEY_CAPABLE
extern const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_K_fn;
#endif
extern const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_L_fn;

#endif
#endif /* __SIGFOX_RFP_TEST_MODE_TYPES_H__ */
