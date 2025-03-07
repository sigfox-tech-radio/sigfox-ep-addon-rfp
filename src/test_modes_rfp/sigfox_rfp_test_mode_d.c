/**
 * \file sigfox_rfp_test_mode_d.c
 * \brief Sigfox addon RF & Protocol test mode D module
 * \details Wait for a downlink message at the Downlink Central Frequency with
 *  AUTHENTICATION OFF with RXGFSK static buffer pattern value and 30s
 *  listening window duration.
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
#include "manuf/mcu_api.h"

#if (defined SIGFOX_EP_CERTIFICATION) && (defined SIGFOX_EP_BIDIRECTIONAL)

#define TIMEOUT_MS 30000

typedef struct {
    struct {
        sfx_u8 ep_api_message_cplt    : 1;
        sfx_u8 test_mode_req          : 1;
    }flags;
    SIGFOX_RFP_test_mode_t test_mode;
    SIGFOX_EP_ADDON_RFP_API_progress_status_t progress_status;
} SIGFOX_RFP_TEST_MODE_D_context_t;

static const sfx_u8 dl_pattern[SIGFOX_DL_PAYLOAD_SIZE_BYTES] = {0x32, 0x68, 0xc5, 0xba, 0x53, 0xae, 0x79, 0xe7};

static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_D_init_fn(SIGFOX_RFP_test_mode_t *test_mode_callback);
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_D_process_fn(void);
static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_D_get_progress_status_fn(void);

const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_D_fn = {
    .init_fn = &SIGFOX_RFP_TEST_MODE_D_init_fn,
    .process_fn = &SIGFOX_RFP_TEST_MODE_D_process_fn,
    .get_progress_status_fn = &SIGFOX_RFP_TEST_MODE_D_get_progress_status_fn,
};

static SIGFOX_RFP_TEST_MODE_D_context_t sigfox_rfp_test_mode_d_ctx = {
    .flags.ep_api_message_cplt      = 0,
    .flags.test_mode_req            = 0,
    .test_mode.rc                   = SIGFOX_NULL,
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    .test_mode.ul_bit_rate          = 0,
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    .test_mode.tx_power_dbm_eirp    = 0,
#endif
#ifdef SIGFOX_EP_BIDIRECTIONAL
    .test_mode.downlink_cplt_cb     = SIGFOX_NULL,
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    .test_mode.process_cb           = SIGFOX_NULL,
    .test_mode.cplt_cb              = SIGFOX_NULL,
#endif
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

    sigfox_rfp_test_mode_d_ctx.flags.ep_api_message_cplt = 1;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    if (sigfox_rfp_test_mode_d_ctx.test_mode.process_cb != SIGFOX_NULL) {
        sigfox_rfp_test_mode_d_ctx.test_mode.process_cb();
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
    SIGFOX_EP_API_application_message_t application_message = {0};
#ifdef SIGFOX_EP_UL_PAYLOAD_SIZE
#if (SIGFOX_EP_UL_PAYLOAD_SIZE != 0)
    sfx_u8 data_cnt;
    sfx_u8 data[SIGFOX_EP_UL_PAYLOAD_SIZE] = {0x00};
#endif
#else
    sfx_u8 data_cnt;
    sfx_u8 data[SIGFOX_UL_PAYLOAD_MAX_SIZE_BYTES] = {0x00};
#endif

    // Configure application message structure
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    application_message.common_parameters.ul_bit_rate = sigfox_rfp_test_mode_d_ctx.test_mode.ul_bit_rate;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    application_message.common_parameters.tx_power_dbm_eirp = sigfox_rfp_test_mode_d_ctx.test_mode.tx_power_dbm_eirp;
#endif
#ifndef SIGFOX_EP_SINGLE_FRAME
    application_message.common_parameters.number_of_frames = 3;
#ifndef SIGFOX_EP_T_IFU_MS
    application_message.common_parameters.t_ifu_ms = 500;
#endif
#endif
#ifdef SIGFOX_EP_PUBLIC_KEY_CAPABLE
    application_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
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
    application_message.ul_payload_size_bytes = SIGFOX_UL_PAYLOAD_MAX_SIZE_BYTES;
    application_message.type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY;
    for (data_cnt = 0; data_cnt < SIGFOX_UL_PAYLOAD_MAX_SIZE_BYTES; data_cnt++) {
        data[data_cnt] = (sfx_u8) 0xAA;
    }
    application_message.ul_payload = data;
#endif
#ifndef SIGFOX_EP_T_CONF_MS
    application_message.t_conf_ms = 1400;
#endif
    application_message.bidirectional_flag = 1;
#else
    application_message.type = SIGFOX_CONTROL_MESSAGE_TYPE_KEEP_ALIVE;
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    application_message.uplink_cplt_cb = SIGFOX_NULL;
    application_message.downlink_cplt_cb = SIGFOX_NULL;
    application_message.message_cplt_cb = &_SIGFOX_EP_API_message_cplt_cb;
#endif
    test_param.tx_frequency_hz = 0;
    test_param.rx_frequency_hz = sigfox_rfp_test_mode_d_ctx.test_mode.rc->f_dl_hz;
    test_param.dl_t_rx_ms = TIMEOUT_MS;
    test_param.dl_t_w_ms = 0;
    test_param.flags.field.ul_enable = 0;
    test_param.flags.field.dl_enable = 1;
    test_param.flags.field.dl_decoding_enable = 0;
    test_param.flags.field.dl_conf_enable = 0;
#if (defined SIGFOX_EP_REGULATORY) && (defined SIGFOX_EP_SPECTRUM_ACCESS_FH)
    test_param.flags.field.tx_control_fh_enable = SIGFOX_FALSE;
#endif
#if (defined SIGFOX_EP_REGULATORY) && (defined SIGFOX_EP_SPECTRUM_ACCESS_LBT)
    test_param.flags.field.tx_control_lbt_enable = SIGFOX_FALSE;
#endif
#if (defined SIGFOX_EP_REGULATORY) && (defined SIGFOX_EP_SPECTRUM_ACCESS_LDC)
    test_param.flags.field.tx_control_ldc_enable = SIGFOX_FALSE;
#endif
    // Send Application message
#ifdef SIGFOX_EP_ERROR_CODES
    sigfox_ep_api_status = SIGFOX_EP_API_TEST_send_application_message(&application_message, &test_param);
    SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
    SIGFOX_EP_API_TEST_send_application_message(&application_message, &test_param);
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
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_D_start_fn(SIGFOX_RFP_test_mode_t *rfp_test_mode)
 * \brief Start Test Mode C.
 * \param[in]   rfp_test_mode: test mode parameters
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_D_init_fn(SIGFOX_RFP_test_mode_t *rfp_test_mode) {
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
    sigfox_rfp_test_mode_d_ctx.flags.ep_api_message_cplt = 0;
    sigfox_rfp_test_mode_d_ctx.progress_status.status.error = 0;
    sigfox_rfp_test_mode_d_ctx.progress_status.progress = 0;
    // Store test mode parameters locally.
    sigfox_rfp_test_mode_d_ctx.test_mode.rc = rfp_test_mode->rc;
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    sigfox_rfp_test_mode_d_ctx.test_mode.ul_bit_rate = rfp_test_mode->ul_bit_rate;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    sigfox_rfp_test_mode_d_ctx.test_mode.tx_power_dbm_eirp = rfp_test_mode->tx_power_dbm_eirp;
#endif
#ifdef SIGFOX_EP_BIDIRECTIONAL
    sigfox_rfp_test_mode_d_ctx.test_mode.downlink_cplt_cb = rfp_test_mode->downlink_cplt_cb;
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_rfp_test_mode_d_ctx.test_mode.process_cb = rfp_test_mode->process_cb;
    sigfox_rfp_test_mode_d_ctx.test_mode.cplt_cb = rfp_test_mode->cplt_cb;
#endif
    sigfox_rfp_test_mode_d_ctx.flags.test_mode_req = 1;
#ifdef SIGFOX_EP_PARAMETERS_CHECK
errors:
#endif
    SIGFOX_RETURN();
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_D_process_fn(void)
 * \brief Process Test Mode C.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_D_process_fn(void) {
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
    SIGFOX_EP_API_message_status_t message_status;
    sfx_bool dl_check;
    sfx_u8 payload_cnt, dl_payload[SIGFOX_DL_PAYLOAD_SIZE_BYTES];
    sfx_s16 dl_rssi_dbm;

    if (sigfox_rfp_test_mode_d_ctx.flags.test_mode_req == 1) {
        sigfox_rfp_test_mode_d_ctx.flags.test_mode_req = 0;
#ifdef SIGFOX_EP_ERROR_CODES
        status = _send_application_message();
        SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
        _send_application_message();
#endif
    }
#ifndef SIGFOX_EP_ASYNCHRONOUS
    while (1) {
#endif
        if (sigfox_rfp_test_mode_d_ctx.flags.ep_api_message_cplt == 1) {
            sigfox_rfp_test_mode_d_ctx.flags.ep_api_message_cplt = 0;
            message_status = SIGFOX_EP_API_get_message_status();
            if (message_status.field.execution_error == 1) {
                goto errors;
            }
            if (message_status.field.network_error == 1) {
#ifndef SIGFOX_EP_ASYNCHRONOUS
                sigfox_rfp_test_mode_d_ctx.progress_status.progress = 100;
                break;
#else
            if (sigfox_rfp_test_mode_d_ctx.test_mode.cplt_cb != SIGFOX_NULL) {
                sigfox_rfp_test_mode_d_ctx.progress_status.progress = 100;
                sigfox_rfp_test_mode_d_ctx.test_mode.cplt_cb();
            }
#endif
            }
            if (message_status.field.dl_frame == 1) {
#ifdef SIGFOX_EP_ERROR_CODES
                sigfox_ep_api_status = SIGFOX_EP_API_get_dl_payload(dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES, &dl_rssi_dbm);
                SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
            SIGFOX_EP_API_get_dl_payload(dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES, &dl_rssi_dbm);
#endif
                dl_check = SIGFOX_TRUE;
                for (payload_cnt = 0; payload_cnt < SIGFOX_DL_PAYLOAD_SIZE_BYTES; payload_cnt++) {
                    if (dl_payload[payload_cnt] != dl_pattern[payload_cnt]) {
                        dl_check = SIGFOX_FALSE;
                    }
                }

                if ((dl_check == SIGFOX_TRUE) && (sigfox_rfp_test_mode_d_ctx.test_mode.downlink_cplt_cb != SIGFOX_NULL)) {
                    sigfox_rfp_test_mode_d_ctx.test_mode.downlink_cplt_cb(dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES, dl_rssi_dbm);
                }
#ifdef SIGFOX_EP_ERROR_CODES
                status = _send_application_message();
                SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
            _send_application_message();
#endif
            }
        }
#ifndef SIGFOX_EP_ASYNCHRONOUS
    }
#endif
    SIGFOX_RETURN();
errors:
    sigfox_rfp_test_mode_d_ctx.progress_status.status.error = 1;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    // test procedure done.
    sigfox_rfp_test_mode_d_ctx.test_mode.cplt_cb();
#endif
    SIGFOX_RETURN();
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_D_get_progress_status_fn(void) {
 * \brief Get the progression status
 * \param[in]   none
 * \param[out]  none
 * \retval      Progression status
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_D_get_progress_status_fn(void) {
    return sigfox_rfp_test_mode_d_ctx.progress_status;
}
#endif // SIGFOX_EP_CERTIFICATION
