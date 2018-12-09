#ifndef SN76496
#define SN76496

#include "stdint.h"
#include "stm32f10x_gpio.h" 


uint8_t 	READY_Read(void); 

uint8_t 	CE_Read(void);  
void 				CE_Write( BitAction act);
 
uint8_t 	WE_Read(void);  
void 				WE_Write( BitAction act);

void SN76496_Init(void);
void SN76496_SendData(unsigned char data);

//Chip Operations
 typedef enum
{ 
    ESNCTone_0 = 0, //00
    ESNCTone_1,     //01
    ESNCTone_2,     //10
    ESNCTone_3,     //11
}ESN76496Channel;

 typedef enum
{ 
    White_Noise = 0,
    Periodic_noise
}ENoiseConfig;



/*
                      Input clock (Hz) (4M)
   Frequency (Hz) = -------------------------
                       32 x register value 


UpdateNoiseSource:
High <--                 --> Low
|bits|00|01|02|03|04|05|06 |07 |
|data|01|R0|R1|R2|--|FB|NF0|NF1|

Where 
-----------------------
|FB|   CONFIGURATlON  |
|00|  "Periodic" Noise|
|01|  "White" Noise   |
-----------------------


NOISE GENERATOR FREQUENCY CONTROL

 BITS
NFO|NFl| SHIFT RATE
|0 |0  |N/512                    |
|0 |0  |N/1024                   |
|1 |0  |N/2048                   |
|1 |1  |Tone Generator #3 Output |

------------------------------------------------------------------------------------

SN76489 register writes
When a byte is written to the SN76489, it processes it as follows:

If bit 7 is 1 then the byte is a LATCH/DATA byte.

  %1cctdddd
    |||````-- Data
    ||`------ Type
    ``------- Channel
Bits 6 and 5 (cc) give the channel to be latched, ALWAYS. This selects the row in the above table - %00 is channel 0, %01 is channel 1, %10 is channel 2, %11 is channel 3 as you might expect.
Bit 4 (t) determines whether to latch volume (1) or tone/noise (0) data - this gives the column.

The remaining 4 bits (dddd) are placed into the low 4 bits of the relevant register. For the three-bit noise register, the highest bit is discarded.

The latched register is NEVER cleared by a data byte.

If bit 7 is 0 then the byte is a DATA byte.

  %0-DDDDDD
    |``````-- Data
    `-------- Unused
If the currently latched register is a tone register then the low 6 bits of the byte (DDDDDD) are placed into the high 6 bits of the latched register. If the latched register is less than 6 bits wide (ie. not one of the tone registers), instead the low bits are placed into the corresponding bits of the register, and any extra high bits are discarded.

The data have the following meanings (described more fully later):

Tone registers
DDDDDDdddd = cccccccccc
DDDDDDdddd gives the 10-bit half-wave counter reset value.

Volume registers
(DDDDDD)dddd = (--vvvv)vvvv
dddd gives the 4-bit volume value.
If a data byte is written, the low 4 bits of DDDDDD update the 4-bit volume value. However, this is unnecessary.

Noise register
(DDDDDD)dddd = (---trr)-trr
The low 2 bits of dddd select the shift rate and the next highest bit (bit 2) selects the mode (white (1) or "periodic" (0)).
If a data byte is written, its low 3 bits update the shift rate and mode in the same way.


----------------------------------------
REGISTER ADDRESS FIELD

RO	Rl	R2	DESTINATION CONTROL REGISTER
0 	0 	0 	Tone 1 Frequency
0 	0 	0 	Tone 1 Attenuation
0 	1 	0 	Tone 2 Frequency
0 	1 	1 	Tone 2 Attenuation
1 	0 	0 	Tone 3 Frequency
1 	0 	1 	Tone 3 Attenuation
1 	1 	0 	Noise Control
1 	1 	1 	Noise Attenuation 
----------------


-----------------
Update Tone Freq (CH1-3 ONLY)(Double bytes transfer):
BIT7										BIT0    B7									B0 
		|CHANNEL|	|FREQ DATA LOW|		| | |   FREQ DATA HI  |
1		|CH0|CH1|0|F3|F2|F1|F0  |		|0|-|F9|F8|F7|F6|F5|F4|
				FIRST BYTE										SEQONC BYTE


UPDATE NOISE SOURCE (SINGLE BYTE TRANSFER)

BIT7								BIT0
|					|  | SHIFT |
|1|1|1|0|-|FB|NF1|NF0|


UPDATE ATTENUATOR  (SINGLE BYTE TRANSFER)

B7											B0
| |CHANNEL| | ATTENUATOR|
|1|CH0|CH1|1|A3|A2|A1|A0|

*/
  
void SN76496_UpdateNoiseSource(unsigned char data);
void SN76496_UpdateAttenuator(ESN76496Channel channel,unsigned char data);


#endif
