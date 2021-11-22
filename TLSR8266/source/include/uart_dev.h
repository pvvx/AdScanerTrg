/*
 * uart_dev.h
 *
 *  Created on: 23.02.2020
 *      Author: pvvx
 */

#ifndef UART_DEV_H_
#define UART_DEV_H_

#define USE_UART_RX_IRQ		0
#define USE_UART_TX_IRQ		0

#ifndef UART_TX_RX_DMA_BUFFER_SIZE
#define UART_TX_RX_DMA_BUFFER_SIZE (64+4) // max 1024
#endif

typedef struct  __attribute__((packed)) _dev_uart_cfg_t{
	uint16_t baud;   // 3..53333 x100 -> 300 baud to 4 Mbaud (16M/4,5,6,7,8..262144)
	uint8_t  stop;   // =0 ->1, =1 -> 1.5, =2 => 2
	uint8_t  parity; // =0 -> none, =1 -> even, =2 -> odd
} dev_uart_cfg_t;

extern unsigned char uart_rx_buff[UART_TX_RX_DMA_BUFFER_SIZE];
extern unsigned char uart_tx_buff[UART_TX_RX_DMA_BUFFER_SIZE];

//extern dev_uart_cfg_t cfg_uart;
//extern const dev_uart_cfg_t def_cfg_uart;

extern unsigned char uart_enabled;
extern volatile unsigned char uart_rx_irq, uart_tx_irq;

#define UartTxBusy() (reg_dma_tx_rdy0 & FLD_DMA_UART_TX)
#define UartRxReady() (reg_dma_irq_src & FLD_DMA_UART_RX)

void uart_init(dev_uart_cfg_t * p);
void uart_deinit(void);
unsigned char uart_send(unsigned char* addr);


#endif /* UART_DEV_H_ */
