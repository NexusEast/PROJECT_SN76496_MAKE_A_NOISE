#include "stm32f10x.h"
#include "pwm.h"
#include "SN76496.h"
#include "delay.h"
 
 void mydelay(int d)
 {
	 int i = 0;
	 for(i = 0; i < d;i++){}
 }
 int main(void)
 {	  
	  
	 // sets channel 0 tone to 0x123
	 
	 	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;			    
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;	
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	 
	 
	 InitPWM();
	 SN76496_Init();
	 delay_init();
	 
	 
	// WE_Write(Bit_RESET);
	 //SN76496_SendData(0x83);//‭10000011‬
	 //SN76496_SendData(0x12);//‭00010010‬
	 
	 
	 while(1)
	 { 		
		 // sets channel 0 to loudest possible 
			//SN76496_SendData(0x90);//10010000‬  
			//SN76496_SendData(0x9F);//10010000‬  
				//b=!b;
		 
			/*
			// sets channel 0 to lowest volume (silencing it)
			SN76496_SendData(0x9F);   //‭10011111‬
			delay_ms(100); */
	 }
	  
 }
