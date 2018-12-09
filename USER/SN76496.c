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
		    
  //GPIO_InitStructure.GPIO_Mode = GPIO_Pin_8; 	  
  //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 	
 // GPIO_Init(GPIOC, &GPIO_InitStructure); 
	

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
	/*
	u8 _d = 0x90;//1001 0000
	_d |= (channel<<5);
	_d |= (data>>4);
	SN76496_SendData(_d);*/
	
	
	u8 _d = 0xF0;//1111 0000
	
}

void SN76496_SendData(unsigned char data)
{ 
	// PULL DOWN WE
	WE_Write(Bit_RESET); 
	//WRITE BUS and keep data 
	/*
	u16 bb = data;
	bb = bb << 8;*/
	GPIOC->ODR = 0XFFFF; 
	/*
	data = ~data;
    GPIOC->BSRR = (1U << 8) << (16 * ((data >> 0)&1));
    GPIOC->BSRR = (1U << 9) << (16 * ((data >> 1)&1));
    GPIOC->BSRR = (1U << 13) << (16 * ((data >> 2)&1));
    GPIOC->BSRR = (1U << 14) << (16 * ((data >> 3)&1));
    GPIOC->BSRR = (1U << 15) << (16 * ((data >> 4)&1));
    GPIOC->BSRR = (1U << 0) << (16 * ((data >> 5)&1));
    GPIOC->BSRR = (1U << 1) << (16 * ((data >> 6)&1));
    GPIOC->BSRR = (1U << 2) << (16 * ((data >> 7)&1));*/
	
	/*
	GPIO_WriteBit(GPIOC,GPIO_Pin_0,  (data&1)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_1,  (data&2)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_2,  (data&4)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_3,  (data&8)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_4,  (data&16)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_5,  (data&32)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_6,  (data&64)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(GPIOC,GPIO_Pin_7,  (data&128)?Bit_SET:Bit_RESET);*/
	//mydelay2(40000);
	//wait for one PSG writecycle
	//while(READY_Read()!=0){}//wait for ready change to low.  
	//while(READY_Read()){}//wait for ready. 
		
	//PULL UP WE
	WE_Write(Bit_SET);    
}
