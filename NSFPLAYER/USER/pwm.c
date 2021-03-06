
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "pwm.h"
//#define DEBUGGG
 
TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_OCInitTypeDef TIM_OCInitStructure;

uint16_t CCR1_Val = 3;

uint16_t PrescalerValue = 0;

#define ONEKHZ (SystemCoreClock / 1000)

/*
 时钟频率为72MHZ，
 如果指定预分频器即TIM_Prescaler的值为72000（-1），
 那么经72000分频之后的工作频率就是1000，
 也就是所谓的1KHz。


 如果再指定计数值即TIM_Period为1000（-1）的话，恰好就是1秒了。
 
 T=SystemCoreClock/(TIM_Period+1)*(TIM_Prescaler+1);
 
  TIM4_TimeBaseStructure.TIM_Period = 1631;
  TIM4_TimeBaseStructure.TIM_Prescaler = 0;//(SystemCoreClock / 44100) - 1;//44.1KHz?
 */
 
 void InitNMIInterrupt()//20HZ@T2C1
 {
	 //3,600,000‬
	  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOA, &GPIO_InitStructure);
	 
 }
void InitTickTImer(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM4_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  
	
	 
	TIM4_TimeBaseStructure.TIM_Period = arr;//arr;  
	TIM4_TimeBaseStructure.TIM_Prescaler =psc;//psc; 
	TIM4_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  
	TIM4_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM4, &TIM4_TimeBaseStructure);  
 
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE );  

 
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);  


	TIM_Cmd(TIM4, ENABLE);   	  
}
void RCC_Configuration(void)
{
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* GPIOA and GPIOB clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOB, &GPIO_InitStructure);
}


void InitPWM(void)
{ 
  RCC_Configuration();

  GPIO_Configuration();
//2.142856983219934‬
  //PrescalerValue =  (uint16_t) (SystemCoreClock / 24000000) - 1;
  TIM_TimeBaseStructure.TIM_Period =  2;//(4-1);
  TIM_TimeBaseStructure.TIM_Prescaler = 0;//PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 2;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC3Init(TIM3, &TIM_OCInitStructure);

  TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}
 