/*
 * main.c
 *
 *  Created on: Nov 19, 2018
 *      Author: vass
 */


#include "matrix.h"
#include "periph.h"
#include "bitmaps.h"

int main(void)
{
	/* perifériák inicializáló függvényei */
	MySysInit();
	InitGPIO();
	InitSPI();
	InitTIM();
	InitDMA();
	InitUSART1();

	/* a képernyő buffer beállítja egy eltárolt képre a bitmaps.h ból */
	rowBufPtr = doggo_mogi;

	/* mátrix reset */
	SelectRow(0);
	ResetEN();

	/* start row scanning */
	ScanControl(ENABLE);

    while(1){}
}






























