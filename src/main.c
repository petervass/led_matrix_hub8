#include "matrix.h"
#include "periph.h"
#include "bitmaps.h"

int main(void)
{

	MySysInit();
	InitGPIO();
	InitSPI();
	InitTIM();
	InitDMA();
	InitUSART1();


	rowBufPtr = doggo_mogi; /* initialize with static bitmap */
	SelectRow(0);
	ResetEN();

	/* start row scanning */
	ScanControl(ENABLE);

    while(1){}
}






























