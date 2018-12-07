 
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "pwm.h" 
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
