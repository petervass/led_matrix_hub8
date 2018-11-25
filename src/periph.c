/*
 * periph_init.c
 *
 *  Created on: Nov 19, 2018
 *      Author: vass
 */
#include <periph.h>

void MySysInit(void){
	SystemInit(); /* órajel beállítások */
	SystemCoreClockUpdate();


	RCC_ClocksTypeDef   clkConfStruct;
	RCC_GetClocksFreq(&clkConfStruct);

	SysTick_Config(clkConfStruct.SYSCLK_Frequency/1000-1); /* 1ms/SysClk tick */

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); /* 4bit-es interrupt prioritások */
	NVIC_SetPriority(SysTick_IRQn, 0); /* systick irq legnagyobb prioritás */
}

/* SPI inicializálása */
void InitSPI(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	SPI_InitTypeDef  SPI_InitStructure;
	SPI_StructInit(&SPI_InitStructure);

	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; /* legnagyobb sebesség amit bír a matrix */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, ENABLE);
}


void InitTIM(void){
	/* TIM2 -- display scanning in update ISR */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef timInitStruct;
	TIM_TimeBaseStructInit(&timInitStruct); /* init time base to default */

	timInitStruct.TIM_Prescaler = 0;
	timInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	timInitStruct.TIM_RepetitionCounter = 0;
	timInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	timInitStruct.TIM_Period = 65535; /* 100us/row refresh rate */

	TIM_TimeBaseInit(TIM2, &timInitStruct);


	/* configure and enable update irq */
	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 6;
	nvicStructure.NVIC_IRQChannelSubPriority = 0;

	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);


	/* TIM4 -- for dimming by PWM on enable */
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
//
//	timInitStruct.TIM_Prescaler = 0;
//	timInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
//	timInitStruct.TIM_RepetitionCounter = 0;
//	timInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
//	timInitStruct.TIM_Period = 72; /* 72MHz/(PSC=9+1)*1ms=7200 == ARR*/
//	TIM_TimeBaseInit(TIM4, &timInitStruct);
//
//	/* set up output compare*/
//	TIM_OCStructInit(&OCInitStruct); /* init to default value*/
//
//	OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
//	OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; /* enable PA1 */
//	OCInitStruct.TIM_Pulse = 18;
//	OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
//
//	TIM_OC1Init(TIM4,&OCInitStruct);
}



void InitGPIO(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIOInitStruct;
	GPIO_StructInit(&GPIOInitStruct);

	/* SPI pins */
	GPIOInitStruct.GPIO_Pin = PIN_CLK | PIN_R1; /* PA5=SCK  PA7=MOSI */
	GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIOInitStruct);


	GPIOInitStruct.GPIO_Pin =PIN_LAT; /*LAT */
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(PORT_LAT, &GPIOInitStruct);



	GPIOInitStruct.GPIO_Pin =PIN_EN; /*EN and LAT */
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIOInitStruct);

	GPIOInitStruct.GPIO_Pin = PIN_LA  | PIN_LB | PIN_LC | PIN_LD ;
	GPIO_Init(GPIOA, &GPIOInitStruct);


	/* USART1 Tx */
	/*GPIOInitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIOInitStruct); */


	GPIOInitStruct.GPIO_Pin = GPIO_Pin_10; /* USART1 Rx */
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIOInitStruct);


	/* dbg */
	GPIOInitStruct.GPIO_Pin = GPIO_Pin_13; /*LAT */
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIOInitStruct);
}

void InitUSART1(void){
	USART_InitTypeDef USARTInitStruct;
	USART_StructInit(&USARTInitStruct);

	/* Enalbe clock for USART1, AFIO and GPIOA */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);


	USARTInitStruct.USART_BaudRate = 115200;
	USARTInitStruct.USART_WordLength = USART_WordLength_8b;
	USARTInitStruct.USART_StopBits = USART_StopBits_1;
	USARTInitStruct.USART_Parity = USART_Parity_No ;
	USARTInitStruct.USART_Mode = USART_Mode_Rx; /* | USART_Mode_Tx */
	USARTInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &USARTInitStruct);


	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE); /* Enable USART1 DMA Rx request */
	USART_Cmd(USART1, ENABLE); /* Enable USART1 */
}



__IO uint8_t usartBuf[USART1_DMA_BUF_LEN]={0}; /* circular buffer for usart */

void InitDMA(void){
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);


	DMA_InitTypeDef DMAInitStruct;
	DMA_StructInit(&DMAInitStruct);

	/* init USART1 Rx DMA */
	DMA_StructInit(&DMAInitStruct);

	DMAInitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMAInitStruct.DMA_MemoryBaseAddr = (uint32_t)usartBuf;
	DMAInitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMAInitStruct.DMA_BufferSize = USART1_DMA_BUF_LEN;
	DMAInitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMAInitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMAInitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMAInitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMAInitStruct.DMA_Mode = DMA_Mode_Circular; /* cirkuláris dupla buffer */
	DMAInitStruct.DMA_Priority = DMA_Priority_VeryHigh;
	DMAInitStruct.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel5, &DMAInitStruct);

	/* configure USART1 DMA IRQ */
	NVIC_InitTypeDef nvicStructure;

	nvicStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 10;
	nvicStructure.NVIC_IRQChannelSubPriority = 0;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&nvicStructure);

	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE); /* USART1 Rx transer complete */
	DMA_ITConfig(DMA1_Channel5, DMA_IT_HT, ENABLE); /* USART1 Rx half transfer complete*/
	DMA_Cmd(DMA1_Channel5, ENABLE);


}


