#include "YM2612.h"

//Notes
#define YM_RD GPIOF,GPIO_Pin_13 //YM_RD = PF13
#define YM_WR GPIOF,GPIO_Pin_12 //YM_WR = PF12
#define YM_A0 GPIOF,GPIO_Pin_11  //YM_A0 =PF11
#define YM_A1 GPIOF,GPIO_Pin_10  //YM_A1 =PF10
#define YM_CS GPIOF,GPIO_Pin_9  //YM_CS = PF9
#define YM_IC GPIOF,GPIO_Pin_8  //YM_IC = PF8 

#define YM2612_PIANO_DATA_LENGHT 38
// piano sound
unsigned char YM2612_test_patch[YM2612_PIANO_DATA_LENGHT] = {
  0x00,0x00,
  0x00,
  0x00,
  0x71,0x0d,0x33,0x01,
  0x23,0x2d,0x26,0x00,
  0x5f,0x99,0x5f,0x94,
  0x05,0x05,0x05,0x07,
  0x02,0x02,0x02,0x02,
  0x11,0x11,0x11,0xa6,
  0x00,0x00,0x00,0x00,
  0x32,0xc0,
  0x00,
  0x22,0x69,
  0xf0,
}; 

unsigned char YM2612_test_addr[YM2612_PIANO_DATA_LENGHT] = {
  0x22,0x27,
  0x28,
  0x2b,
  0x30,0x34,0x38,0x3c,
  0x40,0x44,0x48,0x4c,
  0x50,0x54,0x58,0x5c,
  0x60,0x64,0x68,0x6c,
  0x70,0x74,0x78,0x7c,
  0x80,0x84,0x88,0x8c,
  0x90,0x94,0x98,0x9c,
  0xb0,0xb4,
  0x28,
  0xa4,0xa0,
  0x28
};
u8 ym2612_selftestTimes = 3;
void YM2612_InitTestPiano(void)
{
    unsigned char i = 0,times = 0;
int r1,r2 =0;
     
    YM2612_Init();
     for (i = 0; i < YM2612_PIANO_DATA_LENGHT; i++)  
     { 
         YM2612_SendDataPins(YM2612_test_addr[i],YM2612_test_patch[i],Bit_RESET); 
     }  

    while(times < ym2612_selftestTimes)
    {  
        for (i=0; i<10; ++i) 
        {    
            r1 = 0x30+rand()%(0x20);
            r2 = rand()%255;
        // YM2612_SendDataPins(rand(),rand(),Bit_RESET);
        }
    YM2612_SendDataPins(0x28,0xF0,Bit_RESET);    
    YM2612_SendDataPins(0x28,0xF1,Bit_RESET);    
    YM2612_SendDataPins(0x28,0xF2,Bit_RESET);  
    LED0=0;
	LED1=0;   
    delay_ms(500);

    //PORTB &= ~0x4;
    YM2612_SendDataPins(0x28,0x00,Bit_RESET);
    YM2612_SendDataPins(0x28,0x01,Bit_RESET);
    YM2612_SendDataPins(0x28,0x02,Bit_RESET);
    LED0=1;
	LED1=1; 
    delay_ms(500);
    ++times;
    }
 

}
 

void YM2612_Init(void)
{
    //DATA PIN SETUP
        unsigned char ledft = 0;
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;			    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOF, &GPIO_InitStructure); 
    


    GPIO_WriteBit(YM_CS,Bit_RESET); //_CS HIGH 
    GPIO_WriteBit(YM_RD,Bit_SET); //_RD HIGH 
    GPIO_WriteBit(YM_WR,Bit_SET);  //_WR HIGH 
    GPIO_WriteBit(YM_IC,Bit_SET);  //_IC HIGH
    
    //GPIO_WriteBit(YM_IC,Bit_RESET);  //_IC HIGH
 
    GPIO_WriteBit(YM_A0,Bit_RESET); //_A0 LOW 
    GPIO_WriteBit(YM_A1,Bit_RESET); //_A1 LOW 
    YM2612_Reset();
    while(ledft < 2)
    {
    LED0=0;
	LED1=0; 
        delay_ms(100);
    LED0=1;
	LED1=1; 
        delay_ms(100);
        ++ledft;
    }


} 

void YM2612_Send8BitData(const unsigned char data)
{
    //GPIOF->ODR =0;
    //GPIOF->ODR = data;
    
	 GPIO_WriteBit(GPIOF,GPIO_Pin_7,  (data&128)?Bit_SET:Bit_RESET);
	 GPIO_WriteBit(GPIOF,GPIO_Pin_6,  (data&64)?Bit_SET:Bit_RESET);
	 GPIO_WriteBit(GPIOF,GPIO_Pin_5,  (data&32)?Bit_SET:Bit_RESET);
	 GPIO_WriteBit(GPIOF,GPIO_Pin_4,  (data&16)?Bit_SET:Bit_RESET);
	 GPIO_WriteBit(GPIOF,GPIO_Pin_3,  (data&8)?Bit_SET:Bit_RESET);
	 GPIO_WriteBit(GPIOF,GPIO_Pin_2,  (data&4)?Bit_SET:Bit_RESET);
	 GPIO_WriteBit(GPIOF,GPIO_Pin_1,  (data&2)?Bit_SET:Bit_RESET);
	 GPIO_WriteBit(GPIOF,GPIO_Pin_0,  (data&1)?Bit_SET:Bit_RESET);
}
void YM2612_Reset(void)
{ 
    GPIO_WriteBit(YM_IC,Bit_RESET);  // GPIOB->regs->BSRR = (1U << 4) << (16 * 1); //_IC LOW 
    
	delay_us(300);
    GPIO_WriteBit(YM_IC,Bit_SET);  // GPIOB->regs->BSRR = (1U << 4) << (16 * 0); //_IC HIGH 
}
 
void YM2612_SendDataPins(unsigned char addr, unsigned char data, BitAction setA1) //0x52 = A1 LOW, 0x53 = A1 HIGH
{
    
    
    GPIO_WriteBit(YM_A1, setA1);  // GPIOB->regs->BSRR = (1U << 0) << (16 * !setA1); //_A1 PB0  

    GPIO_WriteBit(YM_A0,Bit_RESET);// GPIOA->regs->ODR &= ~(0x0800); //_A0 LOW 
     GPIO_WriteBit(YM_CS,Bit_RESET);// GPIOB->regs->ODR &= ~(0x0808); //_CS LOW
    YM2612_Send8BitData(addr);// _bus->Write(addr);  
	//  delay_us(1);
    GPIO_WriteBit(YM_WR,Bit_RESET);// GPIOA->regs->ODR &= ~(0x1000); //_WR LOW
	// delay_us(5);
    GPIO_WriteBit(YM_WR,Bit_SET);// GPIOA->regs->ODR |= 0x1000;    //_WR HIGH
     GPIO_WriteBit(YM_CS,Bit_SET);// GPIOB->regs->ODR |= 0x0808;    //_CS HIGH
    GPIO_WriteBit(YM_A0,Bit_SET);/// GPIOA->regs->ODR |= 0x0800;    //_A0 HIGH
    GPIO_WriteBit(YM_CS,Bit_RESET);// GPIOB->regs->ODR &= ~(0x0808); //_CS LOW
    GPIO_WriteBit(YM_A1, setA1);  // GPIOB->regs->BSRR = (1U << 0) << (16 * !setA1); //_A1 PB0  
    YM2612_Send8BitData(data);// _bus->Write(data);
    GPIO_WriteBit(YM_WR,Bit_RESET);/// GPIOA->regs->ODR &= ~(0x1000); //_WR LOW
    GPIO_WriteBit(YM_WR,Bit_SET);/// GPIOA->regs->ODR |= 0x1000;    //_WR HIGH
    GPIO_WriteBit(YM_CS,Bit_SET);// GPIOB->regs->ODR |= 0x0808;    //_CS HIGH
}
