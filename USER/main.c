#include "stm32f10x.h"
#include "pwm.h"
#include "SN76496.h"
#include "delay.h"
#include "vgmdata.h"
u16 waitSamples = 0;
  u8 dataarray[4] =
{
	0x9f,
	0xbf,
	0xdf,
	0xf0
	
	
};
int currnetVGMIndex = 0;
u8 samplePlaying = 0;
u8 readBuffer()
{
	 
	u8 cmd = VGMDATA[currnetVGMIndex];
	currnetVGMIndex++;
	if(currnetVGMIndex >= VGNDATA_SIZE)
	{
		currnetVGMIndex = 0;
	}
	
	return cmd;
}


 u16 readBuffer16()
{
  u16 d;
  u8 v0 = readBuffer();
  u8 v1 = readBuffer();
  d = (v0 + (v1 << 8));
  //bufferPos+=2;
 // cmdPos+=2;
  return d;
}

u16 proccessVGMCmd()
{
	u8 cmd = readBuffer();
	
	switch(cmd)
  {
    case 0x4F:
		{
    SN76496_SendData(0x06);
    SN76496_SendData(readBuffer());
		}
    return 1;
    case 0x50:
		{
    SN76496_SendData(readBuffer());
		}
    return 1;
		    case 0x61:
				{ 
					return  readBuffer16();
				}
    case 0x62:
    return 735;
    case 0x63:
    return 882;
		case 0x7F:
    {
      return (cmd & 0x0F)+1;
    }
  }
  return 1 ;
	
	
	
	
	
}

 void mydelay(int d)
 {
	 int i = 0;
	 for(i = 0; i < d;i++){}
 }
 
void Tick(void)
{
 
  if(waitSamples > 0)
	{
    waitSamples--;
		return;
	}
	if(waitSamples == 0 && 0==samplePlaying)
  {
    samplePlaying = 1;
    waitSamples += proccessVGMCmd();
    samplePlaying = 0;
    return;
  } 
}
 
 int main(void)
 {	  
	   u8 blab = 0;
	 u8 fourMTick = 160;
	 u8 curFTick = fourMTick;
	 InitPWM();
	 SN76496_Init(); 
	 //delay_init();
 
	
	//SN76496_SendData(0x9f);
	//SN76496_SendData(0xbf);
	//SN76496_SendData(0xdf);
	//SN76496_SendData(0xff);
	 
	 while(1)
	 { 		
		if(curFTick>0)
		{
			curFTick--;
		}
		else
		{
			curFTick = fourMTick;
		 Tick();
		}
		
		 //mydelay(100);
		// delay_ms(proccessVGMCmd());
		 
		 //delay_ms(20);
		// 		SN76496_SendData(blab++);//10010000‬  
 
	//		SN76496_UpdateAttenuator	
		
	 }
	  
 }
 