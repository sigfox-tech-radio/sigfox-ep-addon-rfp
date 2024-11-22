/*!*****************************************************************
 * \file    sigfox_rfp_test_mode_g.c
 * \brief   Sigfox addon RF & Protocol test mode G module
 * \details Send twice, one of the supported types of Sigfox messages
 *          ( Uplink request only ) with no delay between the messages.
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

#include "test_modes_rfp/sigfox_rfp_test_mode_types.h"
#include "sigfox_error.h"
#include "sigfox_ep_api_test.h"
#if (defined SIGFOX_EP_CERTIFICATION) && ((defined SIGFOX_EP_SPECTRUM_ACCESS_LBT))

#ifndef SIGFOX_EP_SINGLE_FRAME
#define LOOP 2
#ifndef SIGFOX_EP_T_IFU_MS
#define INTERFRAME_MS 1000
#endif
#else
#define LOOP 2
#endif

typedef struct {
    struct {
        sfx_u8 ep_api_message_cplt    : 1;
        sfx_u8 test_mode_req          : 1;
    }flags;
    SIGFOX_RFP_test_mode_t test_mode;
    sfx_u8 loop_iter;
    SIGFOX_EP_ADDON_RFP_API_progress_status_t progress_status;
} SIGFOX_RFP_TEST_MODE_G_context_t;

static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_G_init_fn(SIGFOX_RFP_test_mode_t *test_mode_callback);
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_G_process_fn(void);
static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_G_get_progress_status_fn(void);

const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_G_fn = {
    .init_fn = &SIGFOX_RFP_TEST_MODE_G_init_fn,
    .process_fn = &SIGFOX_RFP_TEST_MODE_G_process_fn,
    .get_progress_status_fn = &SIGFOX_RFP_TEST_MODE_G_get_progress_status_fn,
};

static SIGFOX_RFP_TEST_MODE_G_context_t sigfox_rfp_test_mode_g_ctx = {
    .flags.ep_api_message_cplt      = 0,
    .flags.test_mode_req            = 0,
    .test_mode.rc                   = SIGFOX_NULL,
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    .test_mode.ul_bit_rate          = 0,
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    .test_mode.tx_power_dbm_eirp    = 0,
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    .test_mode.process_cb           = SIGFOX_NULL,
    .test_mode.cplt_cb              = SIGFOX_NULL,
#endif
    .loop_iter                      = 0,
    .progress_status.status.error   = 0,
    .progress_status.progress       = 0,
};

/*!******************************************************************
 * \fn static void _SIGFOX_RFP_TEST_MODE_completion_callback(void)
 * \brief Execute the rfp test mode completion callback if the not null.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static void _SIGFOX_EP_API_message_cplt_cb(void) {
    // Local variables.
    sigfox_rfp_test_mode_g_ctx.flags.ep_api_message_cplt = 1;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    if (sigfox_rfp_test_mode_g_ctx.test_mode.process_cb != SIGFOX_NULL) {
        sigfox_rfp_test_mode_g_ctx.test_mode.process_cb();
    }
#endif
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t _send_application_message(void)
 * \brief Send application message
 * \param[in]   none
 * \param[out]  none
 * \retval      SIGFOX_EP_ADDON_RFP_API_status_t
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t _send_application_message(void) {
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
    SIGFOX_EP_API_TEST_parameters_t test_param = {0};
#ifdef SIGFOX_EP_APPLICATION_MESSAGES
    SIGFOX_EP_API_application_message_t application_message = {0};
#ifdef SIGFOX_EP_UL_PAYLOAD_SIZE
#if (SIGFOX_EP_UL_PAYLOAD_SIZE != 0)
    sfx_u8 data_cnt;
    sfx_u8 data[SIGFOX_EP_UL_PAYLOAD_SIZE] = {0x00};
#endif
#endif
#else
    SIGFOX_EP_API_control_message_t application_message = {0};
#endif
    // Configure application message structure
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    /*TODO manage 100/600*/
    application_message.common_parameters.ul_bit_rate = sigfox_rfp_test_mode_g_ctx.test_mode.ul_bit_rate;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    application_message.common_parameters.tx_power_dbm_eirp = sigfox_rfp_test_mode_g_ctx.test_mode.tx_power_dbm_eirp;
#endif
#ifndef SIGFOX_EP_SINGLE_FRAME
    application_message.common_parameters.number_of_frames = 3;
#ifndef SIGFOX_EP_T_IFU_MS
    application_message.common_parameters.t_ifu_ms = INTERFRAME_MS;
#endif
#endif
#ifdef SIGFOX_EP_PUBLIC_KEY_CAPABLE
    application_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
#endif
    test_param.tx_frequency_hz = 0;
#ifdef SIGFOX_EP_BIDIRECTIONAL
    test_param.rx_frequency_hz = 0;
    test_param.dl_t_rx_ms = 0;
    test_param.dl_t_w_ms = 0;
#endif
    test_param.flags.all = 0xFF;
#if (defined SIGFOX_EP_REGULATORY) && (defined SIGFOX_EP_SPECTRUM_ACCESS_FH)
    test_param.flags.field.tx_control_fh_enable = SIGFOX_FALSE;
#endif
#if (defined SIGFOX_EP_REGULATORY) && (defined SIGFOX_EP_SPECTRUM_ACCESS_LBT)
    test_param.lbt_cs_max_duration_first_frame_ms = 12000;
    test_param.flags.field.tx_control_lbt_enable = SIGFOX_TRUE;
#endif
#if (defined SIGFOX_EP_REGULATORY) && (defined SIGFOX_EP_SPECTRUM_ACCESS_LDC)
    test_param.flags.field.tx_control_ldc_enable = SIGFOX_FALSE;
#endif
#ifdef SIGFOX_EP_APPLICATION_MESSAGES
#ifdef SIGFOX_EP_UL_PAYLOAD_SIZE
#if (SIGFOX_EP_UL_PAYLOAD_SIZE == 0)
    application_message.type = SIGFOX_APPLICATION_MESSAGE_TYPE_EMPTY;
#else
    application_message.type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY;
    for (data_cnt = 0; data_cnt < SIGFOX_EP_UL_PAYLOAD_SIZE; data_cnt++) {
        data[data_cnt] = (sfx_u8) 0xAA;
    }
    application_message.ul_payload = data;
#endif
#else
    application_message.type = SIGFOX_APPLICATION_MESSAGE_TYPE_EMPTY;
#endif
#else
    application_message.type = SIGFOX_CONTROL_MESSAGE_TYPE_KEEP_ALIVE;
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    application_message.uplink_cplt_cb = SIGFOX_NULL;
    application_message.message_cplt_cb = &_SIGFOX_EP_API_message_cplt_cb;
#endif

    // Send Application message
#ifdef SIGFOX_EP_ERROR_CODES
#ifdef SIGFOX_EP_APPLICATION_MESSAGES
    sigfox_ep_api_status = SIGFOX_EP_API_TEST_send_application_message(&application_message, &test_param);
#else
    sigfox_ep_api_status = SIGFOX_EP_API_TEST_send_control_message(&application_message, &test_param);
#endif
    SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
#ifdef SIGFOX_EP_APPLICATION_MESSAGES
    SIGFOX_EP_API_TEST_send_application_message(&application_message, &test_param);
#else
    SIGFOX_EP_API_TEST_send_control_message(&application_message, &test_param);
#endif
#endif
#ifndef SIGFOX_EP_ASYNCHRONOUS
    _SIGFOX_EP_API_message_cplt_cb();
#endif
#ifdef SIGFOX_EP_ERROR_CODES
errors:
#endif
    SIGFOX_RETURN();
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_G_start_fn(SIGFOX_RFP_test_mode_t *rfp_test_mode)
 * \brief Start Test Mode C.
 * \param[in]   rfp_test_mode: test mode parameters
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_G_init_fn(SIGFOX_RFP_test_mode_t *rfp_test_mode) {
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
#endif
#ifdef SIGFOX_EP_PARAMETERS_CHECK
    if (rfp_test_mode == SIGFOX_NULL) {
#ifdef SIGFOX_EP_ERROR_CODES
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_RFP_API_ERROR_NULL_PARAMETER);
#else
        goto errors;
#endif
    }
#endif /* SIGFOX_EP_PARAMETERS_CHECK */
    // Reset static context
    sigfox_rfp_test_mode_g_ctx.flags.ep_api_message_cplt = 0;
    sigfox_rfp_test_mode_g_ctx.loop_iter = 0;
    sigfox_rfp_test_mode_g_ctx.progress_status.status.error = 0;
    sigfox_rfp_test_mode_g_ctx.progress_status.progress = 0;
    // Store test mode parameters locally.
    sigfox_rfp_test_mode_g_ctx.test_mode.rc = rfp_test_mode->rc;
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    sigfox_rfp_test_mode_g_ctx.test_mode.ul_bit_rate = rfp_test_mode->ul_bit_rate;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    sigfox_rfp_test_mode_g_ctx.test_mode.tx_power_dbm_eirp = rfp_test_mode->tx_power_dbm_eirp,
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_rfp_test_mode_g_ctx.test_mode.process_cb = rfp_test_mode->process_cb;
    sigfox_rfp_test_mode_g_ctx.test_mode.cplt_cb = rfp_test_mode->cplt_cb;
#endif
    sigfox_rfp_test_mode_g_ctx.flags.test_mode_req = 1;
#ifdef SIGFOX_EP_PARAMETERS_CHECK
errors:
#endif
    SIGFOX_RETURN();
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_G_process_fn(void)
 * \brief Process Test Mode C.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_G_process_fn(void) {
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sfx_u16 tmp;
#endif
    SIGFOX_EP_API_message_status_t message_status;

    if (sigfox_rfp_test_mode_g_ctx.flags.test_mode_req == 1) {
        sigfox_rfp_test_mode_g_ctx.flags.test_mode_req = 0;
#ifdef SIGFOX_EP_ERROR_CODES
        status = _send_application_message();
        SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
        _send_application_message();
#endif
    }
#ifdef SIGFOX_EP_ASYNCHRONOUS
    else {
        if (sigfox_rfp_test_mode_g_ctx.flags.ep_api_message_cplt == 1) {
            sigfox_rfp_test_mode_g_ctx.flags.ep_api_message_cplt = 0;
            message_status = SIGFOX_EP_API_get_message_status();
            if (message_status.field.execution_error == 1) {
                goto errors;
            }
            sigfox_rfp_test_mode_g_ctx.loop_iter++;
            if (sigfox_rfp_test_mode_g_ctx.loop_iter < LOOP) {
                tmp = 100 * (sigfox_rfp_test_mode_g_ctx.loop_iter);
                tmp /= LOOP;
                sigfox_rfp_test_mode_g_ctx.progress_status.progress = (sfx_u8) tmp;
#ifdef SIGFOX_EP_ERROR_CODES
                status = _send_application_message();
                SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
                _send_application_message();
#endif
            } else {
                if (sigfox_rfp_test_mode_g_ctx.test_mode.cplt_cb != SIGFOX_NULL) {
                    sigfox_rfp_test_mode_g_ctx.progress_status.progress = 100;
                    sigfox_rfp_test_mode_g_ctx.test_mode.cplt_cb();
                }
            }
        }
    }
#else
    while (sigfox_rfp_test_mode_g_ctx.loop_iter < LOOP) {
        if (sigfox_rfp_test_mode_g_ctx.flags.ep_api_message_cplt == 1) {
            sigfox_rfp_test_mode_g_ctx.flags.ep_api_message_cplt = 0;
            message_status = SIGFOX_EP_API_get_message_status();
            if (message_status.field.execution_error == 1) {
                goto errors;
            }
            sigfox_rfp_test_mode_g_ctx.loop_iter++;
            if (sigfox_rfp_test_mode_g_ctx.loop_iter < LOOP) {
#ifdef SIGFOX_EP_ERROR_CODES
                status = _send_application_message();
                SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
                _send_application_message();
#endif
            }
        }
    }
    sigfox_rfp_test_mode_g_ctx.progress_status.progress = 100;
#endif

    SIGFOX_RETURN();
errors:
    sigfox_rfp_test_mode_g_ctx.progress_status.status.error = 1;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    // test procedure done.
    sigfox_rfp_test_mode_g_ctx.test_mode.cplt_cb();
#endif
    SIGFOX_RETURN();
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_G_get_progress_status_fn(void) {
 * \brief Get the progression status
 * \param[in]   none
 * \param[out]  none
 * \retval      Progression status
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_G_get_progress_status_fn(void) {
    return sigfox_rfp_test_mode_g_ctx.progress_status;
}
#endif // SIGFOX_EP_CERTIFICATION
