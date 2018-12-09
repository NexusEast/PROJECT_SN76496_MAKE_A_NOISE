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
	 
	 u8 blab = 0; 
	  
	 InitPWM();
	 SN76496_Init(); 
	   
	 
	
	 while(1)
	 { 		
	 
		 		SN76496_SendData(blab++);//10010000‬  
				mydelay(300000);
		
	 }
	  
 }
