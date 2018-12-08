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

//Chip Operations
 typedef enum
{ 
    Tone_0 = 0, //00
    Tone_1,     //01
    Ton2_2,     //10
    Ton2_3,     //11
}ESN76496Channel;

 typedef enum
{ 
    White_Noise = 0,
    Periodic_noise
}ENoiseConfig;



/*
UpdateNoiseSource:
High <--                 --> Low
|bits|00|01|02|03|04|05|06 |07 |
|data|01|R0|R1|R2|--|FB|NF0|NF1|

Where 



*/

void SN76496_UpdateNoiseSource(unsigned char data);
void SN76496_UpdateAttenuator(ESN76496Channel channel,unsigned char data);


#endif
