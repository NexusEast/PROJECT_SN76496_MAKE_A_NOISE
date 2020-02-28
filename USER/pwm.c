
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "pwm.h"
//#define DEBUGGG

#ifdef DEBUGGG


TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure; 

uint16_t CCR1_Val = 3; 

uint16_t PrescalerValue = 0;


void RCC_Configuration(void)
{
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* GPIOA and GPIOB clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
}

 
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOA, &GPIO_InitStructure);
 
}
 

void InitPWM(void)
{
 
  RCC_Configuration();
 
  GPIO_Configuration();
 
  PrescalerValue = (uint16_t) (SystemCoreClock / 24000000) - 1; 
	TIM_TimeBaseStructure.TIM_Period = (6-1); 
  TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM3, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable); 

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
 
}

#else
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
void InitTickTImer(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM4_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //ÉèÖÃNVICÖÐ¶Ï·Ö×é2:2Î»ÇÀÕ¼ÓÅÏÈ¼¶£¬2Î»ÏìÓ¦ÓÅÏÈ¼¶
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //Ê±ÖÓÊ¹ÄÜ
	
	//¶¨Ê±Æ÷TIM3³õÊ¼»¯
	TIM4_TimeBaseStructure.TIM_Period = arr;//arr; //ÉèÖÃÔÚÏÂÒ»¸ö¸üÐÂÊÂ¼þ×°Èë»î¶¯µÄ×Ô¶¯ÖØ×°ÔØ¼Ä´æÆ÷ÖÜÆÚµÄÖµ	
	TIM4_TimeBaseStructure.TIM_Prescaler =psc;//psc; //ÉèÖÃÓÃÀ´×÷ÎªTIMxÊ±ÖÓÆµÂÊ³ýÊýµÄÔ¤·ÖÆµÖµ
	TIM4_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //ÉèÖÃÊ±ÖÓ·Ö¸î:TDTS = Tck_tim
	TIM4_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIMÏòÉÏ¼ÆÊýÄ£Ê½
	TIM_TimeBaseInit(TIM4, &TIM4_TimeBaseStructure); //¸ù¾ÝÖ¸¶¨µÄ²ÎÊý³õÊ¼»¯TIMxµÄÊ±¼ä»ùÊýµ¥Î»
 
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE ); //Ê¹ÄÜÖ¸¶¨µÄTIM3ÖÐ¶Ï,ÔÊÐí¸üÐÂÖÐ¶Ï

	//ÖÐ¶ÏÓÅÏÈ¼¶NVICÉèÖÃ
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM3ÖÐ¶Ï
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //ÏÈÕ¼ÓÅÏÈ¼¶0¼¶
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //´ÓÓÅÏÈ¼¶3¼¶
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQÍ¨µÀ±»Ê¹ÄÜ
	NVIC_Init(&NVIC_InitStructure);  //³õÊ¼»¯NVIC¼Ä´æÆ÷


	TIM_Cmd(TIM4, ENABLE);  //Ê¹ÄÜTIMx		  
}
void RCC_Configuration(void)
{
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* GPIOA and GPIOB clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIOA, &GPIO_InitStructure);
}


void InitPWM(void)
{ 
  RCC_Configuration();

  GPIO_Configuration();
//2.142856983219934‬
  //PrescalerValue =  (uint16_t) (SystemCoreClock / 24000000) - 1;
  TIM_TimeBaseStructure.TIM_Period =  8;//(4-1);
  TIM_TimeBaseStructure.TIM_Prescaler = 0;//PrescalerValue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* PWM1 Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = 3;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM3, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}

	#endif