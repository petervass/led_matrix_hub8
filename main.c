#include "stm32f10x.h"
#include "stm32f10x_rcc.h" /* Reset & Clock Control */
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "misc.h" /* NVIC configuration */

#define PIN_LA 	GPIO_Pin_0
#define PIN_LB 	GPIO_Pin_1
#define PIN_LC 	GPIO_Pin_2
#define PIN_LD 	GPIO_Pin_3
#define PORT_LX GPIOA


#define PIN_R1 	GPIO_Pin_5
#define PORT_R1 GPIOA /* used as MOSI */

#define PIN_CLK GPIO_Pin_7
#define PORT_CLK GPIOA /* used as SCK */

#define PIN_LAT GPIO_Pin_0
#define PORT_LAT GPIOB

#define PIN_EN 	GPIO_Pin_1
#define PORT_EN GPIOB

#define PIN_EN_PWM GPIO_Pin_6
#define PORT_EN_PWM GPIOB

#define H 16
#define W 64


/* increment SysTick counter, useful for delay functions */
/* with systick f=1kHz, it overflows every 49 days */
__IO uint32_t SysTickCnt=0;
void SysTick_Handler(void){
	SysTickCnt++;
}


void DelayMs(uint32_t t){
	uint32_t startTick=SysTickCnt;

	while(SysTickCnt-startTick<=t){}
}





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
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; /* TODO: try maximum value */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &SPI_InitStructure);
}

#define T_PULSE (72-1) /* 72MHz/(PSC=0+1)*1us=72 == ARR*/
void InitTIM(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);


	/* TIM3 */
	/* Tuev= TIM_TCLK/((PSC+1)*(ARR+1)*(RCR+1)) */
	TIM_TimeBaseInitTypeDef timInitStruct;
	TIM_TimeBaseStructInit(&timInitStruct); /* init time base to default */

	timInitStruct.TIM_Prescaler = 0;
	timInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	timInitStruct.TIM_RepetitionCounter = 0;
	timInitStruct.TIM_CounterMode = TIM_CounterMode_Down;
	timInitStruct.TIM_Period = T_PULSE;

	TIM_TimeBaseInit(TIM3, &timInitStruct);

	/* set up output compare*/
	TIM_OCInitTypeDef OCInitStruct;
	TIM_OCStructInit(&OCInitStruct); /* init to default value*/

	OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; /* enable PA1 */
	OCInitStruct.TIM_Pulse = T_PULSE-1 ;
	OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC3Init(TIM3, &OCInitStruct);

	TIM_SelectOnePulseMode(TIM3,TIM_OPMode_Single);



	/* TIM2 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	timInitStruct.TIM_Prescaler = 0;
	timInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	timInitStruct.TIM_RepetitionCounter = 0;
	timInitStruct.TIM_CounterMode = TIM_CounterMode_Down;
	timInitStruct.TIM_Period = 72*50; /* 100us/row refresh rate */

	TIM_TimeBaseInit(TIM2, &timInitStruct);


	/* configure and enable update irq */
	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = TIM2_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 1;
	nvicStructure.NVIC_IRQChannelSubPriority = 0;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);


	/* TIM4 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	timInitStruct.TIM_Prescaler = 0;
	timInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	timInitStruct.TIM_RepetitionCounter = 0;
	timInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	timInitStruct.TIM_Period = 72; /* 72MHz/(PSC=9+1)*1ms=7200 == ARR*/
	TIM_TimeBaseInit(TIM4, &timInitStruct);

	/* set up output compare*/
	TIM_OCStructInit(&OCInitStruct); /* init to default value*/

	OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; /* enable PA1 */
	OCInitStruct.TIM_Pulse = 18;
	OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM4,&OCInitStruct);



}



void InitGPIO(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIOInitStruct;
	GPIO_StructInit(&GPIOInitStruct);

	/* SPI pins */
	GPIOInitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7; /* PA5=SCK  PA7=MOSI */
	GPIOInitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOA, &GPIOInitStruct);

	GPIOInitStruct.GPIO_Pin = GPIO_Pin_6; /* PA6=MISO */
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIOInitStruct);
	/*           */

	GPIOInitStruct.GPIO_Pin =PIN_LAT | PIN_EN_PWM; /*LAT */
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIOInitStruct);


	GPIOInitStruct.GPIO_Pin =PIN_EN; /*EN and LAT */
	GPIOInitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIOInitStruct);

	GPIOInitStruct.GPIO_Pin = 0b00001111; /* half GPIOA */
	GPIO_Init(GPIOA, &GPIOInitStruct);
}



void SelectRow(uint32_t row){
	uint32_t odr=GPIOA->ODR;

	GPIOA->ODR=(odr & 0xfffffff0) | (row & 0x0f);
}

void SetEN(void){
	PORT_EN->BSRR = PIN_EN;
}

void ResetEN(void){
	PORT_EN->BRR = PIN_EN;
}


void PulseLAT(void){
	TIM_Cmd(TIM3, ENABLE);
	while(TIM3->CR1&TIM_CR1_CEN){}
}



uint8_t rowBuf[H*(W>>3)];

void SendRow(uint16_t r){
	SPI_Cmd(SPI1, ENABLE);

	for(uint16_t i=0;i<(W>>3);i++){
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

		SPI_I2S_SendData(SPI1,~rowBuf[r*(W>>3)+i]);
	}

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET); /* must be checked in addition to TXE, otherwise function will exit with transmission ongoing */

	/* SPI_Cmd(SPI1, DISABLE); */
}


__IO uint8_t rCnt=0;
uint8_t duty=128; /* every x cycles */

void TIM2_IRQHandler(void){ /* x Hz refresh rate */
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET){
        TIM_ClearITPendingBit(TIM2,  TIM_IT_Update);
		SendRow(rCnt);

		//TIM_CCxCmd(TIM4,TIM_Channel_1,TIM_CCx_Enable);
		//TIM_ForcedOC1Config(TIM4,TIM_ForcedAction_Active);
		SetEN();
		SelectRow(rCnt);

		PulseLAT();
		ResetEN();

		//TIM_CCxCmd(TIM4,TIM_Channel_1,TIM_CCx_Enable);




		rCnt+=1;

		if(rCnt>=16){
			rCnt=0;
		}
	}
}




int main(void)
{
	SystemInit();


	RCC_ClocksTypeDef   clkConfStruct;
	RCC_GetClocksFreq(&clkConfStruct);

	SysTick_Config(clkConfStruct.SYSCLK_Frequency/1000-1); /* 1ms/SysClk tick */

	InitGPIO();
	InitSPI();
	InitTIM();

	/* set up display buffer */
	for(uint16_t i=0;i<H*(W>>3);i++){
			rowBuf[i]=0b10101010;
	}

	SelectRow(0);
	SetEN();

	TIM_Cmd(TIM2, ENABLE);
	TIM_Cmd(TIM4, ENABLE);

    while(1){}
}



























