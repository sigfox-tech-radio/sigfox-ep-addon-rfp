/*!*****************************************************************
 * \file    sigfox_ep_addon_rfp_api.h
 * \brief   [mandatory]One Line file Description
 * \details This file provides firmware functions to manage the following
 *          functionalities of the Serial Peripheral Interface (SPI) peripheral:
 *           + Initialization and de-initialization functions
 *           + IO operation functions
 *           + Peripheral Control functions
 *           + Peripheral State functions
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

#ifndef __SIGFOX_EP_ADDON_RFP_API_H__
#define __SIGFOX_EP_ADDON_RFP_API_H__

#ifndef SIGFOX_EP_DISABLE_FLAGS_FILE
#include "sigfox_ep_flags.h"
#endif
#include "sigfox_types.h"
#include "sigfox_error.h"

#ifdef SIGFOX_EP_CERTIFICATION

#ifdef SIGFOX_EP_ERROR_CODES
typedef enum {
    SIGFOX_EP_ADDON_RFP_API_SUCCESS = 0,
    SIGFOX_EP_ADDON_RFP_API_ERROR_NULL_PARAMETER,
    SIGFOX_EP_ADDON_RFP_API_ERROR_EP_API,
    SIGFOX_EP_ADDON_RFP_API_ERROR_MCU_API,
    SIGFOX_EP_ADDON_RFP_API_ERROR_STATE,
    SIGFOX_EP_ADDON_RFP_API_ERROR_TEST_MODE,
    SIGFOX_EP_ADDON_RFP_API_ERROR_TEST_MODE_START,
    SIGFOX_EP_ADDON_RFP_API_ERROR_TEST_MODE_PROCESS,
} SIGFOX_EP_ADDON_RFP_API_status_t;
#else
typedef void SIGFOX_EP_ADDON_RFP_API_status_t;
#endif

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*!******************************************************************
 * \brief Sigfox EP ADDON RFP API callback functions.
 * \fn SIGFOX_EP_ADDON_RFP_API_process_cb_t:      Will be called each time a low level IRQ is handled by the addon library. Warning: runs in a IRQ context. Should only change variables state, and call as soon as possible @ref SIGFOX_EP_ADDON_EP_API_process.
 * \fn SIGFOX_EP_ADDON_RFP_API_test_mode_cplt_cb_t:  Will be called when the test mode sequence is done. Optional, could be set to NULL.
 *******************************************************************/
typedef void (*SIGFOX_EP_ADDON_RFP_API_process_cb_t)(void);
typedef void (*SIGFOX_EP_ADDON_RFP_API_test_mode_cplt_cb_t)(void);
#endif

#ifdef SIGFOX_EP_BIDIRECTIONAL
/*!******************************************************************
 * \brief Sigfox EP ADDON RFP test mode D callback function.
 * \brief This callback replaces the previous MCU_API_print_dl_payload() function defined in the Sigfox EP library until v3.6.
 * \fn SIGFOX_EP_ADDON_RFP_API_downlink_cplt_cb_t:  Will be called as soon as a valid downlink message is received.
 *******************************************************************/
typedef void (*SIGFOX_EP_ADDON_RFP_API_downlink_cplt_cb_t)(sfx_u8 *dl_payload, sfx_u8 dl_payload_size, sfx_s16 rssi_dbm);
#endif

/*!******************************************************************
 * \enum SIGFOX_EP_ADDON_RFP_API_config_t
 * \briefS Sigfox EP ADDON RFP configuration structure.
 *******************************************************************/
typedef struct {
    const SIGFOX_rc_t *rc;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    SIGFOX_EP_ADDON_RFP_API_process_cb_t process_cb;
#endif
#ifndef SIGFOX_EP_MESSAGE_COUNTER_ROLLOVER
    SIGFOX_message_counter_rollover_t message_counter_rollover;
#endif
} SIGFOX_EP_ADDON_RFP_API_config_t;

/*!******************************************************************
 * \enum SIGFOX_EP_ADDON_RFP_API_test_mode_t
 * \brief RF and protocol Test mode type. All tests modes reference are described in Sigfox RF & Protocol Test Specification document chapter 5 (https://support.sigfox.com/docs/rf-protocol-test-specification)
 *******************************************************************/
typedef enum {
    SIGFOX_EP_ADDON_RFP_API_TEST_MODE_C = 0, /*!< Only BPSK with Synchro Bit + Synchro frame + PN sequence : no hopping centered on the TX_frequency */
    SIGFOX_EP_ADDON_RFP_API_TEST_MODE_J = 1, /*!< with full protocol with AES key: send all SIGFOX protocol frames available with hopping */
#if defined SIGFOX_EP_BIDIRECTIONAL
    SIGFOX_EP_ADDON_RFP_API_TEST_MODE_F = 2, /*!< with full protocol with AES key: send SIGFOX protocol frames with bidirectional flag set */
    SIGFOX_EP_ADDON_RFP_API_TEST_MODE_D = 3, /*!< with known pattern with SB + SF + Pattern on RX_Frequency */
    SIGFOX_EP_ADDON_RFP_API_TEST_MODE_E = 4, /*!< Do uplink +  downlink frame with AES key but specific shorter timings */
#endif
    SIGFOX_EP_ADDON_RFP_API_TEST_MODE_A = 5, /*!< Do 9 uplink frames to measure frequency synthesis step */
    SIGFOX_EP_ADDON_RFP_API_TEST_MODE_B = 6, /*!< Call all Sigfox frames of all types and size on all the Sigfox Band  */
#ifdef SIGFOX_EP_SPECTRUM_ACCESS_LBT
    SIGFOX_EP_ADDON_RFP_API_TEST_MODE_G = 11, /*!< Call twice the Sigfox frames (payload 1 bit only) */
#endif
#ifdef SIGFOX_EP_PUBLIC_KEY_CAPABLE
    SIGFOX_EP_ADDON_RFP_API_TEST_MODE_K = 12, /*!< Execute the public key test - all the frames of the protocol needs to be sent */
#endif
    SIGFOX_EP_ADDON_RFP_API_TEST_MODE_L = 13, /*!< Execute the NVM test */
} SIGFOX_EP_ADDON_RFP_API_test_mode_reference_t;

/*!******************************************************************
 * \struct SIGFOX_EP_ADDON_RFP_API_test_mode_t
 * \brief RFP test mode parameters.
 *******************************************************************/
typedef struct {
    SIGFOX_EP_ADDON_RFP_API_test_mode_reference_t test_mode_reference;
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    SIGFOX_ul_bit_rate_t ul_bit_rate;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    sfx_s8 tx_power_dbm_eirp;
#endif
#ifdef SIGFOX_EP_BIDIRECTIONAL
    SIGFOX_EP_ADDON_RFP_API_downlink_cplt_cb_t downlink_cplt_cb;
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    SIGFOX_EP_ADDON_RFP_API_test_mode_cplt_cb_t test_mode_cplt_cb;
#endif
} SIGFOX_EP_ADDON_RFP_API_test_mode_t;

/*!******************************************************************
 * \union SIGFOX_EP_ADDON_RFP_API_progress_status_t
 * \brief RFP test mode progress status.
 *******************************************************************/
typedef struct {
    struct {
        sfx_u8 error :1;
    } status;
    sfx_u8 progress :7;
} SIGFOX_EP_ADDON_RFP_API_progress_status_t;

/*** SIGFOX EP API functions ***/

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_open(SIGFOX_EP_API_config_t *config)
 * \brief Open the EP library.
 * \param[in]   config: Pointer to the library configuration.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_open(SIGFOX_EP_ADDON_RFP_API_config_t *config);

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_close(void)
 * \brief Close the EP library.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_close(void);

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_process(void)
 * \brief Main process function of the library. This function should be called as soon as possible when the process callback is triggered.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_process(void);
#endif

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_test_mode()
 * \brief This function executes the test modes needed for the Sigfox RF and Protocol Tests.
 * \param[in]   config: .
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_test_mode(SIGFOX_EP_ADDON_RFP_API_test_mode_t *test_mode);

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_EP_ADDON_RFP_API_get_test_mode_progress_status(void)
 * \brief Get the current message status.
 * \param[in]   none
 * \param[out]  none
 * \retval      Current progress status.
 *******************************************************************/
SIGFOX_EP_ADDON_RFP_API_progress_status_t SIGFOX_EP_ADDON_RFP_API_get_test_mode_progress_status(void);

#ifdef SIGFOX_EP_VERBOSE
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_get_version(sfx_u8 **version, sfx_u8 *version_size_char)
 * \brief Get EP library version.
 * \param[out]  version: Version string.
 * \param[out]  version_size_char: Pointer to the string size.
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_RFP_API_status_t SIGFOX_EP_ADDON_RFP_API_get_version(sfx_u8 **version, sfx_u8 *version_size_char);
#endif

#else
#error "SIGFOX_EP_CERTIFICATION flag must be define for this RFP addon"
#endif
#endif /* __SIGFOX_EP_ADDON_RFP_API_H__ */
