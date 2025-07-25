/*
 * can_tp.h
 *
 *  Created on: Jul 24, 2025
 *      Author: naveenkumar.m
 */

#ifndef INC_CAN_TP_H_
#define INC_CAN_TP_H_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* Return values */
#define CAN_TP_OK     1
#define CAN_TP_ERROR  0

/* Constants */
//#define CAN_TP_BLOCK_SIZE        0    // 0 = unlimited
#define CAN_TP_ST_MIN_MS         0    // Separation time (0 = immediate)
#define CAN_TP_TX_BUF_SIZE       256
#define CAN_TP_CF_FRAME_DATA     7
#define CAN_TP_MAX_PAYLOAD        4095
#define CAN_TP_BLOCK_SIZE         8
#define CAN_TP_RX_ID              0x7E0  // Tester -> ECU
#define CAN_TP_TX_ID              0x7E8  // ECU -> Tester

void can_tp_init(void);
void can_tp_receive_isr(uint32_t id, uint8_t *data, uint8_t len);
void can_tp_periodic_task(void);
void can_tp_send(uint32_t can_id, uint8_t* data, uint16_t len);
void can_tp_tx_task(void);

/* External CAN send */
extern void can_send(uint32_t id, uint8_t* data, uint8_t len);



#endif /* INC_CAN_TP_H_ */

