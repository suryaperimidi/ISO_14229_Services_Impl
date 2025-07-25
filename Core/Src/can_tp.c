/*
 * can_tp.c
 *
 *  Created on: Jul 24, 2025
 *      Author: naveenkumar.m
 */

#include "can_tp.h"
#include "main.h"
#include <string.h>
#include <stdbool.h>
#include "uds_services_handler_disptcher.h"
#include "Data_Transmission_functional_unit.h"

/* Constants */
#define CAN_TP_RX_BUF_SIZE     256
#define CAN_TP_TX_BUF_SIZE     256
#define CAN_TP_MAX_CF_SEQ      0x0F

/* Internal receive buffer */
static uint8_t rx_buffer[CAN_TP_RX_BUF_SIZE];
static uint16_t rx_expected_len = 0;
static uint16_t rx_current_len = 0;
static uint8_t rx_next_sn = 1;  // Next expected sequence number
static bool rx_in_progress = false;

/* TX state variables */
static uint8_t tx_buffer[CAN_TP_TX_BUF_SIZE];
static uint16_t tx_total_len = 0;
static uint16_t tx_sent_len = 0;
static uint8_t tx_next_sn = 1;
static bool tx_in_progress = false;
static uint32_t tx_can_id = 0;
static bool tx_wait_fc = false;

/* CAN HAL function placeholder (you should implement this) */

void can_tp_handle_fc(uint8_t* data);

/* Flow Control Response (Server sends this when receiving FF from tester) */
static void can_tp_send_flow_control(uint32_t tx_id)
{
    uint8_t fc_frame[8] = { 0x30, 0x00, 0x00 }; // FC | CTS | BS=0 | STmin=0
    //can_send(tx_id, fc_frame, 8);
    send_can_message(fc_frame, 8);
}

/* This function must be called from your CAN RX ISR with received CAN frame */
void can_tp_receive_isr(uint32_t can_id, uint8_t* data, uint8_t len)
{
    uint8_t pci = data[0];
    uint8_t pci_type = pci >> 4;

    switch (pci_type)
    {
        /*case 0x0: // Single Frame
        {
            uint8_t sf_len = pci & 0x0F;
            if (sf_len > 7 || len < (sf_len + 1)) return;

            uds_process_request(&data[1], sf_len);
            break;
        }*/

        case 0x1: // First Frame
        {
            rx_expected_len = ((pci & 0x0F) << 8) | data[1];
            if (rx_expected_len > CAN_TP_RX_BUF_SIZE) return;

            uint8_t first_data_len = len - 2;
            memcpy(rx_buffer, &data[2], first_data_len);
            rx_current_len = first_data_len;
            rx_next_sn = 1;
            rx_in_progress = true;

            // Send Flow Control Frame (FC)
            can_tp_send_flow_control(can_id);  // Response to FF
            break;
        }

        case 0x2: // Consecutive Frame
        {
            if (!rx_in_progress) return;

            uint8_t seq_num = pci & 0x0F;
            if (seq_num != rx_next_sn) {
                rx_in_progress = false; // Sequence error
                return;
            }

            uint8_t copy_len = len - 1;
            if ((rx_current_len + copy_len) > rx_expected_len)
                copy_len = rx_expected_len - rx_current_len;

            memcpy(&rx_buffer[rx_current_len], &data[1], copy_len);
            rx_current_len += copy_len;
            rx_next_sn = (rx_next_sn + 1) & CAN_TP_MAX_CF_SEQ;

            if (rx_current_len >= rx_expected_len) {
            	uds_services_handler_disptcher(rx_buffer, rx_expected_len);
                rx_in_progress = false;
            }
            break;
        }

        case 0x3: // Flow Control frame received (from Tester)
        {
            // Usually handled in TX path – not required for RX
        	can_tp_handle_fc(data);
            break;
        }

        default:
            // Unknown or unsupported frame
            break;
    }
}

void can_tp_handle_fc(uint8_t* data)
{
    if (data[0] != 0x30) return;

    // FC received: allow CF transmission
    tx_wait_fc = false;
}

void can_tp_send(uint32_t can_id, uint8_t* data, uint16_t len)
{
    if (tx_in_progress) return;  // prevent overlapping

    tx_can_id = can_id;
    tx_total_len = len;
    tx_sent_len = 0;
    tx_next_sn = 1;
    tx_wait_fc = false;
    tx_in_progress = true;

    if (len <= 7) {
        // Send as Single Frame (SF)
        uint8_t sf_frame[8] = { 0 };
        sf_frame[0] = 0x00 | (len & 0x0F);  // SF | length
        memcpy(&sf_frame[1], data, len);
        //can_send(can_id, sf_frame, len + 1);
        tx_in_progress = false;
    } else {
        // Store to internal buffer
        memcpy(tx_buffer, data, len);

        // Send First Frame (FF)
        uint8_t ff_frame[8] = { 0 };
        ff_frame[0] = 0x10 | ((len >> 8) & 0x0F);  // FF | MSB len
        ff_frame[1] = len & 0xFF;                 // LSB len
        memcpy(&ff_frame[2], &tx_buffer[0], 6);
       // can_send(can_id, ff_frame, 8);
        send_can_message(ff_frame, 8);

        tx_sent_len = 6;  // First 6 bytes sent
        tx_wait_fc = true;  // wait for FC from tester
        	can_tp_tx_task();

    }
}


void can_tp_tx_task(void)
{

    if (!tx_in_progress || tx_wait_fc)
    	return;
    if (tx_sent_len >= tx_total_len) {
        tx_in_progress = false;
     return ;
    }

    uint8_t cf_frame[8] = { 0 };
    cf_frame[0] = 0x20 | (tx_next_sn & 0x0F);
    uint8_t remaining = tx_total_len - tx_sent_len;
    uint8_t copy_len = (remaining > CAN_TP_CF_FRAME_DATA) ? CAN_TP_CF_FRAME_DATA : remaining;

    memcpy(&cf_frame[1], &tx_buffer[tx_sent_len], copy_len);
    //can_send(tx_can_id, cf_frame, copy_len + 1);
    send_can_message(cf_frame, copy_len + 1);

    tx_sent_len += copy_len;
    tx_next_sn = (tx_next_sn + 1) & 0x0F;

}


