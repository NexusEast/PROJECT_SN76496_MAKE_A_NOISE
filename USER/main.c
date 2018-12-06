#include "stm32f10x.h"
#include "pwm.h"
#include "SN76496.h"
#include "delay.h"
 
 int main(void)
 {	 
	 InitPWM();
	 SN76496_Init();
	 delay_init();
	  
	 // sets channel 0 tone to 0x123
	 SN76496_SendData(0x83);
	 SN76496_SendData(0x12);
	 
	 while(1)
	 { 
			SN76496_SendData(0x90); // sets channel 0 to loudest possible
			delay_ms(100);   
			SN76496_SendData(0x9F);  // sets channel 0 to lowest volume (silencing it)
			delay_ms(100); 
	 }
	  
 }
