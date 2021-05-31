#include <stdint.h>
#include <RF69.h>
#include <RF69_avr.h>
#define __PROG_TYPES_COMPAT__
#define NOP __asm__ __volatile__ ("nop\n\t")
#include <avr/pgmspace.h>
#include <util/delay_basic.h>
#include "RFAPI.h"
extern rfAPI rfapi;
///////////////////////////////////////////////////////////////////////////////

#define TX_INTERRUPT 1

#define SX1276	0	// Also see setting in RF69_avr.h & RFxConsole.ino

///////////////////////////////////////////////////////////////////////////////
#define ROM_UINT8       const uint8_t // Does this change storage to RAM?
#define ROM_READ_UINT8  pgm_read_byte
#define ROM_DATA        PROGMEM
#define RF_MAX   72

#if !SX1276
// How to get this working... extern PGM_P constant char radioType[] PROGMEM = "RFM69 ";
#warning RF69.cpp: Building for RFM69xx 
      
#define LIBRARY_VERSION     15      // Stored in REG_SYNCVALUE6 by initRadio 
#define REG_FIFO            0x00
#define REG_OPMODE          0x01
#define DATAMODUL           0x02 
#define REG_BITRATEMSB      0x03
#define REG_BITRATELSB      0x04
#define REG_BITFDEVMSB		0x05
#define REG_BITFDEVLSB		0x06
#define REG_FRFMSB          0x07
#define REG_OSC1            0x0A
#define REG_PALEVEL			0x11
#define REG_OCP             0x13
#define REG_LNA             0x18
#define REG_AFCFEI          0x1E
#define REG_AFCMSB          0x1F
#define REG_AFCLSB          0x20
#define REG_FEIMSB          0x21
#define REG_FEILSB          0x22
#define REG_RSSICONFIG      0x23
#define REG_RSSIVALUE       0x24
#define REG_DIOMAPPING1     0x25
#define REG_IRQFLAGS1       0x27
#define REG_IRQFLAGS2       0x28
#define REG_RSSITHRESHOLD   0x29
#define REG_SYNCCONFIG      0x2E
#define REG_SYNCVALUE1      0x2F
#define REG_SYNCVALUE2      0x30
#define REG_SYNCVALUE3      0x31
#define REG_SYNCVALUE4      0x32
#define REG_SYNCVALUE5      0x33
#define REG_SYNCVALUE6      0x34
#define REG_SYNCVALUE7      0x35
#define REG_SYNCVALUE8      0x36
#define REG_SYNCGROUP       0x32
#define REG_NODEADRS        0x39
#define REG_FIFOTHRESH      0x3C
#define REG_PACKETCONFIG2   0x3D
#define REG_AESKEY1         0x3E
#define REG_TEMP1           0x4E
#define REG_TEMP2           0x4F
#define REG_TESTLNA         0x58
#define REG_TESTPA1         0x5A
#define REG_TESTPA2         0x5C

#define MODE_SLEEP          0x00
#define MODE_STANDBY        0x04
#define MODE_FS             0x08    // Unpredictable results with this value
#define MODE_RECEIVER       0x10
#define MODE_LISTENABORT    0x20
#define MODE_LISTENON       0x40
#define MODE_TRANSMITTER    0x0C
#define MODE_SEQUENCER_OFF  0x80
#define MODE_MASK           0x1C
#define TESTLNA_NORMAL      0x1B
#define TESTLNA_BOOST       0x2D
#define TESTPA1_NORMAL      0x55
#define TESTPA1_20DB		0x5D
#define TESTPA2_NORMAL      0x70
#define TESTPA2_20DB		0x7C
#define OCP_NORMAL			0x1A
#define OCP_20DB			0x0F

#define COURSE_TEMP_COEF      -89 // starter callibration figure
#define RF_TEMP1_MEAS_START   0x08
#define RF_TEMP1_MEAS_RUNNING 0x04

#define IRQ1_MODEREADY      0x80
#define IRQ1_RXREADY        0x40
#define IRQ1_PLL            0x10
#define IRQ1_RSSI           0x08
#define IRQ1_TIMEOUT        0x04
#define IRQ1_SYNCMATCH      0x01

#define START_TX            0x80  // With 125Khz SPI a minimum
#define DELAY_TX            0x18  // 22 byte head start required, 24 to be safer 


#define IRQ2_FIFOFULL       0x80
#define IRQ2_FIFONOTEMPTY   0x40
#define IRQ2_FIFOOVERRUN    0x10	// Also OK for SX1276
#define IRQ2_PACKETSENT     0x08
#define IRQ2_PAYLOADREADY   0x04

#define PACKET2_RESTART     0x04

#define DIO0_PACKETSENT     0x00

// FS Mode
#define DIO0_FS_UNDEF_RX    0x40
#define DIO0_FS_UNDEF_TX    0x80
// RX Mode
#define DIO0_CRCOK          0x00
#define DIO0_PAYLOADREADY   0x40
#define DIO0_SYNCADDRESS    0x80
#define DIO0_RSSI           0xC0
// TX Mode
#define DIO0_TX_UNDEFINED   0x80
#define DIO0_PACKETSENT     0x00
#define DIO3_FIFOFULL       0x00
#define DIO3_RSSI           0x01
#define DIO3_SYNCADDRESS    0x02
#define DIO3_FIFOFULL_TX    0x00
#define DIO3_TX_UNDEFINED   0x02

#define RcCalStart          0x81
#define RcCalDone           0x40
#define FeiStart            0x20
#define FeiDone             0x40
#define RssiStart           0x01
#define RssiDone            0x02
#define oneByteSync         0x80
#define twoByteSync         0x88
#define threeByteSync       0x90
#define fourByteSync        0x98
#define fiveByteSync        0xA0

#define AFC_AUTOCLR         0x80
#define AFC_AUTO			0x40
#define AFC_DONE            0x10
#define AFC_CLEAR           0x02
#define AFC_START           0x01
#define FEI_START           0x20
#define FEI_DONE			0x40

static ROM_UINT8 configRegs_compat [] ROM_DATA = {
  0x25, 0x00, // Set DIOMAPPING1 to POR value
  0x28, IRQ2_FIFOOVERRUN, // Clear the FIFO
  0x2E, 0x98, // SyncConfig = sync on, sync size = 4 (Dynamically changed in code)
  0x2F, 0xAA, // SyncValue1 = 0xAA
  0x30, 0xAA, // SyncValue2 = 0xAA
  0x31, 0x2D, // SyncValue3 = 0x2D
  0x32, 0xD4, // SyncValue4 = 0xD4, 212, group
//  0x33, 0x00, // SyncValue5

  0x03, 0x02, // BitRateMsb, data rate = 49,261 khz
  0x04, 0x8A, // BitRateLsb, divider = 32 MHz / 650 == 49,230 khz

//  0x05, 0x00, // FdevMsb = 9.943 KHz
//  0x06, 0xA3, // FdevLsb = 9.943 KHz
//  0x05, 0x01, // FdevMsb = 23 KHz
//  0x06, 0x79, // FdevLsb = 23 KHz
//  0x05, 0x02, // FdevMsb = 45 KHz
//  0x06, 0xE1, // FdevLsb = 45 KHz
//  0x04, 0x03, // FdevMsb = 55 KHz 25/3/20 from Martyn
//  0x05, 0x86, // FdevLsb = 55 KHz 25/3/20 from Martyn
//  0x05, 0x05, // FdevMsb = 81 KHz
//  0x06, 0x2F, // FdevLsb = 81 KHz
  0x05, 0x05, // FdevMsb = 90 KHz
  0x06, 0xC3, // FdevLsb = 90 KHz
// 0x07, 0xD9, // FrfMsb, freq = 868.000 MHz
// 0x08, 0x00, // FrfMib, divider = 14221312
// 0x09, 0x00, // FrfLsb, step = 61.03515625
// 0x0B, 0x20, // AfcCtrl, afclowbetaon
	0x11,0x9F, // PA0 only and maximum -3dB
// Mismatching PA1 below with the RFM69x module present risks blowing a hole in the LNA
// 0x11, 0x5F, // PA1 enable, Pout = max // uncomment this for RFM69H

//  0x18, 0x02, // Manual LNA = 2 = -6dB
// More prone to restarts in noisy environment
  0x19, 0x29, // RxBw 200 KHz, DCC 16%
  0x1A, 0x29, // RxBwAFC 200 Khz, DCC 16%. Only handling initial RSSI phase, not payload!
// Less prone to restarts in noisy environment
//  0x19, 0xE2, // RxBw 125 KHz, if DCC set to 0 is more sensitive
//  0x1A, 0xF7, // RxBwAFC 2.6 Khz Only handling initial RSSI phase, not payload!
  0x1E, 0x00,	// RegAFCFEI

  0x26, 0x07, // disable clkout

  0x29, 0xA0, // RssiThresh ... -95dB

  0x37, 0x00, // PacketConfig1 = fixed, no crc, filt off
  0x38, 0x00, // PayloadLength = 0, unlimited
  0x3C, 0x8F, // FifoTresh, not empty, level 15 bytes, unused here
//  0x3C, 0x05, 	// FifoTresh, level 5 bytes, the sync length TODO This TX setting crashes remote receiver
  0x3D, 0x10, // PacketConfig2, interpkt = 1, autorxrestart off
  0x58, 0x2D, // High sensitivity mode
//  0x6F, 0x30, // TestDagc ...
//  0x71, 0x01, // AFC offset set for low modulation index systems, used if
              // AfcLowBetaOn=1. Offset = LowBetaAfcOffset x 488 Hz 
  0
};

/////////////////////////////////
#else	/////////////////////////
/////////////////////////////////
// SX1276 in FSK Mode
// How to get this working... extern PGM_P constant char radio[] PROGMEM = "SX1276 ";
#warning RF69.cpp: Building for SX1276 

#define LIBRARY_VERSION     128      // Stored in REG_SYNCVALUE6 by initRadio 
#define REG_FIFO            0x00
#define REG_OPMODE          0x01
#define DATAMODUL           0x02 
#define REG_BITRATEMSB      0x02
#define REG_BITRATELSB      0x03
#define REG_BITFDEVMSB		0x04
#define REG_BITFDEVLSB		0x05
#define REG_FRFMSB          0x06
#define REG_OSC1            0x24
#define REG_PACONFIG		0x09
#define REG_PALEVEL			0x09
#define REG_OCP             0x0B
#define REG_LNA             0x0C
#define REG_AFCFEI          0x1A
#define REG_AFCMSB          0x1B
#define REG_AFCLSB          0x1C
#define REG_FEIMSB          0x1D
#define REG_FEILSB          0x1E
//#define REG_RSSICONFIG      0x23
#define REG_RSSITHRESHOLD   0x10
#define REG_RSSIVALUE       0x11
#define REG_DIOMAPPING1     0x40
#define REG_DIOMAPPING2     0x41
#define REG_VERSION			0x42
#define REG_IRQFLAGS1       0x3E
#define REG_IRQFLAGS2       0x3F
#define REG_SYNCCONFIG      0x27
#define REG_SYNCVALUE1      0x28
#define REG_SYNCVALUE2      0x29
#define REG_SYNCVALUE3      0x2A
#define REG_SYNCVALUE4      0x2B
#define REG_SYNCVALUE5      0x2C
#define REG_SYNCVALUE6      0x2D
#define REG_SYNCVALUE7      0x2E
#define REG_SYNCVALUE8      0x2F
#define REG_SYNCGROUP       0x2B
#define REG_NODEADRS        0x33
#define REG_FIFOTHRESH      0x35
#define REG_PACKETCONFIG2   0x31
// Unsupported #define REG_AESKEY1         0x3E
//#define REG_TEMP1           0x4E
#define REG_TEMP2		      0x3C
//#define REG_TESTLNA         0x58
//#define REG_TESTPA1         0x5A
//#define REG_TESTPA2         0x5C

#define MODE_SLEEP          0x00
#define MODE_STANDBY        0x01
#define MODE_FS_TX          0x02
#define MODE_TRANSMITTER    0x03
#define MODE_FS_RX          0x04
#define MODE_RECEIVER       0x05
//#define MODE_LISTENABORT    0x20
//#define MODE_LISTENON       0x40
//#define MODE_SEQUENCER_OFF  0x80
#define MODE_MASK           0x07
//#define TESTLNA_NORMAL      0x1B
//#define TESTLNA_BOOST       0x2D
//#define TESTPA1_NORMAL      0x55
//#define TESTPA1_20DB		0x5D
//#define TESTPA2_NORMAL      0x70
//#define TESTPA2_20DB		0x7C
//#define OCP_NORMAL			0x1A
//#define OCP_20DB			0x0F

#define COURSE_TEMP_COEF      14 // starter callibration figure
//#define RF_TEMP1_MEAS_START   0x08
//#define RF_TEMP1_MEAS_RUNNING 0x04

#define IRQ1_MODEREADY      0x80
#define IRQ1_RXREADY        0x40
#define IRQ1_TXREADY		0x20
#define IRQ1_PLL            0x10
#define IRQ1_RSSI           0x08
#define IRQ1_TIMEOUT        0x04
#define IRQ1_PREAMBLE		0x02
#define IRQ1_SYNCMATCH      0x01

#define START_TX            0x80  // With 125Khz SPI a minimum
#define DELAY_TX            0x18  // 22 byte head start required, 24 to be safer 


#define IRQ2_FIFOFULL       0x80
#define IRQ2_FIFOEMPTY   	0x40
#define IRQ2_FIFOLEVEL		0x20
#define IRQ2_FIFOOVERRUN    0x10
#define IRQ2_PACKETSENT     0x08
#define IRQ2_PAYLOADREADY   0x04
#define IRQ2_CRC_OK			0x02
#define IRQ2_LOWBAT			0x01

//#define PACKET2_RESTART     0x04

#define DIO0_PACKETSENT     0x00

// FS Mode
#define DIO0_FS_UNDEF_RX    0x80
#define DIO0_FS_UNDEF_TX    0x80
// RX Mode
#define DIO4_CRCOK          0x40
//#define DIO0_PAYLOADREADY   0x40
#define DIO0_SYNCADDRESS    0x00
#define DIO0_RSSI           0x40
// TX Mode
#define DIO0_TX_UNDEFINED   0x80
#define DIO0_PACKETSENT     0x00

//#define DIO3_FIFOFULL       0x00
#define DIO4_RSSI           0xC0
#define DIO4_TempChangeLowBat	0x00
//#define DIO3_SYNCADDRESS    0x02
//#define DIO3_FIFOFULL_TX    0x00
//#define DIO3_TX_UNDEFINED   0x02

#define RcCalStart          0x0F
#define RcCalDone           0x40
#define FeiStart            0x20
#define FeiDone             0x40
#define RssiStart           0x01
#define RssiDone            0x02
#define oneByteSync         0x10
#define twoByteSync         0x11
#define threeByteSync       0x12
#define fourByteSync        0x13
//#define fiveByteSync        0xA0

#define AFC_AUTOCLR         0x80
#define AFC_AUTO			0x40
#define AFC_DONE            0x10
#define AFC_CLEAR           0x02
#define AFC_START           0x01
#define FEI_START           0x20
#define FEI_DONE			0x40

static ROM_UINT8 configRegs_compat [] ROM_DATA = {
  0x27, 0x13, // SyncConfig = sync on, sync size = 4 (Dynamically changed in code)
  0x28, 0xAA, // SyncValue1 = 0xAA
  0x29, 0xAA, // SyncValue2 = 0xAA
  0x2A, 0x2D, // SyncValue3 = 0x2D
  0x2B, 0xD4, // SyncValue4 = 0xD4, 212, group

  0x02, 0x02, // BitRateMsb, data rate = 49,261 khz
  0x03, 0x89, // BitRateLsb, divider = 32 MHz / 650 == 49,230 khz
  0x5D,	0x0A, // RegBitRateFrac  data rate = 49,259.187 kHz
  
//  0x04, 0x02, // FdevMsb = 45 KHz 28/3/20
//  0x05, 0xE1, // FdevLsb = 45 KHz 28/3/20
//  0x04, 0x03, // FdevMsb = 55 KHz 25/3/20 from Martyn
//  0x05, 0x86, // FdevLsb = 55 KHz 25/3/20 from Martyn
  0x04, 0x05, // FdevMsb = 90 KHz
  0x05, 0xC3, // FdevLsb = 90 KHz
  
  0x09, 0xDF, // RegPaConfig: PA Boost, max power
//  0x0B, 0x2B, // RegOcp

  0x0D, 0x09, // AgcAutoOn, RxTrigger:RSSI
//  0x0D, 0x0E, // AgcAutoOn
  0x0E, 0x00, // RSSI two sample smoothing - we are a star network
  
  0x10, 0xA0, // RSSI Threshold -100dB
  0x12, 0x09, // RxBw 200 KHz, DCC 16%
  0x13, 0x09, // RxBwAFC 200 Khz, DCC 16%. Only handling initial RSSI phase, not payload!

  0x1F, 0x00, // Preamble Detector Off
//  0x1F, 0xAA, 	// Preamble Detector On, 2 bytes, 10 chips
  
  0x30, 0x00, // PacketConfig1 = fixed, no crc
  0x31, 0x40, // Packet Mode
  0x32, 0x00, // Payload length unlimited
  
  0x35, 0x84, // FifoTresh, not empty
  0x36, 0x40, // Sequencer Stop
  
  0x3B, 0x01, // Auto temperature calibration disabled
  
  0x3D,	0x00, // LowBat detector disabled
  
  0x40, 0x00, // DIOMAPPING1 DIO0_PACKETSENT
  0x41, 0xC0, // DIOMAPPING2, DIO4_RSSI
  
//  0x4D, 0x87,	  // Set Pmax to +20dBm for PA_HP
  0x75, 0x29,

  0
};

#endif	/////////////////////////// SX1276

// transceiver states, these determine what to do with each interrupt
enum { TXCRC1, TXCRC2, TXDONE, TXIDLE, TXRECV, RXFIFO };

byte clearAir = 190;

namespace RF69 {
    uint32_t frf;
    uint8_t  group;
    uint8_t  node;
    uint8_t microOffset;
    uint16_t crc;
    uint8_t  rssi;
    uint8_t  rssiDelay;
    uint8_t  lastState;
    uint8_t  REGIRQFLAGS1;
    int16_t  afc;
    int16_t  fei;
    uint8_t  lna;
    uint16_t rxP;
    uint16_t txP;
    uint16_t discards;
    uint16_t unexpected;
    uint8_t  unexpectedFSM;
    uint8_t  unexpectedIRQFLAGS2;
    uint8_t  unexpectedMode;
    uint16_t byteCount;
    uint16_t underrun;
    uint8_t  present;
    uint16_t pcIntCount;
    uint8_t  pcIntBits;
    int8_t   payloadLen;
    uint16_t badLen;
    uint16_t packetShort;
    uint8_t  IRQFLAGS2;
    uint8_t  DIOMAPPING1;
    }
static volatile uint8_t lna;
static volatile uint8_t rxfill;      // number of data bytes in buffer
static volatile uint8_t rxdone;      // 
static volatile int8_t rxstate;      // current transceiver state
static volatile uint8_t packetBytes; // Count of bytes in packet
static volatile uint8_t rf69_skip;   // header bytes to skip
static volatile uint8_t rf69_fix;    // Maximum for fixed length packet
static volatile int16_t afc;
static volatile int16_t fei;
static volatile int16_t lastFEI;
static volatile uint16_t delayTXRECV;
static volatile uint16_t rst;
volatile int32_t tfr;
static volatile uint32_t previousMillis;
static volatile uint32_t noiseMillis;
static volatile uint32_t SYNCinterruptMillis;
static volatile uint16_t RssiToSync;
static volatile uint16_t rxTail;
static volatile uint8_t startRSSI;
static volatile uint8_t afcfei;
static long missedCarry = 512;
/*

0x13 RegOcp default is Current limiter active, threshold at 45+ 5*trim bits.
( i.e 45 +5*10 = 95mA).
The intent here is to run PA1 only, to keep out of the turbo boost range for the
moment. The spec sheet doesn't have a specific current draw table for PA1 alone,
but you can infer that PA1 is probably an identical TX stage to PA0, so using 
that data suggests a maximum TX current draw of ~45mA.  
So the default cap of 95mA leaves plenty of head room.
The alternative would be just to disable the feature - it is only needed in the 
"nearly flat battery" case.


*/
uint8_t RF69::control(uint8_t cmd, uint8_t val) {
//    PreventInterrupt RF69_avr_h_INT;
    return spiTransfer(cmd, val);
}

// Do not change the order or values in the array below, add new values to a max of 127
// pre-existing code is using these translate values!
// See enum in RF69_compat.h line 52 (2018/09/28)
const char translateReg[] ROM_DATA = { 
	REG_SYNCVALUE7,			//[0]
	REG_SYNCVALUE8,			//[1]
	REG_BITRATEMSB,			//[2]
	REG_BITRATELSB,			//[3]
	REG_BITFDEVMSB,			//[4]
	REG_BITFDEVLSB,			//[5]
	REG_RSSIVALUE,			//[6]
	REG_SYNCCONFIG,			//[7]
	REG_SYNCGROUP,			//[8]
	REG_PALEVEL,			//(9)
	REG_RSSITHRESHOLD		//(10)
    };
uint8_t RF69::radioIndex(uint8_t index, uint8_t val) {
	uint8_t cmd = index & 128;				// Preserve the write register bit
	cmd |= translateReg[(index & 0x7F)];	// Apply translated reg number

//    PreventInterrupt RF69_avr_h_INT;
    return spiTransfer(cmd, val);
}

static void writeReg (uint8_t addr, uint8_t value) {
    RF69::control(addr | 0x80, value);
}

static uint8_t readReg (uint8_t addr) {
    return RF69::control(addr, 0);
}

static void flushFifo () {
#if !SX1276
    while (readReg(REG_IRQFLAGS2) & (IRQ2_FIFONOTEMPTY | IRQ2_FIFOOVERRUN))
        readReg(REG_FIFO);
#else
    while (readReg(REG_IRQFLAGS2) & !(IRQ2_FIFOEMPTY | IRQ2_FIFOOVERRUN))
        readReg(REG_FIFO);
#endif
}

#if !SX1276
uint8_t setMode (uint8_t mode) {	// TODO enhance return code
    uint8_t c = 0;
	cli();	// The approach negates the *buffering* of single interrupts
	sei();	// Following instruction will not be interrupted
	uint8_t eimsk = EIMSK;
	EIMSK = 0;
	writeReg(REG_OPMODE, (mode | MODE_SEQUENCER_OFF));
        
    while ((readReg(REG_IRQFLAGS1) & IRQ1_MODEREADY) == 0) {
        c++; 
        if (c >= 254) { 
        	Serial.print("SetMode overrun"); Serial.println();
        	break;
        }
    }
// EIMSK was also re-enabled by the PreventInterrupt structure
	EIMSK = eimsk;
    return c;	// May need to beef this up since sometimes we don't appear to setmode correctly
}

#else

uint8_t setMode (uint8_t mode) {	// TODO enhance return code
//    PreventInterrupt RF69_avr_h_INT;
	uint8_t eimsk = EIMSK;
	cli();
	sei();	// Following instruction will not be interrupted
	EIMSK = 0;
    spiTransfer(REG_OPMODE | 0x80, mode);
    EIMSK = eimsk;
	return true;
}

#endif

static uint8_t initRadio (ROM_UINT8* init) {

#if SX1276
//	#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
//    bitSet(PORTH, 6);
//    delay(10);
//    bitClear(DDRH, 6);	// ATMega2560 D9 wired to radio RESET line
//    delay(100);
//    #else
	pinMode(9, OUTPUT);
    bitClear(DDRB, 1);	// D9 wired to radio RESET line
    delay(10);
	pinMode(9, INPUT);
    bitSet(PORTB, 1);
    delay(10);
//    #endif
#endif

    spiInit();
// Verify that reset actually worked
#if SX1276
	if ((readReg(REG_SYNCVALUE7) != 0x55) && (readReg(REG_SYNCVALUE8) != 0x55)) return 0;   
#else
//	if ((readReg(REG_SYNCVALUE7) != 0x00) && (readReg(REG_SYNCVALUE8) != 0x00)) return 0;
#endif    

// Validate SPI bus operation
    writeReg(REG_SYNCVALUE6, LIBRARY_VERSION);
    writeReg(REG_SYNCVALUE7, 0xAA);	// Default value is 0x55
    writeReg(REG_SYNCVALUE8, 0xAA);
    if ((readReg(REG_SYNCVALUE7) == 0xAA) && (readReg(REG_SYNCVALUE8) == 0xAA)) {

// Configure radio
        for (;;) {
            uint8_t cmd = ROM_READ_UINT8(init);
            if (cmd == 0) break;
            writeReg(cmd, ROM_READ_UINT8(init+1));
            init += 2;
        }
        writeReg(REG_RSSITHRESHOLD, rfapi.configThreshold);

		previousMillis = millis();
		rfapi.rtpMin = 0; /*65535;*/ rfapi.rtpMax = 0; rfapi.noiseTailLo = 65535;

        InitIntPin();
        
        return 1;
    }
	else return 0;
}

void RF69::setFrequency (uint32_t freq) {
    // Frequency steps are in units of (32,000,000 >> 19) = 61.03515625 Hz
    // use multiples of 64 to avoid multi-precision arithmetic, i.e. 3906.25 Hz
    // due to this, the lower 6 bits of the calculated factor will always be 0
    // this is still 4 ppm, i.e. well below the radio's 32 MHz crystal accuracy
    // 868.0 MHz = 0xD90000, 868.3 MHz = 0xD91300, 915.0 MHz = 0xE4C000 
    frf = (((freq << 2) / (32000000L >> 11)) << 6) + microOffset;
    rf69_skip = 0;    // Ensure default Jeenode RF12 operation

	// Init RF API values
    rfapi.len = sizeof rfapi;
    rfapi.noiseFloorMin = 255;
    rfapi.noiseFloorMax = 0;
}

uint8_t RF69::canSend (uint8_t clear) {
	clearAir = clear;
	if (((rxfill == 0) || (rxdone))) {
		setMode(MODE_STANDBY);
        rfapi.sendRSSI = currentRSSI();
   
        if(rfapi.sendRSSI >= clearAir) {
            rxstate = TXIDLE;
            return rfapi.sendRSSI;
        }
    } else {
    	rfapi.sendRSSI = 0;
    	rfapi.rxfill = rxfill;
    	rfapi.rxdone = rxdone;
    }
    return false;
}

bool RF69::sending () {
    return (rxstate < TXIDLE);
}

//  Note: RF12_WAKEUP returns with receiver mode disabled!
void RF69::sleep (bool off) {
    setMode(off ? MODE_SLEEP : MODE_STANDBY);
    rxstate = TXIDLE;
}

// returns raw temperature from chip
int8_t RF69::readTemperature(int8_t userCal) {
  sleep(false);        // this ensures the mode is in standby, using setMode directly had undesirable side effects.
#if !SX1276
  writeReg(REG_TEMP1, RF_TEMP1_MEAS_START);
  while ((readReg(REG_TEMP1) & RF_TEMP1_MEAS_RUNNING));
#endif
  return ~readReg(REG_TEMP2) + COURSE_TEMP_COEF + userCal; //'complement' corrects the slope, rising temp = rising val
}

uint8_t* RF69::SPI_pins() {
  return (SPI_Pins());  // {OPTIMIZE_SPI, PINCHG_IRQ, RF69_COMPAT, RFM_IRQ, 
                        //  SPI_SS, SPI_MOSI, SPI_MISO, SPI_SCK, INT_NUMBER }
}

uint8_t RF69::currentRSSI() {

  if (((rxfill == 0) || (rxdone))) {
      uint8_t storedMode = (readReg(REG_OPMODE) & MODE_MASK);
      uint8_t storeDIOM = readReg(REG_DIOMAPPING1);// Collect Interrupt triggers

      writeReg(REG_DIOMAPPING1, 0x00);      // Mask most radio interrupts
//	  setMode(MODE_FS_RX);
      writeReg(REG_RSSITHRESHOLD, 0xFF); 	// Max out threshold
      setMode(MODE_RECEIVER);   // Looses contents of FIFO and 36 spins
#if !SX1276
      rssiDelay = 0;

      writeReg(REG_RSSICONFIG, RssiStart);	// Trigger an RSSI measurement
      while (!(readReg(REG_IRQFLAGS1) & IRQ1_RSSI)) {
          rssiDelay++;
      }
#endif      

      uint8_t r = readReg(REG_RSSIVALUE);           // Collect RSSI value
      
      writeReg(REG_AFCFEI, AFC_CLEAR);
      writeReg(REG_RSSITHRESHOLD, 64);  			// Quiet down threshold
#if !SX1276      
	  setMode(MODE_SLEEP);                       	// Get out of RX mode
#else
	  setMode(MODE_FS_RX);                        	// Get out of RX mode
#endif       
      writeReg(REG_RSSITHRESHOLD, rfapi.rssiThreshold);  // Set threshold
      writeReg(REG_DIOMAPPING1, storeDIOM);         // Restore Interrupt trigger
      setMode(storedMode); 							// Restore mode
    
      return r;
      
  } else return 0;

}

// References to the RF12 driver above this line will generate compiler errors!
#include <RF69_compat.h>
#include <RF12.h>

void RF69::configure_compat () {
/*#if !SX1276
Serial.println("Built for RFM69"); 
#else    
Serial.println("Built for SX1276");      
#endif*/
    present = 0;                                    // Assume radio is absent
    if (initRadio(configRegs_compat)) {
        if (group == 0) {
            writeReg(REG_SYNCCONFIG, threeByteSync);
        } else {
        	writeReg(REG_SYNCGROUP, group);
            writeReg(REG_SYNCCONFIG, fourByteSync);
        }   

        writeReg(REG_FRFMSB, frf >> 16);
        writeReg(REG_FRFMSB+1, frf >> 8);
        writeReg(REG_FRFMSB+2, frf);
        setMode(MODE_STANDBY);
        writeReg(REG_OSC1, RcCalStart);             // Calibrate
#if !SX1276
    	while(!(readReg(REG_OSC1) & RcCalDone));    // Wait for completion
#endif
        writeReg(REG_IRQFLAGS2, IRQ2_FIFOOVERRUN);  // Clear FIFO
        rxstate = TXIDLE;

        present = 1;                                // Radio is present
#if F_CPU == 16000000UL
        rfapi.RssiToSyncLimit = JEEPACKET16;
#elif F_CPU == 8000000UL
		rfapi.RssiToSyncLimit = JEEPACKET8;
#endif
    }
}

uint8_t* recvBuf;

volatile uint32_t startRX;
volatile uint32_t ms;

uint16_t RF69::recvDone_compat (uint8_t* buf) {

    switch (rxstate) {
    
    case TXIDLE:
/*
    	if (millis() <= (ms + 2UL) ) {
    		// Brief update to millis, too many interrupts? Make IRQ time for Serial et al
			for (uint16_t tick = 0; tick < 1000; tick++) NOP;	// Interruptible delay 1ms
			rfapi.softDelay++;
    	}
*/
        rxdone = false;
        rxfill = rf69_buf[2] = 0;
        recvBuf = buf;
        startRSSI = currentRSSI();       

		rf12_drx = delayTXRECV;

#if !SX1276
		writeReg(REG_DIOMAPPING1, (DIO0_RSSI /*| DIO3_RSSI  DIO0_SYNCADDRESS*/));// Interrupt triggers
		writeReg(REG_PALEVEL, ((rfapi.txPower & 0x1F) | 0x80));	// PA1/PA2 off
        writeReg(REG_OCP, OCP_NORMAL);			// Overcurrent protection on
        writeReg(REG_TESTPA1, TESTPA1_NORMAL);	// Turn off high power 
        writeReg(REG_TESTPA2, TESTPA2_NORMAL);  // transmit
#else
		writeReg(REG_PALEVEL, (rfapi.txPower));	// PA1/PA2 off		
#endif
        
        if (rfapi.ConfigFlags & 0x80) afcfei = AFC_START;
        else afcfei = 0;
        rfapi.ConfigFlags = (rfapi.ConfigFlags | afcfei);

        rxstate = TXRECV;
//        writeReg(REG_DIOMAPPING2, DIO4_RSSI);

#if !SX1276      
	  	rfapi.setmode = setMode(MODE_RECEIVER);
#else
        rfapi.setmode = setMode(MODE_FS_RX);
#endif
        writeReg(REG_IRQFLAGS2, IRQ2_FIFOOVERRUN);  // Clear FIFO
        rfapi.setmode = setMode(MODE_RECEIVER);
        rxstate = TXRECV;
		writeReg(REG_AFCFEI, (AFC_CLEAR));
		
        rfapi.mode = readReg(REG_OPMODE);
        rfapi.irqflags1 = readReg(REG_IRQFLAGS1); 
               
        break;
        
    case TXRECV:
        if (rxdone) {
            // Move attributes & packet into rf12_buf
            rf12_rssi = rssi;
            rf12_lna = lna;
            rf12_afc = afc;
            rf12_fei = fei;
            rf12_interpacketTS = rfapi.interpacketTS;
            rf12_rxTail = rxTail;
            delayTXRECV = 0;
            rf12_sri = currentRSSI();
            
            rf12_rtp = RssiToSync; // Extra count between RSSI & Sync Match
//			if (rfapi.rtpMin > RssiToSync) rfapi.rtpMin = RssiToSync;
			if (RssiToSync > 1) rfapi.rtpMin++;	// Count none standard sync matches
			if (rfapi.rtpMax < RssiToSync) rfapi.rtpMax = RssiToSync;

            rf12_rst = rst; // Count of resets used to capture packet
            rf12_tfr = tfr; // Time to receive in microseconds
            rfapi.advisedLen = rfapi.lastLen;
            
            for (byte i = 0; i < (payloadLen + 5); i++) {
                rf12_buf[i] = rf69_buf[i];
            }     
            rf12_crc = crc;
            rxstate = TXIDLE;

            if (rf12_crc == 0) {
            	rfapi.goodCRC++;
                if (!(rf69_hdr & RF12_HDR_DST) || node == 31 ||
                    (rf69_hdr & RF12_HDR_MASK) == node) {
                    return 0; // it's for us, good packet received
                } else {
                    rfapi.discards++;
                    // Packet wasn't for us so we want too discard it silently
                    // This will happen on next entry to recvDone_compat
                    // because rxstate == TXIDLE
                }
            } else return 1;
        }
        break;
    }
    return ~0; // keep going, not done yet
}

void RF69::skip_hdr (uint8_t skip) {
    rf69_skip = skip;
}

void RF69::fix_len (uint8_t fix) {
    rf69_fix = fix;
}

uint16_t rf69_status () {
    return (rxstate << 8) | rxfill;   
}

void RF69::sendStart_compat (uint8_t hdr, const void* ptr, uint8_t len) {
#if SX1276
    setMode(MODE_STANDBY);
#endif

// Uses rf12_buf as the send buffer, rf69_buf reserved for RX
    for (int i = 0; i < len; ++i)
        rf12_data[i] = ((const uint8_t*) ptr)[i];
    rf12_hdr = hdr & RF12_HDR_DST ? hdr : (hdr & ~RF12_HDR_MASK) + node; 
    rf12_len = len;
    rxstate = - (2 + rf12_len);// Preamble/SYN1/SYN2/2D/Group are inserted by hardware
//    flushFifo();
    writeReg(REG_IRQFLAGS2, IRQ2_FIFOOVERRUN);  // Clear FIFO
/*  All packets are transmitted with a 4 byte header SYN1/SYN2/2D/Group  
    even when the group is zero                                               */
    
    // REG_SYNCGROUP must have been set to an appropriate group before this.
    crc = _crc16_update(~0, readReg(REG_SYNCGROUP));

//    if (rf12_len > 9)                       // Expedite short packet TX
//      writeReg(REG_FIFOTHRESH, DELAY_TX);   // Wait for FIFO to hit 32 bytes
//    the above code is to facilitate slow SPI bus speeds.  
#if !SX1276
	if (rfapi.txPower & 0x80) {
		rfapi.txPower = (rfapi.txPower & 0x9F);
	}
	else
	if (rfapi.txPower == 0x7F) {
      	writeReg(REG_OCP, OCP_20DB);    		// Overcurrent protection OFF!
		writeReg(REG_TESTPA1, TESTPA1_20DB);    // Turn on transmit highest power 
		writeReg(REG_TESTPA2, TESTPA2_20DB);    // cross your fingers
		// Beware the duty cycle - 1% only
    }
    #if TX_INTERRUPT
	writeReg(REG_DIOMAPPING1, (DIO0_PACKETSENT /*| DIO3_TX_UNDEFINED*/));
	#endif
#endif

    if (ptr != 0) {
    	writeReg(REG_SYNCCONFIG, fourByteSync);
    	writeReg(REG_PALEVEL, rfapi.txPower);
    } else {
    	writeReg(REG_SYNCCONFIG, 0);	// Turn off sync generation
//	    writeReg(REG_PALEVEL, 0);
    }
//Serial.println("Transmit Mode"); Serial.flush();
    setMode(MODE_TRANSMITTER);
//	delay(1);
#
//	Serial.print("OpMode=");
//	Serial.println(readReg(REG_OPMODE)); Serial.flush();
    
/*  We must begin transmission to avoid overflowing the FIFO since
    jeelib packet size can exceed FIFO size. We also want to avoid the
    transmissions of excessive preamble before payload is presented.          */
    
/* Page 54
The transmission of packet data is initiated by the Packet Handler only if the 
module is in Tx mode and the transmission condition defined by TxStartCondition 
is fulfilled. If transmission condition is not fulfilled then the packet handler
transmits a preamble sequence until the condition is met. This happens only if 
the preamble length /= 0, otherwise it transmits a zero or one until the 
condition is met to transmit the packet data.
*/    
// TODO It would be nice to pace the writes to FIFO to allow a few bytes to
// be transmitted before the FIFO gets very close to full.     
    while (rxstate < TXDONE) {
        if ((readReg(REG_IRQFLAGS2) & IRQ2_FIFOFULL) == 0) { // FIFO is 66 bytes
            uint8_t out;
            if (rxstate < 0) {
                // rf12_buf used since rf69_buf is now reserved for RX
                out = rf12_buf[3 + rf12_len + rf69_skip + rxstate];
                crc = _crc16_update(crc, out);
            } else {
                switch (rxstate) {
                    case TXCRC1: out = crc; break;
                    case TXCRC2: out = crc >> 8;
                    rf12_crc = crc; 
                    break;
                }
            }
            writeReg(REG_FIFO, out);
            ++rxstate;
        }
    }
//        writeReg(REG_FIFOTHRESH, START_TX);     // if < 32 bytes, release FIFO
                                                  // for transmission
/*  At this point packet is typically in the FIFO but not fully transmitted.
    transmission complete will be indicated by an interrupt or conditional code below:                  
*/
//	Serial.print("OpMode=");
//	Serial.println(readReg(REG_OPMODE)); Serial.flush();
//Serial.println("Buffer filled"); Serial.flush();

#if !TX_INTERRUPT
#warning RF69.cpp: TX completed using scanning       
	while (1) {
	    if (readReg(REG_IRQFLAGS2) & IRQ2_PACKETSENT) {
			writeReg(REG_PALEVEL, 0);	// Drop TX power to clear airwaves quickly	
#if SX1276
			setMode(MODE_STANDBY);
#else
			setMode(MODE_SLEEP);
			rfapi.interruptCountTX++;
          	writeReg(REG_OCP, OCP_NORMAL);			// Overcurrent protection on
          	writeReg(REG_TESTPA1, TESTPA1_NORMAL);	// Turn off high power 
          	writeReg(REG_TESTPA2, TESTPA2_NORMAL);	// transmit
#endif
          	// rxstate will be TXDONE at this point
          	txP++;
            writeReg(REG_AFCFEI, AFC_CLEAR);	// If we are in RX mode
			setMode(MODE_STANDBY);
    		writeReg(REG_PALEVEL, 0);	// Drop TX power to clear airwaves quickly	
 			writeReg(REG_IRQFLAGS2, IRQ2_FIFOOVERRUN);  // Clear FIFO
         	// Restore sync bytes configuration
          	if (group == 0) {               // Allow receiving from all groups
				writeReg(REG_SYNCCONFIG, threeByteSync);             
          	}
          	rxstate = TXIDLE;
          	Serial.print(".");
          	break;
          } else Serial.print('.');
    }
#endif

}

void RF69::interrupt_compat (uint8_t rssi_interrupt) {
/*
  This interrupt service routine retains control for far too long. However,
  the choices are limited because of the short time gap between RSSI & SyncMatch,
  being driven by recvDone and the size of the radio FIFO.
*/

/*
micros() returns the hardware timer contents (which updates continuously), 
plus a count of rollovers (ie. one rollover ever 1.024 mS). 
It can handle one rollover (the hardware remembers that) so it doesn't matter 
if you cross a rollover point, however after 1.024 mS it will not know about the 
second rollover and then will be 1.024 mS out.
*/
        // N.B. millis is not updating until IRQ_ENABLE
        ms = millis();
        if (rxstate == TXRECV) {

        	rfapi.interruptCountRX++;
        
            if (rssi_interrupt) {
            	RssiToSync = 0;
				for (volatile byte tick = 0; tick < 10; tick++) NOP;	// Kill some time waiting for sync bytes
				// volatile above changes the timing
	        	startRX = micros();	// 4µs precision
                while (true) {  // Loop for SyncMatch or Timeout
	                if (RssiToSync == 0) {
	                	writeReg(REG_AFCFEI, (afcfei | FEI_START));
	                	
						for (volatile uint16_t tick = 0; tick < 840; tick++) NOP;	// Keep the SPI quiet while FEI calculation is done.
						
            			rssi = readReg(REG_RSSIVALUE);
#if SX1276
    					lna = (readReg(REG_LNA) >> 5);
#else
    					lna = (readReg(REG_LNA) >> 3) & 7;
#endif
           				fei  = readReg(REG_FEIMSB);
        				fei  = (fei << 8) + readReg(REG_FEILSB);
        	        	afc  = readReg(REG_AFCMSB);
            			afc  = (afc << 8) | readReg(REG_AFCLSB);
            			
 						volatile uint32_t rxGap = ms - rfapi.rxLast;
 						rfapi.rxLast = ms;
		 				if (rxGap < rfapi.minGap) rfapi.minGap = rxGap;
 						if (rxGap > rfapi.maxGap) rfapi.maxGap = rxGap;          			

             			rfapi.rssi = rssi;
             			rfapi.lna = lna;
             			rfapi.fei = fei;
          				if (rssi) {
				          	/* rssi == 0 can happen above, no idea how right now
				          	only seen when using int0 versus pin change interrupt. */
			             	if (rssi < rfapi.noiseFloorMin) rfapi.noiseFloorMin = rssi;
				          	if (rssi > rfapi.noiseFloorMax) rfapi.noiseFloorMax = rssi;
			  			} else  rfapi.rssiZero++;

            		}
                    if (readReg(REG_IRQFLAGS1) & IRQ1_SYNCMATCH) {
            			tfr =  micros() - startRX;	// 4µs precision
						if (tfr < missedCarry) tfr = tfr + 1024L;
                        rfapi.syncMatch++;                     
                		noiseMillis = ms;	// Delay a reduction in sensitivity
                        break;
                    } else 
                    if (RssiToSync++ >= rfapi.RssiToSyncLimit) {
/*
						Timeout: MartynJ "Assuming you are using 5byte synch,
                        then it is just counting the bit times to find the 
                        minimum i.e. 0.02uS per bit x 6bytes is 
                        about 1mS minimum."
*/


// Provide some damping, delaying receiver restart until RSSI is higher than threshold
						for (rfapi.noiseTail = 0; rfapi.noiseTail < 16; ++rfapi.noiseTail) {
							if ( readReg(REG_RSSIVALUE) >  rfapi.configThreshold) break;
							for (volatile uint16_t tick = 0; tick < 511; tick++) NOP;	// Delay around packet tail
						}	// Wait for RSSI level to go above threshold



#if SX1276
						writeReg(REG_AFCFEI, AFC_CLEAR);                                                                
					    setMode(MODE_STANDBY);
#else
						setMode(MODE_SLEEP);
#endif
                        rxstate = TXIDLE;   // Cause a RX restart by FSM
        				// Collect RX stats per LNA
	                	rfapi.RSSIrestart++;
	                	rfapi.cumRSSI[lna] = rfapi.cumRSSI[lna] + (uint32_t)rssi; 
	                	if (fei) {
	                		rfapi.cumFEI[lna] = rfapi.cumFEI[lna] + (int32_t)fei;
	                		rfapi.cumCount[lna]++;
	                	} else rfapi.cumZeros[lna]++;
	                	rfapi.changed = true;
	                	
//	                	rfapi.cumAFC[lna] = rfapi.cumAFC[lna] + (int32_t)afc; 
//	                	rfapi.cumLNA[lna] = rfapi.cumLNA[lna] + (uint32_t)lna; 

            			if ((rfapi.rateInterval) && ((noiseMillis + rfapi.rateInterval) < ms)) {
                        	// Adjust RSSI if in noise region	                	    
							if (rfapi.rssiThreshold > rfapi.configThreshold) { 
									rfapi.rssiThreshold--;
                					noiseMillis = ms;
									previousMillis = ms;// Delay an increase in sensitivity
							}
						}
						lastFEI = fei;
                        return;
                    } // SyncMatch or Timeout 
                } //  while
            } //  RSSI 
                       
// Sync match achieved
			rxstate = RXFIFO;                       
        	IRQ_ENABLE;       // allow nested interrupts from here on  
        	                                             
        	rfapi.interpacketTS = ms;	// Value stored at time of interrupt            			
	
            volatile uint8_t stillCollecting = true;
            crc = ~0;
            packetBytes = 0;
            payloadLen = rf69_fix; // Assumed value if no Jee header used            

            if (group) { 
            	recvBuf[rxfill++] = group;
            	packetBytes++;
                crc = _crc16_update(~0, group);
            } else crc = ~0;
            
            for (;;) { // busy loop, to get each data byte as soon as it comes in 
            	if
#if SX1276
                 (!(readReg(REG_IRQFLAGS2) & 
                  (IRQ2_FIFOEMPTY /*| IRQ2_FIFOOVERRUN*/))) 
#else
                 (readReg(REG_IRQFLAGS2) & 
                  (IRQ2_FIFONOTEMPTY /*| IRQ2_FIFOOVERRUN*/)) 
#endif
                   { volatile uint8_t in = readReg(REG_FIFO);
                    
                    if ((rxfill == 2) && (rf69_skip == 0)) {
                    	rfapi.lastLen = in;
                        if (in <= RF12_MAXDATA) {  // capture and
                            payloadLen = in;       // validate length byte
                        } else {
							in = payloadLen = 10;	// Fix payload to 10!
                            badLen++;
                        }
                    }                    
                    recvBuf[rxfill++] = in;
                    packetBytes++;
                    crc = _crc16_update(crc, in);              
                    if (rxfill >= (payloadLen + (5 - rf69_skip))) {  // Trap end of payload
                        stillCollecting = false;
                        break;
                    }
                } //  if 
            } // busy loop
			for (rxTail = 0; rxTail < 16; ++rxTail) {
				if ( readReg(REG_RSSIVALUE) >  rfapi.configThreshold) break;
				for (volatile uint16_t tick = 0; tick < 511; tick++) NOP;	// Delay around packet tail
			}	// Wait for RSSI level to go above threshold
			

        	if (rxTail >= rfapi.noiseTailHi) {
        		rfapi.noiseTailHi = rxTail;
        		rfapi.noiseHiRSSI = rssi;
        	}
        	
       		if (rxTail <= rfapi.noiseTailLo) {
       			rfapi.noiseTailLo = rxTail;
       			rfapi.noiseLoRSSI = rssi;
       		}

            writeReg(REG_AFCFEI, AFC_CLEAR);
		    setMode(MODE_STANDBY);

            byteCount = rxfill;
            if (packetBytes < (5 - rf69_skip)) underrun++;            
            if (stillCollecting) packetShort++;
  
            rst = rfapi.RSSIrestart;
            rxP++;           
            rxdone = true;      // force TXRECV in RF69::recvDone_compat       
            rxstate = TXRECV;   // Restore state machine
        } else
        if (rxstate == RXFIFO) {	// Interrupted while filling FIFO ?
        	rfapi.intRXFIFO++;
        	return;					// Get back to it.

	    } else 
	    if (readReg(REG_IRQFLAGS2) & IRQ2_PACKETSENT) {
    		writeReg(REG_PALEVEL, 0);	// Drop TX power to clear airwaves quickly	
			rfapi.interruptCountTX++;
#if SX1276
			setMode(MODE_STANDBY);
#else
			setMode(MODE_SLEEP);
          	writeReg(REG_OCP, OCP_NORMAL);			// Overcurrent protection on
          	writeReg(REG_TESTPA1, TESTPA1_NORMAL);	// Turn off high power 
          	writeReg(REG_TESTPA2, TESTPA2_NORMAL);	// transmit
#endif

//    		writeReg(REG_PALEVEL, ((rfapi.txPower & 0x9F) | 0x80));	// PA1/PA2 off
          	// rxstate will be TXDONE at this point
          	txP++;
          	// Restore sync bytes configuration
          	if (group == 0) {               // Allow receiving from all groups
				writeReg(REG_SYNCCONFIG, threeByteSync);             
          	}
          	rxstate = TXIDLE;
        } else {
    		writeReg(REG_PALEVEL, 0);	// Drop TX power to clear airwaves quickly	
            // We get here when a interrupt that is not for RX/TX completion.
            // Appears related to receiving noise when the bad CRC
            // packet display is enabled using "0q".
            // Instances of an interrupt entering here while in FS mode 8 - PLL?
            // Instances of RX mode with rxstate = TXIDILE
            unexpectedFSM = rxstate; // Save Finite State Machine status
            unexpectedIRQFLAGS2 = readReg(REG_IRQFLAGS2);
            unexpectedMode = readReg(REG_OPMODE) >> 2;
            unexpected++;
//			writeReg(REG_IRQFLAGS2, IRQ2_FIFOOVERRUN);  // Clear FIFO
            rxstate = TXIDLE;   // Cause a RX restart by FSM
			setMode(MODE_SLEEP);
        }
}
