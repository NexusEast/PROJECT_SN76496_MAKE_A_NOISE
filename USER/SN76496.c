#include "SN76496.h"
#include "stm32f10x.h"
#include "delay.h" 
/*

//FOR 76498 DATA PIN 0-
PD6     FIFO_WRST      
PG6     NRF_IRQ        
PG7     NRF_CS         
PG8     NRF_CE         
PG11    1WIRE_DQ       
PG13    OV_SDA         
PG14    FIFO_RRST      
PG15    FIFO_OE        
//CONTROL PIN
PF6     VS_XDCS        
PF7     VS_XCS         

*/

//FOR 76498 DATA PIN 0-7
#define SN76496_D0 GPIOD,GPIO_Pin_6 //PD6     FIFO_WRST      
#define SN76496_D1 GPIOG,GPIO_Pin_6 //PG6     NRF_IRQ        
#define SN76496_D2 GPIOG,GPIO_Pin_7 //PG7     NRF_CS         
#define SN76496_D3 GPIOG,GPIO_Pin_8 //PG8     NRF_CE         
#define SN76496_D4 GPIOG,GPIO_Pin_11 //PG11    1WIRE_DQ       
#define SN76496_D5 GPIOG,GPIO_Pin_13 //PG13    OV_SDA         
#define SN76496_D6 GPIOG,GPIO_Pin_14 //PG14    FIFO_RRST      
#define SN76496_D7 GPIOG,GPIO_Pin_15 //PG15    FIFO_OE  
//control pin
#define SN76496_WE GPIOF,GPIO_Pin_6 //PF6
#define SN76496_RDY GPIOF,GPIO_Pin_7 //PF7

  void mydelay22(int d)
 {
	 int i = 0;
	 for(i = 0; i < d;i++){}
 }
 void mydelay2(int d)
 {
	 int i = 0;
	 for(i = 0 ; i < d; i ++){}
 }
void SN76496_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure_D;
	GPIO_InitTypeDef  GPIO_InitStructure_G;
	GPIO_InitTypeDef  GPIO_InitStructure_F;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
  GPIO_InitStructure_D.GPIO_Pin = GPIO_Pin_6;			    
  GPIO_InitStructure_D.GPIO_Mode = GPIO_Mode_Out_PP; 	
  GPIO_InitStructure_D.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOD, &GPIO_InitStructure_D); 
	
	
  GPIO_InitStructure_G.GPIO_Pin = GPIO_Pin_6  |GPIO_Pin_7  |GPIO_Pin_8  |GPIO_Pin_11 |GPIO_Pin_13 |GPIO_Pin_14 |GPIO_Pin_15;			    
  GPIO_InitStructure_G.GPIO_Mode = GPIO_Mode_Out_PP; 	
  GPIO_InitStructure_G.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOG, &GPIO_InitStructure_G); 
	
	
  GPIO_InitStructure_F.GPIO_Pin = GPIO_Pin_6;			    
  GPIO_InitStructure_F.GPIO_Mode = GPIO_Mode_Out_PP; 	
  GPIO_InitStructure_F.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOF, &GPIO_InitStructure_F); 
	
	
	WE_Write(Bit_SET);
		    
  //GPIO_InitStructure8.GPIO_Mode = GPIO_Pin_8; 	  
  //GPIO_InitStructure8.GPIO_Mode = GPIO_Mode_IPU; 	
  //GPIO_Init(GPIOC, &GPIO_InitStructure8); 
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
	GPIO_WriteBit(SN76496_WE,act); 
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
u8 psgFrqLowByte = 0;
void SN76496_SendData(unsigned char data)
{ 
  //PSG noise channel fix
  //A bunny fixed the hi-hat for ya'll.
  if((data & 0x80) == 0)
  {
    if((psgFrqLowByte & 0x0F) == 0)
    {
      if((data & 0x3F) == 0)
        psgFrqLowByte |= 1;
    }
    SN76496_Write(psgFrqLowByte);
    SN76496_Write(data);
  }
  else if((data & 0x90) == 0x80 && (data & 0x60)>>5 != 3)
    psgFrqLowByte = data;
  else
    SN76496_Write(data);
}

void SN76496_Write(unsigned char data)
{ 
 
// 	// PULL DOWN WE
  	WE_Write(Bit_SET); 

	
	GPIO_WriteBit(SN76496_D0,  (data&128)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(SN76496_D1,  (data&64)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(SN76496_D2,  (data&32)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(SN76496_D3,  (data&16)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(SN76496_D4,  (data&8)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(SN76496_D5,  (data&4)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(SN76496_D6,  (data&2)?Bit_SET:Bit_RESET);
	GPIO_WriteBit(SN76496_D7,  (data&1)?Bit_SET:Bit_RESET);
	
	WE_Write(Bit_RESET); 
	//delay_us(1);
	mydelay2(1150);
	//delay_us(14);
  //while(READY_Read()){}
	//while(READY_Read()!=0){}
	//mydelay2(10);
 
		
	WE_Write(Bit_SET);   
	//delay_ms(14);
	//mydelay2(100);
	
// 	//PULL UP WE 
}
