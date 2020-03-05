//#include "sys.h" 
 #include "stm32f10x.h"
 #include "display.h"
 #include "pwm.h"
 #include <string.h>
 
// Delay function

/*

        .--\/--.
 AD1 <- |01  40| -- +5V
 AD2 <- |02  39| -> OUT0
/RST -> |03  38| -> OUT1
 A00 <- |04  37| -> OUT2
 A01 <- |05  36| -> /OE1
 A02 <- |06  35| -> /OE2
 A03 <- |07  34| -> R/W
 A04 <- |08  33| <- /NMI
 A05 <- |09  32| <- /IRQ
 A06 <- |10  31| -> M2
 A07 <- |11  30| <- TST (usually GND)
 A08 <- |12  29| <- CLK
 A09 <- |13  28| <> D0
 A10 <- |14  27| <> D1
 A11 <- |15  26| <> D2
 A12 <- |16  25| <> D3
 A13 <- |17  24| <> D4
 A14 <- |18  23| <> D5
 A15 <- |19  22| <> D6
 GND -- |20  21| <> D7
        `------'
				
CLK : 21.47727 MHz (NTSC) or 26.6017 MHz (PAL) clock input. Internally, this clock is divided by 12 (NTSC 2A03) or 16 (PAL 2A07) to feed the 6502's clock input f0, which is in turn inverted to form f1, which is then inverted to form f2. f1 is high during the first phase (half-cycle) of each CPU cycle, while f2 is high during the second phase.
AD1 : Audio out pin (both pulse waves).
AD2 : Audio out pin (triangle, noise, and DPCM).
Axx and Dx : Address and data bus, respectively. Axx holds the target address during the entire read/write cycle. For reads, the value is read from Dx during f2. For writes, the value appears on Dx during f2 (and no sooner).
OUT0..OUT2 : Output pins used by the controllers ($4016 output latch bits 0-2). These 3 pins are connected to either the NES or Famicom's expansion port, and OUT0 is additionally used as the "strobe" signal (OUT) on both controller ports.
/OE1 and /OE2 : Controller ports (for controller #1 and #2 respectively). Each enable the output of their respective controller, if present.
R/W : Read/write signal, which is used to indicate operations of the same names. Low is write. R/W stays high/low during the entire read/write cycle.
/NMI : Non-maskable interrupt pin. See the 6502 manual and CPU interrupts for more details.
/IRQ : Interrupt pin. See the 6502 manual and CPU interrupts for more details.
M2 : Can be considered as a "signals ready" pin. It is a modified version the 6502's f2 (which roughly corresponds to the CPU input clock f0) that allows for slower ROMs. CPU cycles begin at the point where M2 goes low.
In the NTSC 2A03, M2 has a duty cycle of 5/8th, or 350ns/559ns. Equivalently, a CPU read (which happens during the second, high phase of M2) takes 1 and 7/8th PPU cycles. The internal f2 duty cycle is exactly 1/2 (one half).
In the PAL 2A07, the duty cycle is not known, but suspected to be 19/32.
TST : (tentative name) Pin 30 is special: normally it is grounded in the NES, Famicom, PC10/VS. NES and other Nintendo Arcade Boards (Punch-Out!! and Donkey Kong 3). But if it is pulled high on the RP2A03G, extra diagnostic registers to test the sound hardware are enabled from $4018 through $401A, and the joystick ports $4016 and $4017 become open bus. On older revisions of the CPU, pulling pin 30 high instead causes the CPU to stop execution.
/RST : When low, holds CPU in reset state, during which all CPU pins (except pin 2) are in high impedance state. When released, CPU starts executing code (read $FFFC, read $FFFD, ...) after 6 M2 clocks.





2A03 PIN SETUP
A0-7 -> PA0-7
A8-15 PB8-15

D0:		PA9
D1:		PB1
D2:		PC15
D3:   PB3
D4:		PB4
D5:		PB5
D6:		PA10
D7:		PA11

CLK:	PB0 T3C3
R/W:	PC13 //low is write, light up. high is read. no light.
/NMI:	PA15
/IRQ:	PA12
M2	:	PC14
*/


#define RAMSIZE 0x0800
#define APU_REG_START 0x4000
#define APU_REG_SIZE 0X2000
#define ROM_START_ADDR 0x8000 
#define ROM_SIZE 0x1000 
u8 CPURAM[RAMSIZE] = {0};
u8 CPUROM[ROM_SIZE] = {0};
u8 APUREG[APU_REG_SIZE]={0};

#define FULLSPEED_MODE

unsigned char PROGRAM[42] = {
	0xA9, 0xE0, 0x8D, 0x00, 0x40, 0xA9, 0xFF, 0x8D, 0x01, 0x40, 0xA9, 0x80, 0x8D, 0x02, 0x40, 0xA9, 
	0x80, 0x8D, 0x03, 0x40, 0xA9, 0x07, 0x8D, 0x15, 0x40, 0xA2, 0x00, 0xA0, 0x00, 0xE8, 0xC8, 0xC0, 
	0xFF, 0xD0, 0xFB, 0xE0, 0xFF, 0xD0, 0xF6, 0x4C, 0x00, 0x80
};

void InitProgram()
{
	int size = sizeof(PROGRAM);
	u8 i = 0;
	for(i = 0;i<size;i++)
	{
		CPUROM[i] = PROGRAM[i];
	}
}

BitAction CLK;
BitAction NMICLK;
typedef enum
{ WR_WRITE = 0,
  WR_READ
}WR_ACTION;


#define OLED_MAX_CHAR_PER_LINE 20
char oledBuffer_0[OLED_MAX_CHAR_PER_LINE]={'\0'};
char oledBuffer_1[OLED_MAX_CHAR_PER_LINE]={'\0'};
u8 oledCurLine = 0;

u16 CPU2A03_READ_ADDR()
{
	u16 ret = 0; 
	ret |= GPIO_ReadInputData(GPIOB) & 0xff00;
	ret |= GPIO_ReadInputData(GPIOA)  & 0xff;
	return ret;
}
void InitManualClock()
{
	
		GPIO_InitTypeDef GPIO_InitStruct; 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  
		GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_0 ;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void SETUP_2A03_DATALINE(WR_ACTION WR)
{/*
	
D0:		PA9
D1:		PB1
D2:		PC15
D3:   PB3
D4:		PB4
D5:		PB5
D6:		PA10
D7:		PA11
*/
	GPIO_InitTypeDef GPIO_InitStruct; 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA, ENABLE); 
	
  GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_9 | GPIO_Pin_10 |GPIO_Pin_11;     
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	
	if(WR == WR_READ)//READ FROM CPU
	{
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
	}
	else//WRITE TO CPU
	{ 
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	} 
    GPIO_Init(GPIOA, &GPIO_InitStruct); 
  GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_4 |GPIO_Pin_5;  
    GPIO_Init(GPIOB, &GPIO_InitStruct); 
  GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_15;  
    GPIO_Init(GPIOC, &GPIO_InitStruct);
	//
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST,ENABLE); 
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE); 	
}
u8 IsRead()
{
	return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);
}
u8 M2Status()
{
	return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14);
}
u8 ReadFromCPU()
{
	
/* 
D0:		PA9
D1:		PB1
D2:		PC15
D3:   PB3
D4:		PB4
D5:		PB5
D6:		PA10
D7:		PA11
*/
	
	u8 ret = 0;
	SETUP_2A03_DATALINE(WR_READ);
	/*
ret |= 0x01 & GPIO_ReadInputDataBit(GPIOA,  GPIO_Pin_9);//D0:		PA9
ret |= 0x02 & GPIO_ReadInputDataBit(GPIOB,  GPIO_Pin_1);//D1:		PB1
ret |= 0x04 & GPIO_ReadInputDataBit(GPIOC,  GPIO_Pin_15); //D2:		PC15
ret |= 0x08 & GPIO_ReadInputDataBit(GPIOB,  GPIO_Pin_3);//D3:   	PB3
ret |= 0x10 & GPIO_ReadInputDataBit(GPIOB,  GPIO_Pin_4);//D4:		PB4
ret |= 0x12 & GPIO_ReadInputDataBit(GPIOB,  GPIO_Pin_5);//D5:		PB5
ret |= 0x14 & GPIO_ReadInputDataBit(GPIOA,  GPIO_Pin_10); //D6:		PA10
ret |= 0x18 & GPIO_ReadInputDataBit(GPIOA,  GPIO_Pin_15); //D7:		PA11
*/

ret |= GPIO_ReadInputDataBit(GPIOA,  GPIO_Pin_9);	//D0:		PA9
ret |= GPIO_ReadInputDataBit(GPIOB,  GPIO_Pin_1)	<< 1;	//D1:		PB1
ret |= GPIO_ReadInputDataBit(GPIOC,  GPIO_Pin_15)	<< 2; 	//D2:		PC15
ret |= GPIO_ReadInputDataBit(GPIOB,  GPIO_Pin_3)	<< 3;	//D3:   	PB3
ret |= GPIO_ReadInputDataBit(GPIOB,  GPIO_Pin_4)	<< 4;	//D4:		PB4
ret |= GPIO_ReadInputDataBit(GPIOB,  GPIO_Pin_5)	<< 5;	//D5:		PB5
ret |= GPIO_ReadInputDataBit(GPIOA,  GPIO_Pin_10)	<< 6; 	//D6:		PA10
ret |= GPIO_ReadInputDataBit(GPIOA,  GPIO_Pin_11)	<< 7; 	//D7:		PA11

	return ret;
}

void WriteToCPU(u8 data)
{ 
	SETUP_2A03_DATALINE(WR_WRITE); 

GPIO_WriteBit(GPIOA, GPIO_Pin_9, data&0x01);//D0:		PA9
GPIO_WriteBit(GPIOB, GPIO_Pin_1, data&0x02);//D1:		PB1
GPIO_WriteBit(GPIOC, GPIO_Pin_15, data&0x04);//D2:		PC15
GPIO_WriteBit(GPIOB, GPIO_Pin_3, data&0x08);//D3:   		PB3
GPIO_WriteBit(GPIOB, GPIO_Pin_4, data&0x10);//D4:		PB4
GPIO_WriteBit(GPIOB, GPIO_Pin_5, data&0x20);//D5:		PB5
GPIO_WriteBit(GPIOA, GPIO_Pin_10, data&0x40); //D6:		PA10
GPIO_WriteBit(GPIOA, GPIO_Pin_11, data&0x80); //D7:		PA11
}


void SETUP_2A03_ADDRESSLINE()
{
	//SETUP ADDRESS
		GPIO_InitTypeDef GPIO_InitStruct; 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);  
	  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	  
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
GPIO_InitStruct.GPIO_Pin = 
							 GPIO_Pin_0|
						   GPIO_Pin_1|
						   GPIO_Pin_2|
						   GPIO_Pin_3|
						   GPIO_Pin_4|
						   GPIO_Pin_5|
						   GPIO_Pin_6|
						   GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
	 
GPIO_InitStruct.GPIO_Pin = 
						   GPIO_Pin_8|
						   GPIO_Pin_9|
						   GPIO_Pin_10|
						   GPIO_Pin_11|
						   GPIO_Pin_12|
						   GPIO_Pin_13|
						   GPIO_Pin_14|
						   GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
		
GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_13;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	
}
void ShowLedBuff()
{ 
	 WriteDisplayNoClear(oledBuffer_0 ,SSD1306_COLOR_BLACK, 0,0);
	 WriteDisplayNoClear(oledBuffer_1 ,SSD1306_COLOR_BLACK, 0,20);
}
void oledPrintOneLine(char* InMsg,int line)
{
	char *pch = 0;
	u8 i = 0;
	if(line == 1)
		pch = oledBuffer_0;
	else
		pch = oledBuffer_1;
	
		for( i = 0 ; i < OLED_MAX_CHAR_PER_LINE - 1 ; ++i)
	{
		pch[i] = InMsg[i];
		if(InMsg[i] == '\0')
			break;
	}
	 WriteDisplayNoClear(oledBuffer_0 ,SSD1306_COLOR_BLACK, 0,0);
	 WriteDisplayNoClear(oledBuffer_1 ,SSD1306_COLOR_BLACK, 0,20);
		
}
void oledPrint(char* InMsg)
{
	u8 i = 0 ;
	for( i = 0 ; i < OLED_MAX_CHAR_PER_LINE; ++i)
	oledBuffer_0[i]  = oledBuffer_1[i] ;
	for( i = 0 ; i < OLED_MAX_CHAR_PER_LINE - 1 ; ++i)
	{
		oledBuffer_1[i] = InMsg[i];
		if(InMsg[i] == '\0')
			break;
	}
	oledBuffer_1[OLED_MAX_CHAR_PER_LINE - 1] = '\0';
	 WriteDisplayNoClear(oledBuffer_0 ,SSD1306_COLOR_BLACK, 0,0);
	 WriteDisplayNoClear(oledBuffer_1 ,SSD1306_COLOR_BLACK, 0,20);
}
void puredelay(unsigned int nCount)
{
	    unsigned int i ;
 
    for (i = 0; i < nCount; i++) {}
}
void delay(unsigned int nCount)
{
    unsigned int i, j;
 
    for (i = 0; i < nCount; i++)
        for (j = 0; j < 0x2AFF; j++);
} 

int tickcount = 0;
void Single6502Tick(int interval)
{ 
	 char bit[80] = {0};
	u16 uP_ADDR = 0;
	u8 isread = 0;
	 u8 m2 = 0;
	u8 data = 0xEA;
	 m2 = M2Status(); 
  // Let's capture the ADDR bus
  uP_ADDR = CPU2A03_READ_ADDR();
    isread = IsRead();
	 
	 	tickcount%=0x10;
	 
  if (isread) // HIGH = READ transaction     
  {
 
		 
    // uP wants to read so Arduino to drive databus to uP:
		if(0xFFFC == uP_ADDR)
		{
			data = 0X00;
			WriteToCPU(0X00);//ROM_START_ADDR%0xFF);
		}
		else if(0xFFFD == uP_ADDR)
		{
			data = 0x80;
			WriteToCPU(0x80);//ROM_START_ADDR>>8);
		}
		else if(uP_ADDR >=0 && uP_ADDR < RAMSIZE)//RAM READ
		{
			data = CPURAM[uP_ADDR];
			WriteToCPU(CPURAM[uP_ADDR]);
		} 
		else if(uP_ADDR >=APU_REG_START && uP_ADDR < APU_REG_START+APU_REG_SIZE)//APU READ
		{
			data =APUREG[uP_ADDR-APU_REG_START];
			WriteToCPU(APUREG[uP_ADDR-APU_REG_START]);
		}
		else if(uP_ADDR >=ROM_START_ADDR && uP_ADDR < ROM_START_ADDR + ROM_SIZE)//ROM READ
		{
			data = CPUROM[uP_ADDR - ROM_START_ADDR];
			WriteToCPU(CPUROM[uP_ADDR - ROM_START_ADDR]);
		}  
  } 
  else //R/W = LOW = WRITE 
  {
		data = ReadFromCPU();
		if(uP_ADDR >=0 && uP_ADDR < RAMSIZE)//RAM WRITE
		{
			CPURAM[uP_ADDR] = data;
		}		
		else if(uP_ADDR >=APU_REG_START && uP_ADDR < APU_REG_START+APU_REG_SIZE)//APU WRITE
		{
			APUREG[uP_ADDR-APU_REG_START] = data; 
		}
		 
  }
 
#ifndef FULLSPEED_MODE 
	sprintf(bit,"%1X A:%04X,D:%02X %c %c",tickcount++,uP_ADDR,data,(isread? 'R':'W'),(m2? 'H':'L'));  
	oledPrintOneLine(bit,0);
	delay(interval); 
#endif
}

void InitInterrupt()
{
	 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
	 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA,PORTE时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
	  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource8);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line8; 
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure); 
	
	  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//使能按键WK_UP所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;					//子优先级3
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);
	
} 
void EXTI9_5_IRQHandler(void)
{  
	       if(EXTI_GetITStatus(EXTI_Line8) !=RESET)
        { 
								Single6502Tick(0);
                EXTI_ClearITPendingBit(EXTI_Line8);
        }
 
}

void TickClock(int interval,int ticks)
{
	int curticks = 0;
	while(curticks<ticks)
	{  
			GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET); 
			delay(100);
			GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET); 
			delay(interval);
		++curticks;
		
	}
} 
void TIM4_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  
		 NMICLK=!NMICLK;
		 GPIO_WriteBit(GPIOA, GPIO_Pin_15, NMICLK); 
  }
} 
 int main(void)
 {	   	
char bit[20] = {0};
	u16 uP_ADDR = 0;
	u8 isread = 0;
	u8 data = 0;  


	InitProgram();
	SETUP_2A03_ADDRESSLINE(); 
  InitInterrupt(); 

	//NMI SETUP
	//InitNMIInterrupt();
  //InitTickTImer(1800,300);//60HZ


#ifdef FULLSPEED_MODE
	InitPWM ();//PA6@24MHZ

	 oledPrint("2A03 FULLSPEED MODE");
	 oledPrint("2A03 TEST");
#else 
	 InitDisplay(); 
	 InitManualClock();
	 oledPrint("2A03 DEBUG MODE");
	 oledPrint("2A03 TEST");
#endif

    while (1)
    {
#ifndef FULLSPEED_MODE
			delay(10);
			GPIO_WriteBit(GPIOB, GPIO_Pin_0, CLK);  
			CLK=!CLK;
#endif
		 
    }

	}
 
	  