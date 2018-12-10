#include "SN76496.h"
#include "stm32f10x.h"
#include "delay.h" 
 
 void mydelay2(int d)
 {
	 int i = 0;
	 for(i = 0 ; i < d; i ++){}
 }
void SN76496_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;			    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOC, &GPIO_InitStructure); 
	WE_Write(Bit_SET);
		    
  GPIO_InitStructure.GPIO_Mode = GPIO_Pin_8; 	  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	
  GPIO_Init(GPIOC, &GPIO_InitStructure); 
	//mute all channels
	
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

/*
UPDATE ATTENUATOR  (SINGLE BYTE TRANSFER)

B7											B0
| |CHANNEL| | ATTENUATOR|
|1|CH0|CH1|1|A3|A2|A1|A0|
*/

void SN76496_UpdateAttenuator(ESN76496Channel channel,unsigned char data)
{ 
	u8 _d = 0x90;//1001 0000
	_d |= (channel& 0xf0);
	_d |= (data & 0x0f);
	SN76496_SendData(_d); 
	
	 
	
}

void SN76496_SendData(unsigned char data)
{ 
	// PULL DOWN WE
	WE_Write(Bit_SET); 

	
	GPIO_WriteBit(GPIOC,GPIO_Pin_0,  (data&128)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_1,  (data&64)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_2,  (data&32)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_3,  (data&16)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_4,  (data&8)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_5,  (data&4)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_6,  (data&2)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_7,  (data&1)?Bit_SET:Bit_RESET);
	
	WE_Write(Bit_RESET); 
//delay_us(14);
  while(READY_Read()){}
	while(READY_Read()!=0){}
	//mydelay2(10);
 
		
	WE_Write(Bit_SET);   
	//delay_ms(14);
	//mydelay2(100);
	
	//PULL UP WE 
}
