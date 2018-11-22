/*
 * periph_init.h
 *
 *  Created on: Nov 22, 2018
 *      Author: vass
 */

#ifndef PERIPH_H_
#define PERIPH_H_

#include "stm32f10x_rcc.h" /* Reset & Clock Control */
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "misc.h" /* NVIC configuration */

#include "stm32f1xx_it.h" /* systick isr */

#define PIN_LA 	GPIO_Pin_0
#define PIN_LB 	GPIO_Pin_1
#define PIN_LC 	GPIO_Pin_2
#define PIN_LD 	GPIO_Pin_3
#define PORT_LX GPIOA


#define PIN_R1 	GPIO_Pin_7
#define PORT_R1 GPIOA /* used as MOSI */

#define PIN_CLK GPIO_Pin_5
#define PORT_CLK GPIOA /* used as SCK */

#define PIN_LAT GPIO_Pin_0
#define PORT_LAT GPIOB

#define PIN_EN 	GPIO_Pin_1
#define PORT_EN GPIOB

#define PIN_EN_PWM GPIO_Pin_6
#define PORT_EN_PWM GPIOB

#define H 16
#define W 64

#define USART1_DMA_BUF_LEN (H*W/8*2)

extern __IO uint8_t usartBuf[];

void MySysInit(void);
void InitSPI(void);
void InitTIM(void);
void InitGPIO(void);
void InitDMA(void);
void InitUSART1(void);

#endif /* PERIPH_H_ */
