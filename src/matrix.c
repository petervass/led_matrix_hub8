/*
 * matrix_driver.c
 *
 *  Created on: Nov 22, 2018
 *      Author: vass
 */

#include <matrix.h>

void DelayMs(uint32_t t){
	uint32_t startTick=SysTickCnt;

	while(SysTickCnt-startTick<=t){asm("nop;");}
}


void SelectRow(uint8_t row){
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
	PORT_LAT->BSRR = PIN_LAT;
	PORT_LAT->BRR = PIN_LAT;
}

void ScanControl(uint8_t s){
	assert_param(s==DISABLE | s==ENABLE);

	TIM_Cmd(TIM2, s);

}

__IO uint8_t rowBuf[H*(W>>3)];
__IO uint8_t * rowBufPtr = rowBuf;

void SendRow(uint16_t r){
	/* TODO: replace loop with DMA */
	for(uint16_t i=0;i<(W>>3);i++){
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

		SPI_I2S_SendData(SPI1, ~rowBufPtr[r*(W>>3)+i]);
	}

	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET); /* must be checked in addition to TXE, otherwise function will exit with transmission ongoing */

}


__IO uint8_t new_usart_data = 0;
__IO uint8_t *usartDataPtr;

/* USART1 RX DMA handler, uses circular buffer */
void DMA1_Channel5_IRQHandler(void){
	if(DMA_GetFlagStatus(DMA1_FLAG_TC5)!= RESET){
		DMA_ClearFlag(DMA1_FLAG_TC5);

		usartDataPtr = usartBuf + USART1_DMA_BUF_LEN/2;
		new_usart_data = 1;
	}

	if(DMA_GetFlagStatus(DMA1_FLAG_HT5)!= RESET){
		DMA_ClearFlag(DMA1_FLAG_HT5);

		usartDataPtr = usartBuf;
		new_usart_data = 1;
	}
}



__IO uint16_t rCnt=0;
__IO uint16_t interleave=0;

void TIM2_IRQHandler(void){ /* x Hz refresh rate */
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET){
        TIM_ClearITPendingBit(TIM2,  TIM_IT_Update);

        /* toggle PC13 as heartbeat */
        GPIOC->ODR ^= GPIO_Pin_13;

        SetEN();

        SendRow(rCnt*2+interleave);
        PulseLAT();
        SelectRow(rCnt*2 + interleave);

        ResetEN();


		rCnt+=1;
		if(rCnt == 8){
			rCnt=0;


			if(interleave == 1){
				interleave = 0;
				/* the scan is completed, use new data if available */
				if(new_usart_data){
					rowBufPtr = usartDataPtr;
					new_usart_data = 0;
				}
			}
			else
				interleave = 1;
		}
	}
}




