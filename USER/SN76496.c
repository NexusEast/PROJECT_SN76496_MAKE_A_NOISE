#include "SN76496.h"
#include "stm32f10x.h"
#include "delay.h"


void SN76496_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;			    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;	
  GPIO_Init(GPIOC, &GPIO_InitStructure); 
	
  SN76496_SendData(0x9f);
  SN76496_SendData(0xbf);
  SN76496_SendData(0xdf);
  SN76496_SendData(0xff);
}
 

uint8_t READY_Read(void)
{
	return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8);
}
  
  
void WE_Write( BitAction act)
{ 
		GPIO_WriteBit(GPIOC,GPIO_Pin_9,act);
}


void SN76496_SendData(unsigned char data)
{ 
	WE_Write(Bit_SET); 
	GPIO_WriteBit(GPIOC,GPIO_Pin_0,  (data&1)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_1,  (data&2)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_2,  (data&4)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_3,  (data&8)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_4,  (data&16)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_5,  (data&32)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_6,  (data&64)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_7,  (data&128)?Bit_SET:Bit_RESET);
	WE_Write(Bit_RESET);   
	delay_ms(1);
	WE_Write(Bit_SET);  
}
