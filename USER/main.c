
//#define DISABLE_PLAYBACK
#define SELF_TEST
 
#include "stm32f10x.h"
#include "sys.h"
#include "key.h"
#include "pwm.h"
#include "SN76496.h"
#include "delay.h"
#include "YM2612.h"
//#include "usmart.h" 
#include "sram.h" 
#include <string.h>


#ifndef DISABLE_PLAYBACK 

#ifdef SELF_TEST
#define MAX_PCM_BUFFER_SIZE 50000
#else
#define MAX_PCM_BUFFER_SIZE 5 //5k
#endif
#else


#define MAX_PCM_BUFFER_SIZE 5
#endif

#ifdef SELF_TEST
#include "vgmdata.h"
#endif

#define MAX_FILE_BUFF_SIZE 550*1024 //550kyte

#ifdef SELF_TEST

#else
const unsigned int VGMDATA_SIZE = MAX_FILE_BUFF_SIZE; 
#endif

#include "ssd1306.h"
#include "fattester.h"	
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"  
#include "../BSP/display.h"
#define LED0 PBout(5)                                  // PB5
#define LED1 PEout(5)                                  // PE5
 

#ifdef SELF_TEST
u8 PCMBuffer[MAX_PCM_BUFFER_SIZE] = {0};
#else
u8 *PCMBuffer = 0;
#endif
u8 playbackState = 0;//0-stop 1-play 

#define KEY0_PRES 1 //KEY0°´ÏÂ

#define KEY1_PRES 2 //KEY1°´ÏÂ
#define KEY2_PRES 3 //KEY2°´ÏÂ
#define WKUP_PRES 4 //KEY_UP°´ÏÂ(¼´WK_UP/KEY_UP)
u8* fileBufferPointer;
u16 waitSamples = 0; 
 
int currnetVGMIndex = 0;
u8 samplePlaying = 0;
int pcmBufferPosition = 0;
u8 ramPrefetch = 0x00;
u8 ramPrefetchFlag = 0;
void StartPlayback()
{
 currnetVGMIndex = 0;
 samplePlaying = 0;
  pcmBufferPosition = 0;
 ramPrefetch = 0x00;
 ramPrefetchFlag = 0; 
 waitSamples = 0; 	
 playbackState = 1;
}
u8 readBuffer()
{
#ifdef SELF_TEST
  u8 cmd = VGMDATA[currnetVGMIndex];
	#else
  u8 cmd = fileBufferPointer[currnetVGMIndex];
  #endif
	currnetVGMIndex++;
  if (currnetVGMIndex >= VGMDATA_SIZE)
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

int readBuffer32()
{
  u8 v0 = readBuffer();
  u8 v1 = readBuffer();
  u8 v2 = readBuffer();
  u8 v3 = readBuffer();
  return (v0 + (v1 << 8) + (v2 << 16) + (v3 << 24));
}

u16 proccessVGMCmd()
{

  u8 cmd = readBuffer();
  int PCMSize = 0;
  int i = 0;
  u8 addr = 0;
  u8 data = 0;
  u8 wait = 0;
  switch (cmd)
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
  case 0x52:
  {
    addr = readBuffer();
    data = readBuffer();
    YM2612_SendDataPins(addr, data, Bit_RESET);
  }
    return 1;
  case 0x53:
  {
    addr = readBuffer();
    data = readBuffer();
    YM2612_SendDataPins(addr, data, Bit_SET);
  }
    return 1;
  case 0x61:
  {
    return readBuffer16();
  }
  case 0x62:
    return 735;
  case 0x63:
    return 882;
  case 0x67: //PCM Buffer filling.
  {
    readBuffer16(); //Discard 0x66 and datatype byte
    pcmBufferPosition = currnetVGMIndex;
    PCMSize = readBuffer32();
    if (PCMSize > MAX_PCM_BUFFER_SIZE )
    {
      //     // startTrack(NEXT);
      //     // Serial.println("PCM Size too big!");
    }
    for (i = 0; i < PCMSize; i++)
    {
      PCMBuffer[i] = readBuffer();
    }
  }
    return 0;

  case 0x70:
  case 0x71:
  case 0x72:
  case 0x73:
  case 0x74:
  case 0x75:
  case 0x76:
  case 0x77:
  case 0x78:
  case 0x79:
  case 0x7A:
  case 0x7B:
  case 0x7C:
  case 0x7D:
  case 0x7E:
  case 0x7F:
  {
    wait = (cmd & 0x0F) + 1;
    return  wait;
  }
  case 0x80:
  case 0x81:
  case 0x82:
  case 0x83:
  case 0x84:
  case 0x85:
  case 0x86:
  case 0x87:
  case 0x88:
  case 0x89:
  case 0x8A:
  case 0x8B:
  case 0x8C:
  case 0x8D:
  case 0x8E:
  case 0x8F:
  {
    //RAM Prefetching. Store the next byte of PCM sample data in a char that will cache itself between samples
    ramPrefetchFlag = 1;
    pcmBufferPosition++;
    wait = (cmd & 0x0F);
    YM2612_SendDataPins(0x2A, ramPrefetch, Bit_RESET);
    return  wait;
  }
  case 0xE0:
  {
    pcmBufferPosition = readBuffer32();
    return 0;
  }
    // case 0x66://LOOP
    // {
    // ready = false;
    // clearBuffers();
    // cmdPos = 0;
    // injectPrebuffer();
    // loopCount++;
    // ready = true;
    //return 0;
    
    // }
  }
  return 0;
}

void mydelay(int d)
{
  int i = 0;
  for (i = 0; i < d; i++)
  {
  }
}
 
void Tick(void)
{
  LED0 = 1;
  if (waitSamples > 0)
  {
    waitSamples--;
  //  return;
  }
  if (waitSamples == 0 && 0 == samplePlaying)
  {
    samplePlaying = 1;
    waitSamples += proccessVGMCmd();
    samplePlaying = 0;
    LED0 = 0;
    return;
  }
}
void InitLED(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE, ENABLE); //Ê¹ÄÜPB,PE¶Ë¿ÚÊ±ÖÓ

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;         //LED0-->PB.5 ¶Ë¿ÚÅäÖÃ
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //ÍÆÍìÊä³ö
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO¿ÚËÙ¶ÈÎª50MHz
  GPIO_Init(GPIOB, &GPIO_InitStructure);            //¸ù¾ÝÉè¶¨²ÎÊý³õÊ¼»¯GPIOB.5
  GPIO_SetBits(GPIOB, GPIO_Pin_5);                  //PB.5 Êä³ö¸ß

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //LED1-->PE.5 ¶Ë¿ÚÅäÖÃ, ÍÆÍìÊä³ö
  GPIO_Init(GPIOE, &GPIO_InitStructure);    //ÍÆÍìÊä³ö £¬IO¿ÚËÙ¶ÈÎª50MHz
  GPIO_SetBits(GPIOE, GPIO_Pin_5);          //PE.5 Êä³ö¸ß
}
 
#ifndef DISABLE_PLAYBACK
void TIM4_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  
		if( playbackState == 1)
		{
    LED1=!LED1; 
    {

    
      if (ramPrefetchFlag == 1)
      {
        ramPrefetch = PCMBuffer[pcmBufferPosition];
        ramPrefetchFlag = 0;
      }
      Tick();
    }
		}
  }
}
#endif
void DisplayShowTestPage(char * InfomationL1,char *InfomationL2)
{
		PageContainer_t container;
		container.header = InfomationL1;
		container.line_one = InfomationL2; 
		DrawPage(&container);
} 
void string_copy(char *value1,char *value2,u8 width)
{
u8 i;

for(i=0;i<width;i++)
{
value1[i]=value2[i];
}
}
u8 curmsgline = 1;  
char *curlog = "";  
char *prevlog = "";
void PrintMsg(char* msg)
{  
  prevlog = curlog;
	curlog = msg;
	WriteDisplayNoClear(prevlog , DISPLAY_BLUE_COLOR , 0 , 5); 
	WriteDisplayNoClear(curlog , DISPLAY_BLUE_COLOR , 0 , 20); 
	//delay_ms(500);
}

unsigned char tolower(unsigned char ch) {
    if (ch >= 'A' && ch <= 'Z')
        ch = 'a' + (ch - 'A');
    return ch;
 }

int strcasecmp(const char *s1, const char *s2) {
    const unsigned char *us1 = (const unsigned char  *)s1,
                        *us2 = (const unsigned char  *)s2;

    while (tolower(*us1) == tolower(*us2++))
        if (*us1++ == '\0')
            return (0);
    return (tolower(*us1) - tolower(*--us2));
}

int currFileIndex = 0;
void LoadNextFile(u8* path)
{
	FRESULT res;	
	FRESULT res_file;	
	u8 tmpfileptr[128];
int curidx = 0;  
	char* ext = ".vgm";
	char  fext[4] = {0};
	
	char fullpath[128]={0};
	u8 i = 0;
	FIL  loadfile;
  char *fn;   /* This function is assuming non-Unicode cfg. */ 
	int flnamelen = 0;
	int pathlen = 0;
	playbackState = 0;//stop playback
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = mymalloc(SRAMIN,fileinfo.lfsize);
#endif		  

	printf("LoadNextFile:\r\n");
    res = f_opendir(&dir,(const TCHAR*)path); 
	printf("res:%d\r\n",res);
	PrintMsg("Reading Files...");
    if (res == FR_OK) 
	{	
		printf("---------\r\n"); 
		while(1)
		{
	        res = f_readdir(&dir, &fileinfo);                    
	        if (res != FR_OK || fileinfo.fname[0] == 0) break;  
	        //if (fileinfo.fname[0] == '.') continue;             

			
#if _USE_LFN
        	fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else							   
        	fn = fileinfo.fname;
#endif	                                              /* It is a file. */
			printf("%s/", path);//´òÓ¡Â·¾¶	
			printf("%s\r\n",  fn);//´òÓ¡ÎÄ¼þÃû	 
			flnamelen = 		strlen(fn);	 
			pathlen = 		strlen(path);	
			
			
			printf("flnamelen:%d\r\r\n",flnamelen); 
			if(MAX_FILE_BUFF_SIZE < fileinfo.fsize || (flnamelen+pathlen)>128)
			{
				printf("FILE OR IT'S NAME IS TOO BIG!! [%ld]\r\n",  fileinfo.fsize);//´òÓ¡ÎÄ¼þÃû	
				continue;
			}
			if(flnamelen >= 4)
			{
				for(i = 0;i < 4; i++)
				{
					fext[i] = fn[flnamelen-4+i];
					printf("fext:%s,[%d]:%c\r \r\n",fext,i,fn[i]); 
				}
				if( 0 == strcasecmp(fext,ext))
				{
					PrintMsg("Reading Files...");
				/*	for(i = 0; i < 128;i++)
					{
						if(i<pathlen)
						{
							fullpath[i] = path[i];
						}
						else if(i == pathlen)
						{
							fullpath[i] = '/';
						}
						else if(i<flnamelen +pathlen)
						{
							
							fullpath[i] = fn[i-pathlen];
						}
						
					printf("Reading fullpath [%d][%s]\r\r\n",i,fullpath);
					} */
					sprintf(&fullpath,"%s/%s",path,fn);
						//	strcpy((char*)pname,path);				//¸´ÖÆÂ·¾¶(Ä¿Â¼)
							//strcat((char*)pname,(const char*)fn);  			//½«ÎÄ¼þÃû½ÓÔÚºóÃæ
					printf("Reading fullpath [%s],size:(%ld)\r\r\n",fullpath,fileinfo.fsize);
					res_file=f_open(&loadfile,(const TCHAR*)fullpath,FA_READ);  
					if(res_file) 
					{
						printf("Open Error:%d\r\n",res_file); 
					}
					
            if(fileBufferPointer)
            myfree(SRAMEX,fileBufferPointer);
						
	          fileBufferPointer=mymalloc(SRAMEX,fileinfo.fsize);
					res_file=f_read(&loadfile,fileBufferPointer,fileinfo.fsize,&br); 
					printf("dumping vgm file...\r\r\n");
					for(i=0;i<128;i++)
					{
						printf("0x%02X\t",fileBufferPointer[i]);
					}
					if(res_file)
					{
						printf("Read Error:%d\r\n",res_file); 
					}
					else
					{
						f_close(&loadfile);
						 
            
            if(PCMBuffer)
            myfree(SRAMEX,PCMBuffer);
#ifndef SELF_TEST
	          PCMBuffer=mymalloc(SRAMEX,MAX_PCM_BUFFER_SIZE);
#endif
            StartPlayback();
						break;
					}
		
				}
				else
				{ 
					printf("Skipping file %s\r\n",  fn);//´òÓ¡ÎÄ¼þÃû	 
				} 
			}
		} 
    }	  
	myfree(SRAMIN,fileinfo.lfname);
       
}
void TestSDCard(void)
{
	
  	u32 total,free;
	u8 t=0;	
	u8 res=0;	
	 

	//usmart_dev.init(72);		//初始化USMART		 
	W25QXX_Init();				//初始化W25Q128
 	FSMC_SRAM_Init();			 
 	 my_mem_init(SRAMIN);		//初始化内部内存池 
 	my_mem_init(SRAMEX);		//初始化内部内存池  
		printf("allocating ext memory... \r\n");
		printf("exfuns_init\r\n");
 	exfuns_init();							//为fatfs相关变量申请内存		
		printf("f_mount\r\n");		
res =   f_mount(fs[0],"0:",1);
	if(res)//挂载SD卡 
	{
		delay_ms(200);
		printf("f_mount error! retrying....\r\n"); 
		res =   f_mount(fs[0],"0:",1);
	}
		printf("f_mount 0:success.\r\n"); 
 	res=f_mount(fs[1],"1:",1); 				//挂载FLASH.	
	if(res==0X0D)//FLASH磁盘,FAT文件系统错误,重新格式化FLASH
	{
		printf("Flash Disk Formatting...\r\n\r\n");	//格式化FLASH
		res=f_mkfs("1:",1,4096);//格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇
		if(res==0)
		{
			f_setlabel((const TCHAR *)"1:ALIENTEK");	//设置Flash磁盘的名字为：ALIENTEK
			printf("Flash Disk Format Finish\r\n");	//格式化完成
		}else printf("Flash Disk Format Error \r\n");	//格式化失败
		delay_ms(1000);
	}					
	
		printf("exf_getfree\r\n");		 
	while(exf_getfree("0",&total,&free))	//得到SD卡的总容量和剩余容量
	{
		delay_ms(200);
		printf( "exf_getfree   Error!\r\n"); 
		LED0=!LED0;//DS0闪烁
	}						
	
	PrintMsg( "FATFS OK!."); 
	printf( "FATFS OK! total:%d,free:%d\r\n",total,free); 
	LoadNextFile("0:");//LOAD NEXT SONG.
}

void ScanKeys(void)
{
 
		u8 key=KEY_Scan(0);		//²»Ö§³ÖÁ¬°´	
		
		switch(key)
		{
			case 0:				//Ã»ÓÐ°´¼ü°´ÏÂ	
				
	//printf( "key:%d\r\n",key,free);
				break;
			case KEY0_PRES:		//KEY0°´ÏÂ
				
	printf( "key:%d[RIGHT]\r\n",key,free);
				break;
			case KEY1_PRES:		//KEY1°´ÏÂ	 //  
				
	printf( "key:%d[DOWN]\r\n",key,free);
				break;
			case KEY2_PRES:		//KEY2°´ÏÂ	  
				
	printf( "key:%d[LEFT]\r\n",key,free);
			break;
			case WKUP_PRES:		//KEY UP°´ÏÂ 
				
	printf( "key:%d[UP]\r\n",key,free);
				break;
		}
}
int main(void)
{
  u8 fourMTick = 40;
  u8 curFTick = fourMTick;
	u8 bbb=0;
  delay_init();
   InitLED();  
	KEY_Init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
	InitDisplay();    	
#ifndef DISABLE_PLAYBACK
  YM2612_Init();
  InitPWM();
  InitTickTImer(1631,0);//about 44.1 KHz
 SN76496_Init();
#endif
	
 LED1 = 0; 
#ifndef SELF_TEST
	 PrintMsg( "InitSDCard.");   
	printf("INIT SD CARD");
 	 TestSDCard();
#else
	 PrintMsg( "SELF TEST");   
	 PrintMsg( "MODE ENABLED");   
	printf("SELF_TEST\r\r\n");
StartPlayback();
#endif
  while (1){
		ScanKeys();
		//SN76496_SendData(bbb++);
  } 
} 