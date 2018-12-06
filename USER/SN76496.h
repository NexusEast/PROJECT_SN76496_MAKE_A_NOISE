#ifndef SN76496
#define SN76496

#include "stdint.h"
#include "stm32f10x_gpio.h" 


uint8_t 	READY_Read(void); 

uint8_t 	CE_Read(void);  
void 				CE_Write( BitAction act);
 
uint8_t 	WE_Read(void);  
void 				WE_Write( BitAction act);

void SN76496_Init(void);
void SN76496_SendData(unsigned char data);


#endif
