#ifndef __CCODEC_H
#define __CCODEC_H

#include "cornfedsipua.h"
#include "soundcard.h"

//#define EIGHT_KHZ								0x0

typedef enum
{
   EIGHT_KHZ         =  0x0,
   ELEVEN_KHZ        =  0x1,
   TWELVE_KHZ        =  0x2,
   SIXTEEN_KHZ       =  0x4,
   TWENTY_TWO_KHZ    =  0x5,
   TWENTY_FOUR_KHZ   =  0x6,
   THIRTY_TWO_KHZ    =  0x8,
   FOURTY_FOUR_KHZ   =  0x9,
   FOURTY_EIGHT_KHZ  =  0xA,
   NINETY_SIX_KHZ    =  0xE,
} SAMPLING_RATE_TYPE;
typedef enum
{
   TWELVE_DB,
   TEN_DB,
   EIGHT_DB,
   SIX_DB,
   FOUR_DB,
   TWO_DB,
   ZERO_DB,
   NEG_TWO_DB,
   NEG_FOUR_DB,
   NEG_SIX_DB,
   NEG_EIGHT_DB,
   NEG_TEN_DB,
   NEG_TWELVE_DB,
   NEG_FOURTEEN_DB,
   NEG_SIXTEEN_DB,
   NEG_EIGHTEEN_DB,
   NEG_TWENTY_DB,
   NEG_TWENTY_TWO_DB,
   NEG_TWENTY_FOUR_DB
} ANALOG_VOLUME_TYPE;
typedef enum
{
   ALL,
   ARXL1,
   ARXR1,
   ARXL2,
   ARXR2,
   VDL
} GAIN_OUTPUT_TYPE;

#ifdef __cplusplus
extern "C" {
#endif
/*	void cInitialize (void);*/

	void cReset (void);

	void cSetSamplingRate (SAMPLING_RATE_TYPE rate);

	int cPoll_Serial(void);
	
	int cReadSample (INT16 *data);

	int cWriteSample (INT16 data);
	
	void cEnable_Analog_Loopback(int enable);
	
	//void cEnable_Headset (void);
	
	void cMute (int enabled);
	
	void cSet_Mic_Gain (u8_t volume);
	
	
	void cInitialize_HISR (void (*hisr_function)());
	
	void cSet_Volume (GAIN_OUTPUT_TYPE output, ANALOG_VOLUME_TYPE volume);
	
	void cToggleLED (u8_t led_mask);
	
#ifdef __cplusplus
}
#endif

#endif
