#ifndef YM2612_H_
#define YM2612_H_ 
#include "delay.h"
#include "stdint.h"
#include "stm32f10x_gpio.h" 

#define LED0 PBout(5)// PB5
#define LED1 PEout(5)// PE5	

void YM2612_WriteDataPins(unsigned char data);
void YM2612_Init(void);
void YM2612_Reset(void);
void YM2612_SendDataPins(unsigned char addr, unsigned char data,BitAction setA1);
 
void YM2612_Send8BitData(const unsigned char data);
void YM2612_InitTestPiano(void); 

void YM2612_TestPiano(void);



#endif
