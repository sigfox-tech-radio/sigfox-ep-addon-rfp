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

#include "tests_mode/sigfox_rfp_test_mode_types.h"
#include "manuf/mcu_api.h"
#include "sigfox_error.h"
#include "sigfox_ep_api_test.h"
#ifdef CERTIFICATION

#define WINDOW_TIME_MS 18000
#define TIMER_INSTANCE MCU_API_TIMER_3
#define START_PAYLOAD 0x40

typedef struct test_mode_c_message_s test_mode_c_message_t;
typedef struct test_mode_c_message_s {
    SIGFOX_EP_ADDON_RFP_API_status_t (*send_ptr)(const test_mode_c_message_t *test_mode_c_message);
#ifndef UL_PAYLOAD_SIZE
    sfx_u8 size;
#endif
    union {
#ifdef APPLICATION_MESSAGES
        SIGFOX_application_message_type_t message_type;
#endif
#ifdef CONTROL_KEEP_ALIVE_MESSAGE
        SIGFOX_control_message_type_t control_type;
#endif
    }type;
}test_mode_c_message_t;

typedef struct {
    struct {
        unsigned ep_api_message_cplt    : 1;
        unsigned ep_api_message_error   : 1;
        unsigned mcu_api_timer_cplt     : 1;
        unsigned test_mode_req          : 1;
    }flags;
    sfx_u8 message_list_idx;
    SIGFOX_RFP_test_mode_t test_mode;
    SIGFOX_EP_ADDON_RFP_API_progress_status_t progress_status;
}SIGFOX_RFP_TEST_MODE_J_context_t;

static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_J_init_fn(SIGFOX_RFP_test_mode_t *test_mode_callback);
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_J_process_fn(void);
static SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_RFP_TEST_MODE_J_get_progress_status_fn(void);
#ifdef CONTROL_KEEP_ALIVE_MESSAGE
static SIGFOX_EP_ADDON_RFP_API_status_t _send_control_message(const test_mode_c_message_t *test_mode_c_message);
#endif
#ifdef APPLICATION_MESSAGES
static SIGFOX_EP_ADDON_RFP_API_status_t _send_application_message(const test_mode_c_message_t *test_mode_c_message);
#endif

const SIGFOX_RFP_test_mode_fn_t SIGFOX_RFP_TEST_MODE_J_fn = {
        .init_fn = &SIGFOX_RFP_TEST_MODE_J_init_fn,
        .process_fn = &SIGFOX_RFP_TEST_MODE_J_process_fn,
        .get_progress_status_fn = &SIGFOX_RFP_TEST_MODE_J_get_progress_status_fn,
};

static const test_mode_c_message_t MESSAGE_LIST[] = {
#ifdef APPLICATION_MESSAGES
#ifdef UL_PAYLOAD_SIZE
#if (UL_PAYLOAD_SIZE == 0)
        {&_send_application_message, {SIGFOX_APPLICATION_MESSAGE_TYPE_BIT0}},
        {&_send_application_message, {SIGFOX_APPLICATION_MESSAGE_TYPE_BIT1}},
        {&_send_application_message, {SIGFOX_APPLICATION_MESSAGE_TYPE_EMPTY}},

#else
        {&_send_application_message, {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
#endif
#else
        {&_send_application_message, 0,  {SIGFOX_APPLICATION_MESSAGE_TYPE_BIT0}},
        {&_send_application_message, 0,  {SIGFOX_APPLICATION_MESSAGE_TYPE_BIT1}},
        {&_send_application_message, 1,  {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 2,  {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 3,  {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 4,  {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 5,  {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 6,  {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 7,  {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 8,  {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 9,  {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 10, {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 11, {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 12, {SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY}},
        {&_send_application_message, 0,  {SIGFOX_APPLICATION_MESSAGE_TYPE_EMPTY}},
#endif
#endif
#ifdef CONTROL_KEEP_ALIVE_MESSAGE
#ifdef UL_PAYLOAD_SIZE
        {&_send_control_message, {SIGFOX_CONTROL_MESSAGE_TYPE_KEEP_ALIVE}},
#else
        {&_send_control_message, 0, {SIGFOX_CONTROL_MESSAGE_TYPE_KEEP_ALIVE}},
#endif
#endif
};

static SIGFOX_RFP_TEST_MODE_J_context_t sigfox_rfp_test_mode_j_ctx = {
        .flags.ep_api_message_cplt      = 0,
        .flags.ep_api_message_error     = 0,
        .flags.mcu_api_timer_cplt       = 0,
        .flags.test_mode_req            = 0,
        .message_list_idx               = 0,
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
 * \fn static void _SIGFOX_EP_API_message_cplt_cb(void)
 * \brief Message completion callback.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static void _SIGFOX_EP_API_message_cplt_cb(void) {
    // Local variables.
    SIGFOX_EP_API_message_status_t message_status;
    message_status = SIGFOX_EP_API_get_message_status();
    if (message_status.app_frame_1 == 1) {
        sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt = 1;
    }
    if (message_status.error == 1)
        sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_error = 1;
#ifdef ASYNCHRONOUS
    if (sigfox_rfp_test_mode_j_ctx.test_mode.process_cb != SFX_NULL)
        sigfox_rfp_test_mode_j_ctx.test_mode.process_cb();
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
#ifdef ASYNCHRONOUS
    if (sigfox_rfp_test_mode_j_ctx.test_mode.process_cb != SFX_NULL)
        sigfox_rfp_test_mode_j_ctx.test_mode.process_cb();
#endif
}

#ifdef CONTROL_KEEP_ALIVE_MESSAGE
/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t _send_control_message(const test_mode_c_message_t *test_mode_c_message) {
 * \brief Send control message
 * \param[in]   test_mode_c_message: message to send
 * \param[out]  none
 * \retval      SIGFOX_EP_ADDON_RFP_API_status_t
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t _send_control_message(const test_mode_c_message_t *test_mode_c_message) {
#ifdef ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    SIGFOX_EP_API_status_t ep_api_status = SIGFOX_EP_API_SUCCESS;
    MCU_API_status_t mcu_status = MCU_API_SUCCESS;
#endif
    MCU_API_timer_t timer;
    SIGFOX_EP_API_TEST_parameters_t test_param = {0};
    SIGFOX_EP_API_control_message_t control_message = {0};
    //Configure control message structure
#ifndef UL_BIT_RATE_BPS
    /*TODO manage 100/600*/
    control_message.common_parameters.ul_bit_rate = sigfox_rfp_test_mode_j_ctx.test_mode.ul_bit_rate;
#endif
#ifndef TX_POWER_DBM_EIRP
    control_message.common_parameters.tx_power_dbm_eirp = sigfox_rfp_test_mode_j_ctx.test_mode.tx_power_dbm_eirp;
#endif
#ifndef SINGLE_FRAME
    control_message.common_parameters.number_of_frames = 3;
#ifndef T_IFU_MS
    control_message.common_parameters.t_ifu_ms = 500;
#endif
#endif
#ifdef PUBLIC_KEY_CAPABLE
    control_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
#endif
//    control_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
//    control_message.common_parameters.tx_frequency_hz = 0;
    test_param.tx_frequency_hz = 0;
#if (defined REGULATORY) && (defined SPECTRUM_ACCESS_FH)
    test_param.fh_timer_enable = SFX_FALSE;
#endif
#if (defined REGULATORY) && (defined SPECTRUM_ACCESS_LBT)
    test_param.lbt_enable = SFX_FALSE;
#endif
    control_message.type = test_mode_c_message->type.control_type;
#ifdef ASYNCHRONOUS
    control_message.uplink_cplt_cb = SFX_NULL;
    control_message.message_cplt_cb = &_SIGFOX_EP_API_message_cplt_cb;
    timer.cplt_cb = &_MCU_API_timer_cplt_cb;
#endif
    //Configure timer structure
    timer.duration_ms = WINDOW_TIME_MS;
    timer.instance = TIMER_INSTANCE;
    //Configure timer and send control message
#ifdef ERROR_CODES
    mcu_status = MCU_API_timer_start(&timer);
    MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
    ep_api_status = SIGFOX_EP_API_TEST_send_control_message(&control_message, &test_param);
    SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
    MCU_API_timer_start(&timer);
    SIGFOX_EP_API_TEST_send_control_message(&control_message, &test_param);
#endif
#ifndef ASYNCHRONOUS
    _SIGFOX_EP_API_message_cplt_cb();
#ifdef ERROR_CODES
    mcu_status = MCU_API_timer_wait_cplt(TIMER_INSTANCE);
    MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
#else
    MCU_API_timer_wait_cplt(TIMER_INSTANCE);
#endif
    _MCU_API_timer_cplt_cb();

#endif
#ifdef ERROR_CODES
errors:
#endif
    RETURN();
}
#endif
#ifdef APPLICATION_MESSAGES
/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t _send_application_message(void)
 * \brief Send application message
 * \param[in]   test_mode_c_message: message to send
 * \param[out]  none
 * \retval      SIGFOX_EP_ADDON_RFP_API_status_t
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t _send_application_message(const test_mode_c_message_t *test_mode_c_message) {
#ifdef ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    SIGFOX_EP_API_status_t ep_api_status = SIGFOX_EP_API_SUCCESS;
    MCU_API_status_t mcu_status = MCU_API_SUCCESS;
#endif
    MCU_API_timer_t timer;
    SIGFOX_EP_API_TEST_parameters_t test_param = {0};
    SIGFOX_EP_API_application_message_t application_message = {0};
#ifdef UL_PAYLOAD_SIZE
#if (UL_PAYLOAD_SIZE != 0)
    sfx_u8 data_cnt;
    sfx_u8 data[UL_PAYLOAD_SIZE] = {0x00};
#endif
#else
    sfx_u8 data_cnt;
    sfx_u8 data[SIGFOX_UL_PAYLOAD_MAX_SIZE_BYTES] = {0x00};
#endif
    //Configure control message structure
#ifndef UL_BIT_RATE_BPS
    /*TODO manage 100/600*/
    application_message.common_parameters.ul_bit_rate = sigfox_rfp_test_mode_j_ctx.test_mode.ul_bit_rate;
#endif
#ifndef TX_POWER_DBM_EIRP
    application_message.common_parameters.tx_power_dbm_eirp = sigfox_rfp_test_mode_j_ctx.test_mode.tx_power_dbm_eirp;
#endif
#ifndef SINGLE_FRAME
    application_message.common_parameters.number_of_frames = 3;
#ifndef T_IFU_MS
    application_message.common_parameters.t_ifu_ms = 500;
#endif
#endif
#ifdef PUBLIC_KEY_CAPABLE
    application_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
#endif
//    application_message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
//    application_message.common_parameters.tx_frequency_hz = 0;
    test_param.tx_frequency_hz = 0;
#if (defined REGULATORY) && (defined SPECTRUM_ACCESS_FH)
    test_param.fh_timer_enable = SFX_FALSE;
#endif
#if (defined REGULATORY) && (defined SPECTRUM_ACCESS_LBT)
    test_param.lbt_enable = SFX_FALSE;
#endif
    application_message.type = test_mode_c_message->type.message_type;
#ifdef UL_PAYLOAD_SIZE
#if (UL_PAYLOAD_SIZE > 0)
    for (data_cnt = 0; data_cnt < UL_PAYLOAD_SIZE; data_cnt++)
        data[data_cnt] = START_PAYLOAD + data_cnt;
    application_message.ul_payload = data;
#endif
#else
    application_message.ul_payload_size_bytes = test_mode_c_message->size;
    for (data_cnt = 0; data_cnt < test_mode_c_message->size; data_cnt++)
        data[data_cnt] = START_PAYLOAD + data_cnt;
    application_message.ul_payload = data;
#endif
#ifdef ASYNCHRONOUS
    application_message.uplink_cplt_cb = SFX_NULL;
    application_message.message_cplt_cb = &_SIGFOX_EP_API_message_cplt_cb;
    timer.cplt_cb = &_MCU_API_timer_cplt_cb;
#endif
    //Configure timer structure
    timer.duration_ms = WINDOW_TIME_MS;
    timer.instance = TIMER_INSTANCE;
    //Start timer and send Application message
#ifdef ERROR_CODES
    mcu_status = MCU_API_timer_start(&timer);
    MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
    ep_api_status = SIGFOX_EP_API_TEST_send_application_message(&application_message, &test_param);
    SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API);
#else
    MCU_API_timer_start(&timer);
    SIGFOX_EP_API_TEST_send_application_message(&application_message, &test_param);
#endif
#ifndef ASYNCHRONOUS
    _SIGFOX_EP_API_message_cplt_cb();
#ifdef ERROR_CODES
    mcu_status = MCU_API_timer_wait_cplt(TIMER_INSTANCE);
    MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
#else
    MCU_API_timer_wait_cplt(TIMER_INSTANCE);
#endif
    _MCU_API_timer_cplt_cb();

#endif
#ifdef ERROR_CODES
errors:
#endif
    RETURN();
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
    sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt = 0;
    sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_error = 0;
    sigfox_rfp_test_mode_j_ctx.flags.mcu_api_timer_cplt = 0;
    sigfox_rfp_test_mode_j_ctx.message_list_idx = 0;
    sigfox_rfp_test_mode_j_ctx.progress_status.status.error = 0;
    sigfox_rfp_test_mode_j_ctx.progress_status.progress = 0;
    // Store test mode parameters locally.
    sigfox_rfp_test_mode_j_ctx.test_mode.rc = rfp_test_mode->rc;
#ifndef UL_BIT_RATE_BPS
    sigfox_rfp_test_mode_j_ctx.test_mode.ul_bit_rate = rfp_test_mode->ul_bit_rate;
#endif
#ifndef TX_POWER_DBM_EIRP
    sigfox_rfp_test_mode_j_ctx.test_mode.tx_power_dbm_eirp = rfp_test_mode->tx_power_dbm_eirp,
#endif
#ifdef ASYNCHRONOUS
    sigfox_rfp_test_mode_j_ctx.test_mode.process_cb = rfp_test_mode->process_cb;
    sigfox_rfp_test_mode_j_ctx.test_mode.cplt_cb = rfp_test_mode->cplt_cb;
#endif
    sigfox_rfp_test_mode_j_ctx.flags.test_mode_req = 1;
#ifdef PARAMETERS_CHECK
errors:
#endif
    RETURN();
}

/*!******************************************************************
 * \fn static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_J_process_fn(void)
 * \brief Process Test Mode C.
 * \param[in]   none
 * \param[out]  none
 * \retval      none
 *******************************************************************/
static SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_RFP_TEST_MODE_J_process_fn(void) {
#ifdef ERROR_CODES
    SIGFOX_EP_ADDON_RFP_API_status_t status = SIGFOX_EP_ADDON_RFP_API_SUCCESS;
    MCU_API_status_t mcu_status = MCU_API_SUCCESS;
#endif
#ifdef ASYNCHRONOUS
    sfx_u16 tmp;
#endif

    if (sigfox_rfp_test_mode_j_ctx.flags.test_mode_req == 1) {
        sigfox_rfp_test_mode_j_ctx.flags.test_mode_req = 0;
#ifdef ERROR_CODES
        status = MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
        CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
        MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
#endif
    }
#ifdef ASYNCHRONOUS
    else {
        if (sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_error == 1) {
            sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_error = 0;
            goto errors;
        }
        if ( (sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt == 1) &&
                (sigfox_rfp_test_mode_j_ctx.flags.mcu_api_timer_cplt == 1)) {
            sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt = 0;
            sigfox_rfp_test_mode_j_ctx.flags.mcu_api_timer_cplt = 0;
            sigfox_rfp_test_mode_j_ctx.message_list_idx++;
#ifdef ERROR_CODES
            mcu_status = MCU_API_timer_stop(TIMER_INSTANCE);
            MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
#else
            MCU_API_timer_stop(TIMER_INSTANCE);
#endif
            if (sigfox_rfp_test_mode_j_ctx.message_list_idx < sizeof(MESSAGE_LIST) / sizeof(test_mode_c_message_t)) {
                tmp = 100 * (sigfox_rfp_test_mode_j_ctx.message_list_idx);
                tmp /= (sizeof(MESSAGE_LIST) / sizeof(test_mode_c_message_t));
                sigfox_rfp_test_mode_j_ctx.progress_status.progress = (sfx_u8)tmp;
#ifdef ERROR_CODES

                status = MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
                CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
                MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
#endif
            } else {
                if (sigfox_rfp_test_mode_j_ctx.test_mode.cplt_cb != SFX_NULL) {
                    sigfox_rfp_test_mode_j_ctx.progress_status.progress = 100;
                    sigfox_rfp_test_mode_j_ctx.test_mode.cplt_cb();
                }
            }
        }
    }
#else
    while(sigfox_rfp_test_mode_j_ctx.message_list_idx < sizeof(MESSAGE_LIST) / sizeof(test_mode_c_message_t)) {
        if (sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_error == 1) {
            sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_error = 0;
            goto errors;
        }
        if ((sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt == 1) &&
                (sigfox_rfp_test_mode_j_ctx.flags.mcu_api_timer_cplt == 1)) {
            sigfox_rfp_test_mode_j_ctx.flags.ep_api_message_cplt = 0;
            sigfox_rfp_test_mode_j_ctx.flags.mcu_api_timer_cplt = 0;
#ifdef ERROR_CODES
            mcu_status = MCU_API_timer_stop(TIMER_INSTANCE);
            MCU_API_check_status(SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API);
#else
            MCU_API_timer_stop(TIMER_INSTANCE);
#endif
            sigfox_rfp_test_mode_j_ctx.message_list_idx++;
            if (sigfox_rfp_test_mode_j_ctx.message_list_idx < sizeof(MESSAGE_LIST) / sizeof(test_mode_c_message_t)) {
#ifdef ERROR_CODES
                status = MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
                CHECK_STATUS(SIGFOX_EP_ADDON_RFP_API_SUCCESS);
#else
                MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx].send_ptr(&MESSAGE_LIST[sigfox_rfp_test_mode_j_ctx.message_list_idx]);
#endif
            }
        }
    }
    sigfox_rfp_test_mode_j_ctx.progress_status.progress = 100;
#endif
    RETURN();
errors:
    sigfox_rfp_test_mode_j_ctx.progress_status.status.error = 1;
#ifdef ASYNCHRONOUS
    // test procedure done.
    MCU_API_timer_stop(TIMER_INSTANCE);
    sigfox_rfp_test_mode_j_ctx.test_mode.cplt_cb();
#endif
    RETURN();
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
