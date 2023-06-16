/**
 * \file sigfox_rfp_test_mode_f.c
 * \brief Sigfox addon RF & Protocol test mode F module
 * \details
 *
 * \arg Send one of the supported types of Sigfox messages* with downlink request
 *
 * \arg Check the response payload ( has to be all bytes set with 0x30 + byte index )
 *
 * \arg Send one Sigfox frame (repetition 1)** without downlink request :
 *
 * 1: If at least 1byte supported : with payload (byte[0] = 1 if the downlink
 *  response was correct, byte[0] = 0 if the downlink response was not correct, )
 * 2: Otherwise bit 1 if the downlink response was correct, 0 if the downlink
 *  response was not correct.
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

#include "tests_mode/sigfox_rfp_test_mode_types.h"
#include "sigfox_error.h"
#include "sigfox_ep_api_test.h"
#if (defined CERTIFICATION) && (defined BIDIRECTIONAL)


typedef enum {
    MODE_F_STATE_WAIT,
    MODE_F_STATE_DLK_MESSAGE,
    MODE_F_STATE_CHECKUP_MESSAGE,
    MODE_F_STATE_END,
}test_state_t;

typedef struct {
    struct {
        sfx_u8 ep_api_message_cplt    : 1;
    }flags;
    test_state_t test_state;
    SIGFOX_RFP_test_mode_t test_mode;
    SIGFOX_EP_ADDON_RFP_API_progress_status_t progress_status;
}SIGFOX_RFP_TEST_MODE_C_context_t;

static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_F_init_fn(SIGFOX_RFP_test_mode_t *test_mode_callback);
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_F_process_fn(void);
static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_F_get_progress_status_fn(void);

const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_F_fn = {
        .init_fn = &SIGFOX_RFP_TEST_MODE_F_init_fn,
        .process_fn = &SIGFOX_RFP_TEST_MODE_F_process_fn,
        .get_progress_status_fn = &SIGFOX_RFP_TEST_MODE_F_get_progress_status_fn,
};

static SIGFOX_RFP_TEST_MODE_C_context_t sigfox_rfp_test_mode_f_ctx = {
        .flags.ep_api_message_cplt      = 0,
        .test_state                     = MODE_F_STATE_WAIT,
        .test_mode.rc                   = SFX_NULL,
#ifndef UL_BIT_RATE_BPS
        .test_mode.ul_bit_rate          = 0,
#endif
#ifndef TX_POWER_DBM_EIRP
        .test_mode.tx_power_dbm_eirp    = 0,
#endif
#ifdef ASYNCHRONOUS
        .test_mode.process_cb           = SFX_NULL,
        .test_mode.cplt_cb              = SFX_NULL,
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

    sigfox_rfp_test_mode_f_ctx.flags.ep_api_message_cplt = 1;
#ifdef ASYNCHRONOUS
    if (sigfox_rfp_test_mode_f_ctx.test_mode.process_cb != SFX_NULL)
        sigfox_rfp_test_mode_f_ctx.test_mode.process_cb();
#endif
}


static void _populate_common_param(SIGFOX_EP_API_common_t *common_param) {
    //Configure common parameters structure
#ifndef UL_BIT_RATE_BPS
    common_param->ul_bit_rate = sigfox_rfp_test_mode_f_ctx.test_mode.ul_bit_rate;
#endif
#ifndef TX_POWER_DBM_EIRP
    common_param->tx_power_dbm_eirp = sigfox_rfp_test_mode_f_ctx.test_mode.tx_power_dbm_eirp;
#endif
#ifndef SINGLE_FRAME
    common_param->number_of_frames = 3;
#ifndef T_IFU_MS
    common_param->t_ifu_ms = 500;
#endif
#endif
#ifdef PUBLIC_KEY_CAPABLE
    common_param->ep_key_type = SIGFOX_EP_KEY_PRIVATE;
#endif
}

static void _populate_test_param(SIGFOX_EP_API_TEST_parameters_t * test_param) {
    test_param->tx_frequency_hz = 0;
    test_param->rx_frequency_hz = 0;
    test_param->dl_t_rx_ms = 0;
    test_param->dl_t_w_ms = 0;
    test_param->flags.all = 0xFF;
#if (defined REGULATORY) && (defined SPECTRUM_ACCESS_FH)
    test_param->flags.field.tx_control_fh_enable = SFX_FALSE;
#endif
#if (defined REGULATORY) && (defined SPECTRUM_ACCESS_LBT)
    test_param->lbt_cs_max_duration_first_frame_ms = 0;
    test_param->flags.field.tx_control_lbt_enable = SFX_FALSE;
#endif
#if (defined REGULATORY) && (defined SPECTRUM_ACCESS_LDC)
    test_param->flags.field.tx_control_ldc_enable = SFX_FALSE;
#endif
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_F_start_fn(SIGFOX_RFP_test_mode_t *rfp_test_mode)
 * \brief Start Test Mode C.
 * \param[in]   rfp_test_mode: test mode parameters
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_F_init_fn(SIGFOX_RFP_test_mode_t *rfp_test_mode) {
#ifdef ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
#endif
#ifdef PARAMETERS_CHECK
    if (rfp_test_mode == SFX_NULL) {
#ifdef ERROR_CODES
        EXIT_ERROR(SIGFOX_EP_ADDON_RFP_API_ERROR_NULL_PARAMETER);
#else
        goto errors;
#endif
    }
#endif /* PARAMETERS_CHECK */
    //Reset static context
    sigfox_rfp_test_mode_f_ctx.flags.ep_api_message_cplt = 0;
    sigfox_rfp_test_mode_f_ctx.progress_status.status.error = 0;
    sigfox_rfp_test_mode_f_ctx.progress_status.progress = 0;
    // Store test mode parameters locally.
    sigfox_rfp_test_mode_f_ctx.test_mode.rc = rfp_test_mode->rc;
#ifndef UL_BIT_RATE_BPS
    sigfox_rfp_test_mode_f_ctx.test_mode.ul_bit_rate = rfp_test_mode->ul_bit_rate;
#endif
#ifndef TX_POWER_DBM_EIRP
    sigfox_rfp_test_mode_f_ctx.test_mode.tx_power_dbm_eirp = rfp_test_mode->tx_power_dbm_eirp,
#endif
#ifdef ASYNCHRONOUS
    sigfox_rfp_test_mode_f_ctx.test_mode.process_cb = rfp_test_mode->process_cb;
    sigfox_rfp_test_mode_f_ctx.test_mode.cplt_cb = rfp_test_mode->cplt_cb;
#endif
    sigfox_rfp_test_mode_f_ctx.test_state = MODE_F_STATE_DLK_MESSAGE;
#ifdef PARAMETERS_CHECK
errors:
#endif
    RETURN();
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_F_process_fn(void)
 * \brief Process Test Mode C.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_F_process_fn(void) {
#ifdef ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
    SIGFOX_EP_API_TEST_parameters_t test_param;
    SIGFOX_EP_API_application_message_t message_param;
    SIGFOX_EP_API_message_status_t message_status;
    sfx_u8 data_cnt, dl_payload[SIGFOX_DL_PAYLOAD_SIZE_BYTES];
    sfx_s16 dl_rssi_dbm;
#if (defined UL_PAYLOAD_SIZE) && (UL_PAYLOAD_SIZE != 0)
    sfx_u8 data[UL_PAYLOAD_SIZE];
#else
    sfx_u8 data[1];
#endif

#ifndef ASYNCHRONOUS
    while(sigfox_rfp_test_mode_f_ctx.test_state != MODE_F_STATE_WAIT) {
#endif
    switch(sigfox_rfp_test_mode_f_ctx.test_state) {
        case MODE_F_STATE_WAIT:
            break;
        case MODE_F_STATE_DLK_MESSAGE:
            _populate_test_param(&test_param);
            _populate_common_param(&message_param.common_parameters);
            message_param.bidirectional_flag = SFX_TRUE;
#if (defined UL_PAYLOAD_SIZE) && (UL_PAYLOAD_SIZE != 0)
            message_param.type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY;
            for (data_cnt = 0; data_cnt < UL_PAYLOAD_SIZE; data_cnt++) {
                data[data_cnt] = (sfx_u8)0xAA;
            }
            message_param.ul_payload = data;
#else
            message_param.type = SIGFOX_APPLICATION_MESSAGE_TYPE_EMPTY;
#endif
#ifdef ASYNCHRONOUS
            message_param.uplink_cplt_cb = SFX_NULL;
            message_param.downlink_cplt_cb = SFX_NULL;
            message_param.message_cplt_cb = &_SIGFOX_EP_API_message_cplt_cb;
#endif
#ifndef T_CONF_MS
            message_param.t_conf_ms = 1400;
#endif
#ifdef ERROR_CODES
            sigfox_ep_api_status = SIGFOX_EP_API_TEST_send_application_message(&message_param, &test_param);
            SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
            SIGFOX_EP_API_TEST_send_application_message(&message_param, &test_param);
#endif
            sigfox_rfp_test_mode_f_ctx.test_state = MODE_F_STATE_CHECKUP_MESSAGE;
#ifndef ASYNCHRONOUS
            _SIGFOX_EP_API_message_cplt_cb();
#endif
            break;
        case MODE_F_STATE_CHECKUP_MESSAGE:
            if (sigfox_rfp_test_mode_f_ctx.flags.ep_api_message_cplt == 1) {
                sigfox_rfp_test_mode_f_ctx.flags.ep_api_message_cplt = 0;
                sigfox_rfp_test_mode_f_ctx.progress_status.progress = 50;
                message_status = SIGFOX_EP_API_get_message_status();
                if (message_status.field.execution_error == 1)
                    goto errors;
                if (message_status.field.dl_frame == 1) {
#ifdef ERROR_CODES
                    sigfox_ep_api_status = SIGFOX_EP_API_get_dl_payload(dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES, &dl_rssi_dbm);
                    SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
                    SIGFOX_EP_API_get_dl_payload(dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES, &dl_rssi_dbm);
#endif
                    data[0] = 0x01;
                    for (data_cnt = 0; data_cnt < SIGFOX_DL_PAYLOAD_SIZE_BYTES; data_cnt++) {
                        if(dl_payload[data_cnt] != ((sfx_u8)0x30 + data_cnt)) {
                            data[0] = 0x00;
                        }
                    }
                } else
                    goto errors;
                _populate_test_param(&test_param);
                _populate_common_param(&message_param.common_parameters);
#ifndef SINGLE_FRAME
                message_param.common_parameters.number_of_frames = 1;
#endif
                message_param.bidirectional_flag = SFX_FALSE;
#if (defined UL_PAYLOAD_SIZE) && (UL_PAYLOAD_SIZE != 0)
                message_param.type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY;
                for (data_cnt = 1; data_cnt < UL_PAYLOAD_SIZE; data_cnt++) {
                    data[data_cnt] = (sfx_u8)0x00;
                }
                message_param.ul_payload = data;
#else
#if !(defined UL_PAYLOAD_SIZE) || (UL_PAYLOAD_SIZE > 0)
                message_param.ul_payload = SFX_NULL;
#endif
                if (data[0] == 0x00)
                    message_param.type = SIGFOX_APPLICATION_MESSAGE_TYPE_BIT0;
                if (data[0] == 0x01)
                    message_param.type = SIGFOX_APPLICATION_MESSAGE_TYPE_BIT1;
#endif
#ifdef ASYNCHRONOUS
            message_param.uplink_cplt_cb = SFX_NULL;
            message_param.downlink_cplt_cb = SFX_NULL;
            message_param.message_cplt_cb = &_SIGFOX_EP_API_message_cplt_cb;
#endif
#ifdef ERROR_CODES
            sigfox_ep_api_status = SIGFOX_EP_API_TEST_send_application_message(&message_param, &test_param);
            SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
            SIGFOX_EP_API_TEST_send_application_message(&message_param, &test_param);
#endif
#ifndef ASYNCHRONOUS
            _SIGFOX_EP_API_message_cplt_cb();
#endif
            sigfox_rfp_test_mode_f_ctx.test_state = MODE_F_STATE_END;
            }
            break;
        case MODE_F_STATE_END:
            sigfox_rfp_test_mode_f_ctx.progress_status.progress = 100;
            if (sigfox_rfp_test_mode_f_ctx.flags.ep_api_message_cplt == 1) {
                sigfox_rfp_test_mode_f_ctx.flags.ep_api_message_cplt = 0;
                message_status = SIGFOX_EP_API_get_message_status();
                if (message_status.field.execution_error == 1)
                    goto errors;
#ifdef ASYNCHRONOUS
                if (sigfox_rfp_test_mode_f_ctx.test_mode.cplt_cb != SFX_NULL) {
                    sigfox_rfp_test_mode_f_ctx.test_mode.cplt_cb();
                }
#endif
                sigfox_rfp_test_mode_f_ctx.test_state = MODE_F_STATE_WAIT;
            }
            break;
        default:
            goto errors;


    }
#ifndef ASYNCHRONOUS
    }
#endif
    RETURN();
errors:
    sigfox_rfp_test_mode_f_ctx.test_state = MODE_F_STATE_WAIT;
    sigfox_rfp_test_mode_f_ctx.progress_status.status.error = 1;
#ifdef ASYNCHRONOUS
    // test procedure done.
    sigfox_rfp_test_mode_f_ctx.test_mode.cplt_cb();
#endif
    RETURN();
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_F_get_progress_status_fn(void) {
 * \brief Get the progression status
 * \param[in]   none
 * \param[out]  none
 * \retval      Progression status
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_F_get_progress_status_fn(void) {
    return sigfox_rfp_test_mode_f_ctx.progress_status;
}
#endif //CERTIFICATION
