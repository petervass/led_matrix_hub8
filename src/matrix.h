/*
 * matrix_driver.h
 *
 *  Created on: Nov 20, 2018
 *      Author: vass
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include "periph.h"

/* képernyő buffer */
extern __IO uint8_t * rowBufPtr;
extern __IO uint8_t rowBuf[];


void SelectRow(uint8_t row);
void SetEN(void);
void ResetEN(void);
void PulseLAT(void);
void DelayMs(uint32_t t);
void ScanControl(uint8_t s);

#endif /* MATRIX_H_ */
