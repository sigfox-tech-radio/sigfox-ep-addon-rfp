/*!*****************************************************************
 * \file    sigfox_ep_addon_rfp_api.c
 * \brief   Sigfox End-Point RF & Protocol API
 * \details This file provides functions to manage RF & Protocol test modes
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

#include "sigfox_ep_addon_rfp_api.h"
#include "sigfox_ep_addon_rfp_version.h"
#include "sigfox_ep_api.h"
#include "test_modes_rfp/sigfox_rfp_test_mode_types.h"
#ifdef SIGFOX_EP_CERTIFICATION

typedef enum {
    SIGFOX_EP_ADDON_RFP_API_STATE_CLOSE,
    SIGFOX_EP_ADDON_RFP_API_STATE_READY,
#ifdef SIGFOX_EP_ASYNCHRONOUS
    SIGFOX_EP_ADDON_RFP_API_STATE_PROCESS,
#endif
} SIGFOX_EP_ADDON_RFP_API_state_t;

#ifdef SIGFOX_EP_ASYNCHRONOUS
typedef union {
    struct {
        // IRQ flags.
        sfx_u8 ep_api_process :1;
        sfx_u8 rfp_test_mode_process :1;
        sfx_u8 rfp_test_mode_cplt :1;
    };
    sfx_u8 all;
} SIGFOX_EP_ADDON_RFP_API_flags_t;
#endif

typedef struct {
    const SIGFOX_rc_t *rc;
    const SIGFOX_RFP_test_mode_fn_t *test_mode_fn;
    SIGFOX_EP_ADDON_RFP_API_state_t state;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    volatile SIGFOX_EP_ADDON_RFP_API_flags_t flags;
    SIGFOX_EP_ADDON_RFP_API_process_cb_t process_cb;
    SIGFOX_EP_ADDON_RFP_API_test_mode_cplt_cb_t test_mode_cplt_cb;
#endif
} SIGFOX_EP_ADDON_RFP_API_context_t;

/*** SIGFOX EP API local global variables ***/

static SIGFOX_EP_ADDON_RFP_API_context_t sigfox_ep_addon_rfp_api_ctx = {
    .rc = SIGFOX_NULL,
    .test_mode_fn = SIGFOX_NULL,
    .state = SIGFOX_EP_ADDON_RFP_API_STATE_CLOSE,
#ifdef SIGFOX_EP_ASYNCHRONOUS
    .flags.ep_api_process = 0,
    .flags.rfp_test_mode_process = 0,
    .flags.rfp_test_mode_cplt = 0,
    .process_cb = SIGFOX_NULL,
    .test_mode_cplt_cb = SIGFOX_NULL,
#endif
};

/*** SIGFOX EP API local functions ***/

/*!******************************************************************
 * \fn void _CHECK_RFP_STATE(expected_state)
 * \brief Exit if the library state is not the expected one (given in parameter).
 * \param[in]   expected_state: Expected state of the library which will be compared to the current.
 * \param[out]  none
 * \retval      none
 *******************************************************************/
#define _CHECK_RFP_STATE(state_condition) { if (sigfox_ep_addon_rfp_api_ctx.state state_condition) { SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_RFP_API_ERROR_STATE); } }

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*!******************************************************************
 * \fn void _SIGFOX_EP_API_process_callback(void)
 * \brief Execute the process callback if the not null.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static void _SIGFOX_EP_API_process_callback(void) {
    sigfox_ep_addon_rfp_api_ctx.flags.ep_api_process = 1;
    if (sigfox_ep_addon_rfp_api_ctx.process_cb != SIGFOX_NULL) {
        sigfox_ep_addon_rfp_api_ctx.process_cb();
    }
}

/*!******************************************************************
 * \fn static void _SIGFOX_RFP_TEST_MODE_process_callback(void)
 * \brief Execute the rfp test mode process callback if the not null.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static void _SIGFOX_RFP_TEST_MODE_process_callback(void) {
    sigfox_ep_addon_rfp_api_ctx.flags.rfp_test_mode_process = 1;
    if (sigfox_ep_addon_rfp_api_ctx.process_cb != SIGFOX_NULL) {
        sigfox_ep_addon_rfp_api_ctx.process_cb();
    }
}

/*!******************************************************************
 * \fn static void _SIGFOX_RFP_TEST_MODE_completion_callback(void)
 * \brief Execute the rfp test mode completion callback if the not null.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static void _SIGFOX_RFP_TEST_MODE_completion_callback(void) {
    sigfox_ep_addon_rfp_api_ctx.flags.rfp_test_mode_cplt = 1;
    if (sigfox_ep_addon_rfp_api_ctx.process_cb != SIGFOX_NULL) {
        sigfox_ep_addon_rfp_api_ctx.process_cb();
    }
}
#endif

/*** SIGFOX EP API functions ***/

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_open(SIGFOX_EP_ADDON_RFP_API_config_t *config)
 * \brief Open the RFP addon.
 * \param[in]   config: Pointer to the rfp addon configuration.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_open(SIGFOX_EP_ADDON_RFP_API_config_t *config) {
    // Local variables.
    SIGFOX_EP_API_config_t ep_api_config;
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
    // Check RFP ADDON state.
    _CHECK_RFP_STATE(!= SIGFOX_EP_ADDON_RFP_API_STATE_CLOSE);
    // Open EP library.
    ep_api_config.rc = config->rc;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    ep_api_config.process_cb = &_SIGFOX_EP_API_process_callback;
#endif
#ifndef SIGFOX_EP_MESSAGE_COUNTER_ROLLOVER
    ep_api_config.message_counter_rollover = config->message_counter_rollover;
#endif
    // Open library.
#ifdef SIGFOX_EP_ERROR_CODES
    sigfox_ep_api_status = SIGFOX_EP_API_open(&ep_api_config);
    SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
    SIGFOX_EP_API_open(&ep_api_config);
#endif
    // Store and configure parameters into static context if no error occurred.
    sigfox_ep_addon_rfp_api_ctx.rc = config->rc;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_ep_addon_rfp_api_ctx.process_cb = config->process_cb;
#endif
    // Update ADDON RFP state if no error occurred.
    sigfox_ep_addon_rfp_api_ctx.state = SIGFOX_EP_ADDON_RFP_API_STATE_READY;
errors:
#ifdef SIGFOX_EP_ERROR_CODES
    return status;
#else
    return;
#endif
}

SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_close(void) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
    // Close EP library.
#ifdef SIGFOX_EP_ERROR_CODES
    sigfox_ep_api_status = SIGFOX_EP_API_close();
    SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
    SIGFOX_EP_API_close();
#endif
    // Reset parameters into static context
    sigfox_ep_addon_rfp_api_ctx.rc = SIGFOX_NULL;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_ep_addon_rfp_api_ctx.process_cb = SIGFOX_NULL;
#endif
    // Update ADDON RFP state if no error occurred.
    sigfox_ep_addon_rfp_api_ctx.state = SIGFOX_EP_ADDON_RFP_API_STATE_CLOSE;
#ifdef SIGFOX_EP_ERROR_CODES
errors:
#endif
    SIGFOX_RETURN();
}

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_process(void)
 * \brief Main process function of the RFP addon.
 * \brief In asynchronous mode, this function is called by the client when the process callback is triggered.
 * \brief In blocking mode, this function is called by the library itself until the requested operation is completed.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_process(void) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
    if (sigfox_ep_addon_rfp_api_ctx.flags.ep_api_process == 1) {
#ifdef SIGFOX_EP_ERROR_CODES
        sigfox_ep_api_status = SIGFOX_EP_API_process();
        sigfox_ep_addon_rfp_api_ctx.flags.ep_api_process = 0;
        SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
        SIGFOX_EP_API_process();
#endif
    }

    switch (sigfox_ep_addon_rfp_api_ctx.state) {
    case SIGFOX_EP_ADDON_RFP_API_STATE_CLOSE:
        break;
    case SIGFOX_EP_ADDON_RFP_API_STATE_READY:
        break;
    case SIGFOX_EP_ADDON_RFP_API_STATE_PROCESS:
        if (sigfox_ep_addon_rfp_api_ctx.flags.rfp_test_mode_process == 1) {
            if (sigfox_ep_addon_rfp_api_ctx.test_mode_fn != SIGFOX_NULL) {
#ifdef SIGFOX_EP_ERROR_CODES
                status = sigfox_ep_addon_rfp_api_ctx.test_mode_fn->process_fn();
                SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
                sigfox_ep_addon_rfp_api_ctx.test_mode_fn->process_fn();
#endif
            }
            sigfox_ep_addon_rfp_api_ctx.flags.rfp_test_mode_process = 0;
        }
        if (sigfox_ep_addon_rfp_api_ctx.flags.rfp_test_mode_cplt == 1) {

            if (sigfox_ep_addon_rfp_api_ctx.test_mode_cplt_cb != SIGFOX_NULL) {
                sigfox_ep_addon_rfp_api_ctx.test_mode_cplt_cb();
            }
            sigfox_ep_addon_rfp_api_ctx.flags.rfp_test_mode_cplt = 0;
            sigfox_ep_addon_rfp_api_ctx.state = SIGFOX_EP_ADDON_RFP_API_STATE_READY;
        }
        break;
    default:
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_RFP_API_ERROR_STATE);
    }
    SIGFOX_RETURN();
errors:
    if (sigfox_ep_addon_rfp_api_ctx.test_mode_cplt_cb != SIGFOX_NULL) {
        sigfox_ep_addon_rfp_api_ctx.test_mode_cplt_cb();
    }
    sigfox_ep_addon_rfp_api_ctx.state = SIGFOX_EP_ADDON_RFP_API_STATE_READY;
    SIGFOX_RETURN();
}
#endif

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_test_mode(SIGFOX_EP_ADDON_RFP_API_test_mode_t test_mode)
 * \brief Execute a specific test mode for RF & Protocol test.
 * \param[in]   test_mode: Pointer to the test_mode data.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_test_mode(SIGFOX_EP_ADDON_RFP_API_test_mode_t *test_mode) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
#endif
    SIGFOX_RFP_test_mode_t rfp_test_mode;
    // Check RFP ADDON is opened.
    _CHECK_RFP_STATE(!= SIGFOX_EP_ADDON_RFP_API_STATE_READY);
    switch (test_mode->test_mode_reference) {
    case SIGFOX_EP_ADDON_RFP_API_TEST_MODE_A:
        sigfox_ep_addon_rfp_api_ctx.test_mode_fn = &SIGFOX_RFP_TEST_MODE_A_fn;
        break;
    case SIGFOX_EP_ADDON_RFP_API_TEST_MODE_B:
        sigfox_ep_addon_rfp_api_ctx.test_mode_fn = &SIGFOX_RFP_TEST_MODE_B_fn;
        break;
    case SIGFOX_EP_ADDON_RFP_API_TEST_MODE_C:
        sigfox_ep_addon_rfp_api_ctx.test_mode_fn = &SIGFOX_RFP_TEST_MODE_C_fn;
        break;
#if defined SIGFOX_EP_BIDIRECTIONAL
    case SIGFOX_EP_ADDON_RFP_API_TEST_MODE_D:
        sigfox_ep_addon_rfp_api_ctx.test_mode_fn = &SIGFOX_RFP_TEST_MODE_D_fn;
        break;
    case SIGFOX_EP_ADDON_RFP_API_TEST_MODE_E:
        sigfox_ep_addon_rfp_api_ctx.test_mode_fn = &SIGFOX_RFP_TEST_MODE_E_fn;
        break;
    case SIGFOX_EP_ADDON_RFP_API_TEST_MODE_F:
        sigfox_ep_addon_rfp_api_ctx.test_mode_fn = &SIGFOX_RFP_TEST_MODE_F_fn;
        break;
#endif
#ifdef SIGFOX_EP_SPECTRUM_ACCESS_LBT
    case SIGFOX_EP_ADDON_RFP_API_TEST_MODE_G:
        sigfox_ep_addon_rfp_api_ctx.test_mode_fn = &SIGFOX_RFP_TEST_MODE_G_fn;
        break;
#endif
    case SIGFOX_EP_ADDON_RFP_API_TEST_MODE_J:
        sigfox_ep_addon_rfp_api_ctx.test_mode_fn = &SIGFOX_RFP_TEST_MODE_J_fn;
        break;
#ifdef SIGFOX_EP_PUBLIC_KEY_CAPABLE
    case SIGFOX_EP_ADDON_RFP_API_TEST_MODE_K:
        sigfox_ep_addon_rfp_api_ctx.test_mode_fn = &SIGFOX_RFP_TEST_MODE_K_fn;
        break;
#endif
    case SIGFOX_EP_ADDON_RFP_API_TEST_MODE_L:
        sigfox_ep_addon_rfp_api_ctx.test_mode_fn = &SIGFOX_RFP_TEST_MODE_L_fn;
        break;
    default:
#ifdef SIGFOX_EP_ERROR_CODES
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_RFP_API_ERROR_TEST_MODE)
#else
        goto errors;
#endif
    }
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_ep_addon_rfp_api_ctx.flags.rfp_test_mode_process = 0;
    sigfox_ep_addon_rfp_api_ctx.flags.rfp_test_mode_cplt = 0;
    sigfox_ep_addon_rfp_api_ctx.test_mode_cplt_cb = test_mode->test_mode_cplt_cb;
    rfp_test_mode.process_cb = _SIGFOX_RFP_TEST_MODE_process_callback;
    rfp_test_mode.cplt_cb = _SIGFOX_RFP_TEST_MODE_completion_callback;
#endif
    rfp_test_mode.rc = sigfox_ep_addon_rfp_api_ctx.rc;
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    rfp_test_mode.ul_bit_rate = test_mode->ul_bit_rate;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    rfp_test_mode.tx_power_dbm_eirp = test_mode->tx_power_dbm_eirp;
#endif
#ifdef SIGFOX_EP_BIDIRECTIONAL
    rfp_test_mode.downlink_cplt_cb = test_mode->downlink_cplt_cb;
#endif
    if (sigfox_ep_addon_rfp_api_ctx.test_mode_fn == SIGFOX_NULL)
#ifdef SIGFOX_EP_ERROR_CODES
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_RFP_API_ERROR_TEST_MODE)
#else
        goto errors;
#endif
#ifdef SIGFOX_EP_ERROR_CODES
    status = sigfox_ep_addon_rfp_api_ctx.test_mode_fn->init_fn(&rfp_test_mode);
    SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
    sigfox_ep_addon_rfp_api_ctx.test_mode_fn->init_fn(&rfp_test_mode);
#endif

#ifdef SIGFOX_EP_ERROR_CODES
    status = sigfox_ep_addon_rfp_api_ctx.test_mode_fn->process_fn();
    SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
    sigfox_ep_addon_rfp_api_ctx.test_mode_fn->process_fn();
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_ep_addon_rfp_api_ctx.state = SIGFOX_EP_ADDON_RFP_API_STATE_PROCESS;
    if (sigfox_ep_addon_rfp_api_ctx.process_cb == SIGFOX_NULL) {
        // Block until library goes back to READY state.
        while (sigfox_ep_addon_rfp_api_ctx.state != SIGFOX_EP_ADDON_RFP_API_STATE_READY) {
#ifdef SIGFOX_EP_ERROR_CODES
            status = SIGFOX_EP_ADDON_RFP_API_process();
            SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else // SIGFOX_EP_ERROR_CODES
            SIGFOX_EP_API_process();
#endif
        }
    }
#else // SYNCHRONOUS

#endif
    SIGFOX_RETURN();
errors:
    SIGFOX_RETURN();
}

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_EP_ADDON_RFP_API_get_test_mode_progress_status(void)
 * \brief Get the current message status.
 * \param[in]   none
 * \param[out]  none
 * \retval      Current progression status.
 *******************************************************************/
SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_EP_ADDON_RFP_API_get_test_mode_progress_status(void) {
    // Local variables.
    SIGFOX_EP_ADDON_RFP_API_progress_status_t progress_status;
    if (sigfox_ep_addon_rfp_api_ctx.test_mode_fn->get_progress_status_fn != SIGFOX_NULL) {
        progress_status = sigfox_ep_addon_rfp_api_ctx.test_mode_fn->get_progress_status_fn();
    }
    return progress_status;
}

#ifdef SIGFOX_EP_VERBOSE
/*!******************************************************************
 * \fn SIGFOX_EP_API_status_t SIGFOX_EP_ADDON_RFP_API_get_version(sfx_u8 **version, sfx_u8 *version_size_char)
 * \brief Get EP library version.
 * \param[in]   version_type: Version to get.
 * \param[out]  version: Version string.
 * \param[out]  version_size_char: Pointer to the string size.
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_get_version(sfx_u8 **version, sfx_u8 *version_size_char) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
#endif
#ifdef SIGFOX_EP_PARAMETERS_CHECK
    if ((version == SIGFOX_NULL) || (version_size_char == SIGFOX_NULL)) {
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_RFP_API_ERROR_NULL_PARAMETER);
    }
#endif
    // Check library is opened.
    _CHECK_RFP_STATE(== SIGFOX_EP_ADDON_RFP_API_STATE_CLOSE);
    (*version) = (sfx_u8 *) SIGFOX_EP_ADDON_RFP_VERSION;
    (*version_size_char) = (sfx_u8) sizeof(SIGFOX_EP_ADDON_RFP_VERSION);
errors:
    SIGFOX_RETURN();
}
#endif
#endif // SIGFOX_EP_CERTIFICATION
