/*!*****************************************************************
 * \file    sigfox_rfp_test_mode_j.c
 * \brief   Sigfox addon RF & Protocol test mode J module
 * \details Send all supported types of Sigfox messages with all possible sizes.
 *          Each message has to be send following the below procedure :
 *          \arg Start a 18s timer.
 *          \arg Wait for the end of this 18s before sending a new message.
 *
 *          Sigfox messages could be :
 *          \arg Send bit (0 or 1).
 *          \arg Keep-Alive Control Message.
 *          \arg Send Frame (12 different payload), payload has to be set to 0x40 + byte index.
 *          \arg Send Frame (no payload).
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
#include "manuf/mcu_api.h"
#include "sigfox_error.h"
#include "sigfox_ep_api_test.h"
#ifdef SIGFOX_EP_CERTIFICATION

#define WINDOW_TIME_MS 18000
#define START_PAYLOAD 0x40

typedef struct test_mode_j_message_s test_mode_j_message_t;
typedef struct test_mode_j_message_s {
    SIGFOX_EP_ADDON_RFP_API_status_t (*send_ptr)(const test_mode_j_message_t *test_mode_j_message);
#ifndef SIGFOX_EP_UL_PAYLOAD_SIZE
    sfx_u8 size;
#endif
    union {
#ifdef SIGFOX_EP_APPLICATION_MESSAGES
        SIGFOX_application_message_type_t message_type;
#endif
#ifdef SIGFOX_EP_CONTROL_KEEP_ALIVE_MESSAGE
        SIGFOX_control_message_type_t control_type;
#endif
    } type;
} test_mode_j_message_t;

typedef struct {
    struct {
        sfx_u8 ep_api_message_cplt    : 1;
        sfx_u8 mcu_api_timer_cplt     : 1;
        sfx_u8 test_mode_req          : 1;
    }flags;
    sfx_u8 message_list_idx;
    SIGFOX_RFP_test_mode_t test_mode;
    SIGFOX_EP_ADDON_RFP_API_progress_status_t progress_status;
} SIGFOX_RFP_TEST_MODE_J_context_t;

static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_J_init_fn(SIGFOX_RFP_test_mode_t *test_mode_callback);
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_J_process_fn(void);
static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_J_get_progress_status_fn(void);
#ifdef SIGFOX_EP_CONTROL_KEEP_ALIVE_MESSAGE
static SIGFOX_EP_ADDON_RFP_API_status_t _send_control_message(const test_mode_j_message_t *test_mode_j_message);
#endif
#ifdef SIGFOX_EP_APPLICATION_MESSAGES
static SIGFOX_EP_ADDON_RFP_API_status_t _send_application_message(const test_mode_j_message_t *test_mode_j_message);
#endif

const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_J_fn = {
    .init_fn = &SIGFOX_RFP_TEST_MODE_J_init_fn,
    .process_fn = &SIGFOX_RFP_TEST_MODE_J_process_fn,
    .get_progress_status_fn = &SIGFOX_RFP_TEST_MODE_J_get_progress_status_fn,
};

static const test_mode_j_message_t MESSAGE_LIST[] = {
#ifdef SIGFOX_EP_APPLICATION_MESSAGES
#ifdef SIGFOX_EP_UL_PAYLOAD_SIZE
#if (SIGFOX_EP_UL_PAYLOAD_SIZE == 0)
    {.send_ptr = &_send_application_message, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BIT0},
    {.send_ptr = &_send_application_message, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BIT1},
    {.send_ptr = &_send_application_message, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_EMPTY},

#else
    {.send_ptr = &_send_application_message, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
#endif
#else
    {.send_ptr = &_send_application_message, .size = 0, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BIT0},
    {.send_ptr = &_send_application_message, .size = 0, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BIT1},
    {.send_ptr = &_send_application_message, .size = 1, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 2, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 3, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 4, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 5, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 6, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 7, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 8, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 9, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 10, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 11, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 12, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY},
    {.send_ptr = &_send_application_message, .size = 0, .type.message_type = SIGFOX_APPLICATION_MESSAGE_TYPE_EMPTY},
#endif
#endif
#ifdef SIGFOX_EP_CONTROL_KEEP_ALIVE_MESSAGE
#ifdef SIGFOX_EP_UL_PAYLOAD_SIZE
    {.send_ptr = &_send_control_message, .type.control_type = SIGFOX_CONTROL_MESSAGE_TYPE_KEEP_ALIVE},
#else
    {.send_ptr = &_send_control_message, .size = 0, .type.control_type = SIGFOX_CONTROL_MESSAGE_TYPE_KEEP_ALIVE},
#endif
#endif
};

static SIGFOX_RFP_TEST_MODE_J_context_t sigfox_rfp_test_mode_j_ctx = {
    .flags.ep_api_message_cplt = 0,
    .flags.mcu_api_timer_cplt = 0,
    .flags.test_mode_req = 0,
    .message_list_idx = 0,
    .test_mode.rc = SIGFOX_NULL,
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    .test_mode.ul_bit_rate = 0,
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    .test_mode.tx_power_dbm_eirp = 0,
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    .test_mode.process_cb = SIGFOX_NULL,
    .test_mode.cplt_cb = SIGFOX_NULL,
#endif
    .progress_status.status.error = 0,
    .progress_status.progress = 0,
};

/*!******************************************************************
 * \fn static void _SIGFOX_EP_API_message_cplt_cb(void)
 * \brief Message completion callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static void _SIGFOX_EP_API_message_cplt_cb(void) {
    // Local variables.
    sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt = 1;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    if (sigfox_rfp_test_mode_j_ctx.test_mode.process_cb != SIGFOX_NULL) {
        sigfox_rfp_test_mode_j_ctx.test_mode.process_cb();
    }
#endif
}

/*!******************************************************************
 * \fn static void _MCU_API_timer_cplt_cb(void)
 * \brief Timer completion callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static void _MCU_API_timer_cplt_cb(void) {
    sigfox_rfp_test_mode_j_ctx.flags.mcu_api_timer_cplt = 1;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    if (sigfox_rfp_test_mode_j_ctx.test_mode.process_cb != SIGFOX_NULL) {
        sigfox_rfp_test_mode_j_ctx.test_mode.process_cb();
    }
#endif
}

#ifdef SIGFOX_EP_CONTROL_KEEP_ALIVE_MESSAGE
/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t _send_control_message(const test_mode_j_message_t *test_mode_j_message) {
 * \brief Send control message
 * \param[in]   test_mode_j_message: message to send
 * \param[out]  none
 * \retval      SIGFOX_EP_ADDON_RFP_API_status_t
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t _send_control_message(const test_mode_j_message_t *test_mode_j_message) {
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
    MCU_API_status_t mcu_api_status = MCU_API_SUCCESS;
#endif
    MCU_API_timer_t timer;
    SIGFOX_EP_API_TEST_parameters_t test_param = {0};
    SIGFOX_EP_API_control_message_t control_message = {0};
    // Configure control message structure
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    /*TODO manage 100/600*/
    control_message.common_parameters.ul_bit_rate = sigfox_rfp_test_mode_j_ctx.test_mode.ul_bit_rate;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    control_message.common_parameters.tx_power_dbm_eirp = sigfox_rfp_test_mode_j_ctx.test_mode.tx_power_dbm_eirp;
#endif
#ifndef SIGFOX_EP_SINGLE_FRAME
    control_message.common_parameters.number_of_frames = 3;
#ifndef SIGFOX_EP_T_IFU_MS
    control_message.common_parameters.t_ifu_ms = 500;
#endif
#endif
#ifdef SIGFOX_EP_PUBLIC_KEY_CAPABLE
    control_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
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
    test_param.lbt_cs_max_duration_first_frame_ms = 0;
    test_param.flags.field.tx_control_lbt_enable = SIGFOX_FALSE;
#endif
#if (defined SIGFOX_EP_REGULATORY) && (defined SIGFOX_EP_SPECTRUM_ACCESS_LDC)
    test_param.flags.field.tx_control_ldc_enable = SIGFOX_FALSE;
#endif
    control_message.type = test_mode_j_message->type.control_type;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    control_message.uplink_cplt_cb = SIGFOX_NULL;
    control_message.message_cplt_cb = &_SIGFOX_EP_API_message_cplt_cb;
    timer.cplt_cb = &_MCU_API_timer_cplt_cb;
#endif
    // Configure timer structure
    timer.duration_ms = WINDOW_TIME_MS;
    timer.instance = MCU_API_TIMER_INSTANCE_ADDON_RFP;
    // Configure timer and send control message
#ifdef SIGFOX_EP_ERROR_CODES
    mcu_api_status = MCU_API_timer_start(&timer);
    MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
    sigfox_ep_api_status = SIGFOX_EP_API_TEST_send_control_message(&control_message, &test_param);
    SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
    MCU_API_timer_start(&timer);
    SIGFOX_EP_API_TEST_send_control_message(&control_message, &test_param);
#endif
#ifndef SIGFOX_EP_ASYNCHRONOUS
    _SIGFOX_EP_API_message_cplt_cb();
#ifdef SIGFOX_EP_ERROR_CODES
    mcu_api_status = MCU_API_timer_wait_cplt(MCU_API_TIMER_INSTANCE_ADDON_RFP);
    MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
#else
    MCU_API_timer_wait_cplt(MCU_API_TIMER_INSTANCE_ADDON_RFP);
#endif
    _MCU_API_timer_cplt_cb();

#endif
#ifdef SIGFOX_EP_ERROR_CODES
errors:
#endif
    SIGFOX_RETURN();
}
#endif
#ifdef SIGFOX_EP_APPLICATION_MESSAGES
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_RFP_API_status_t _send_application_message(const test_mode_j_message_t *test_mode_j_message)
 * \brief Send application message
 * \param[in]   test_mode_j_message: message to send
 * \param[out]  none
 * \retval      SIGFOX_EP_ADDON_RFP_API_status_t
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t _send_application_message(const test_mode_j_message_t *test_mode_j_message) {
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
    MCU_API_status_t mcu_api_status = MCU_API_SUCCESS;
#endif
    MCU_API_timer_t timer;
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
    // Configure control message structure
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    /*TODO manage 100/600*/
    application_message.common_parameters.ul_bit_rate = sigfox_rfp_test_mode_j_ctx.test_mode.ul_bit_rate;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    application_message.common_parameters.tx_power_dbm_eirp = sigfox_rfp_test_mode_j_ctx.test_mode.tx_power_dbm_eirp;
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
    test_param.tx_frequency_hz = 0;
    test_param.flags.all = 0xFF;
#if (defined SIGFOX_EP_REGULATORY) && (defined SIGFOX_EP_SPECTRUM_ACCESS_FH)
    test_param.flags.field.tx_control_fh_enable = SIGFOX_FALSE;
#endif
#if (defined SIGFOX_EP_REGULATORY) && (defined SIGFOX_EP_SPECTRUM_ACCESS_LBT)
    test_param.flags.field.tx_control_lbt_enable = SIGFOX_FALSE;
#endif
#if (defined SIGFOX_EP_REGULATORY) && (defined SIGFOX_EP_SPECTRUM_ACCESS_LDC)
    test_param.flags.field.tx_control_ldc_enable = SIGFOX_FALSE;
#endif
    application_message.type = test_mode_j_message->type.message_type;
#ifdef SIGFOX_EP_UL_PAYLOAD_SIZE
#if (SIGFOX_EP_UL_PAYLOAD_SIZE > 0)
    for (data_cnt = 0; data_cnt < SIGFOX_EP_UL_PAYLOAD_SIZE; data_cnt++) {
        data[data_cnt] = START_PAYLOAD + data_cnt;
    }
    application_message.ul_payload = data;
#endif
#else
    application_message.ul_payload_size_bytes = test_mode_j_message->size;
    for (data_cnt = 0; data_cnt < test_mode_j_message->size; data_cnt++) {
        data[data_cnt] = START_PAYLOAD + data_cnt;
    }
    application_message.ul_payload = data;
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    application_message.uplink_cplt_cb = SIGFOX_NULL;
    application_message.message_cplt_cb = &_SIGFOX_EP_API_message_cplt_cb;
    timer.cplt_cb = &_MCU_API_timer_cplt_cb;
#endif
    // Configure timer structure
    timer.duration_ms = WINDOW_TIME_MS;
    timer.instance = MCU_API_TIMER_INSTANCE_ADDON_RFP;
    timer.reason = MCU_API_TIMER_REASON_ADDON_RFP;
    // Start timer and send Application message
#ifdef SIGFOX_EP_ERROR_CODES
    mcu_api_status = MCU_API_timer_start(&timer);
    MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
    sigfox_ep_api_status = SIGFOX_EP_API_TEST_send_application_message(&application_message, &test_param);
    SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
    MCU_API_timer_start(&timer);
    SIGFOX_EP_API_TEST_send_application_message(&application_message, &test_param);
#endif
#ifndef SIGFOX_EP_ASYNCHRONOUS
    _SIGFOX_EP_API_message_cplt_cb();
#ifdef SIGFOX_EP_ERROR_CODES
    mcu_api_status = MCU_API_timer_wait_cplt(MCU_API_TIMER_INSTANCE_ADDON_RFP);
    MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
#else
    MCU_API_timer_wait_cplt(MCU_API_TIMER_INSTANCE_ADDON_RFP);
#endif
    _MCU_API_timer_cplt_cb();

#endif
#ifdef SIGFOX_EP_ERROR_CODES
errors:
#endif
    SIGFOX_RETURN();
}
#endif

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_J_init_fn(SIGFOX_RFP_test_mode_t *rfp_test_mode)
 * \brief Start Test Mode A.
 * \param[in]   rfp_test_mode: test mode parameters
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_J_init_fn(SIGFOX_RFP_test_mode_t *rfp_test_mode) {
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
    sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt = 0;
    sigfox_rfp_test_mode_j_ctx.flags.mcu_api_timer_cplt = 0;
    sigfox_rfp_test_mode_j_ctx.message_list_idx = 0;
    sigfox_rfp_test_mode_j_ctx.progress_status.status.error = 0;
    sigfox_rfp_test_mode_j_ctx.progress_status.progress = 0;
    // Store test mode parameters locally.
    sigfox_rfp_test_mode_j_ctx.test_mode.rc = rfp_test_mode->rc;
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    sigfox_rfp_test_mode_j_ctx.test_mode.ul_bit_rate = rfp_test_mode->ul_bit_rate;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    sigfox_rfp_test_mode_j_ctx.test_mode.tx_power_dbm_eirp = rfp_test_mode->tx_power_dbm_eirp,
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_rfp_test_mode_j_ctx.test_mode.process_cb = rfp_test_mode->process_cb;
    sigfox_rfp_test_mode_j_ctx.test_mode.cplt_cb = rfp_test_mode->cplt_cb;
#endif
    sigfox_rfp_test_mode_j_ctx.flags.test_mode_req = 1;
#ifdef SIGFOX_EP_PARAMETERS_CHECK
errors:
#endif
    SIGFOX_RETURN();
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_J_process_fn(void)
 * \brief Process Test Mode C.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_J_process_fn(void) {
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    MCU_API_status_t mcu_api_status = MCU_API_SUCCESS;
#endif
    SIGFOX_EP_API_message_status_t message_status;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sfx_u16 tmp;
#endif

    if (sigfox_rfp_test_mode_j_ctx.flags.test_mode_req == 1) {
        sigfox_rfp_test_mode_j_ctx.flags.test_mode_req = 0;
#ifdef SIGFOX_EP_ERROR_CODES
        status = MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
        SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
        MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
#endif
    }
#ifdef SIGFOX_EP_ASYNCHRONOUS
    else {
        if ((sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt == 1) && (sigfox_rfp_test_mode_j_ctx.flags.mcu_api_timer_cplt == 1)) {
            sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt = 0;
            sigfox_rfp_test_mode_j_ctx.flags.mcu_api_timer_cplt = 0;
            message_status = SIGFOX_EP_API_get_message_status();
            if (message_status.field.execution_error == 1 || message_status.field.network_error == 1) {
                goto errors;
            }
            sigfox_rfp_test_mode_j_ctx.message_list_idx++;
#ifdef SIGFOX_EP_ERROR_CODES
            mcu_api_status = MCU_API_timer_stop(MCU_API_TIMER_INSTANCE_ADDON_RFP);
            MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
#else
            MCU_API_timer_stop(MCU_API_TIMER_INSTANCE_ADDON_RFP);
#endif
            if (sigfox_rfp_test_mode_j_ctx.message_list_idx < sizeof(MESSAGE_LIST) / sizeof(test_mode_j_message_t)) {
                tmp = 100 * (sigfox_rfp_test_mode_j_ctx.message_list_idx);
                tmp /= (sizeof(MESSAGE_LIST) / sizeof(test_mode_j_message_t));
                sigfox_rfp_test_mode_j_ctx.progress_status.progress = (sfx_u8) tmp;
#ifdef SIGFOX_EP_ERROR_CODES

                status = MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
                if (status != SIGFOX_EP_ADDON_RFP_API_SUCCESS) {
                    MCU_API_timer_stop(MCU_API_TIMER_INSTANCE_ADDON_RFP);
                    goto errors;
                }
#else
                MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
#endif
            } else {
                if (sigfox_rfp_test_mode_j_ctx.test_mode.cplt_cb != SIGFOX_NULL) {
                    sigfox_rfp_test_mode_j_ctx.progress_status.progress = 100;
                    sigfox_rfp_test_mode_j_ctx.test_mode.cplt_cb();
                }
            }
        }
    }
#else
    while (sigfox_rfp_test_mode_j_ctx.message_list_idx < sizeof(MESSAGE_LIST) / sizeof(test_mode_j_message_t)) {
        if ((sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt == 1) && (sigfox_rfp_test_mode_j_ctx.flags.mcu_api_timer_cplt == 1)) {
            sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt = 0;
            sigfox_rfp_test_mode_j_ctx.flags.mcu_api_timer_cplt = 0;
            message_status = SIGFOX_EP_API_get_message_status();
            if (message_status.field.execution_error == 1 || message_status.field.network_error == 1) {
                goto errors;
            }
#ifdef SIGFOX_EP_ERROR_CODES
            mcu_api_status = MCU_API_timer_stop(MCU_API_TIMER_INSTANCE_ADDON_RFP);
            MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
#else
            MCU_API_timer_stop(MCU_API_TIMER_INSTANCE_ADDON_RFP);
#endif
            sigfox_rfp_test_mode_j_ctx.message_list_idx++;
            if (sigfox_rfp_test_mode_j_ctx.message_list_idx < sizeof(MESSAGE_LIST) / sizeof(test_mode_j_message_t)) {
#ifdef SIGFOX_EP_ERROR_CODES
                status = MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
                SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
                MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
#endif
            }
        }
    }
    sigfox_rfp_test_mode_j_ctx.progress_status.progress = 100;
#endif
    SIGFOX_RETURN();
errors:
    sigfox_rfp_test_mode_j_ctx.progress_status.status.error = 1;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    // test procedure done.
    sigfox_rfp_test_mode_j_ctx.test_mode.cplt_cb();
#endif
    SIGFOX_RETURN();
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_J_get_progress_status_fn(void) {
 * \brief Get the progression status
 * \param[in]   none
 * \param[out]  none
 * \retval      Progression status
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_J_get_progress_status_fn(void) {
    return sigfox_rfp_test_mode_j_ctx.progress_status;
}
#endif
