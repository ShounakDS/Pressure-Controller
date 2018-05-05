#include<SdFat.h>

// You can use any 5 pins; see note below about hardware vs software SPI
#define cs   DAC
#define sclk A3
#define mosi A5
#define rst  D5
#define dc   D6
#define CS_MAX D3

#define relay1Pin  P1S0
#define relay2Pin  P1S1
#define relay3Pin  P1S2
#define relay4Pin  P1S3

// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF
#define ORANGE          0xFB20
#define GRAY            0x8410
#define DARKBROWN       0x9448
#define FAINTGRAY       0xA534
#define DARKGREEN       0x0620

#define LOW             0
#define HIGH            1
////////////////////////// EEPROM Address Definitions ///////////////////////
#define ADD_MODE             0
#define ADD_UNIT             1
#define ADD_NEWDEVICE        2
#define ADD_RANGEH           3
#define ADD_RANGEL           5
#define ADD_DIAMTR           7
#define ADD_DENSITY          8

#define ADD_SEC1_LOWERSET     9
#define ADD_SEC1_UPPERSET     11
#define ADD_SEC1_COLOR        13

#define ADD_SEC2_LOWERSET     14
#define ADD_SEC2_UPPERSET     16
#define ADD_SEC2_COLOR        18

#define ADD_SEC3_LOWERSET     19
#define ADD_SEC3_UPPERSET     21
#define ADD_SEC3_COLOR        23

#define ADD_SEC4_LOWERSET     24
#define ADD_SEC4_UPPERSET     26
#define ADD_SEC4_COLOR        30

#define ADD_RLY1_LOWERSET     31
#define ADD_RLY1_LOWERDEL     33
#define ADD_RLY1_UPPERSET     35
#define ADD_RLY1_UPPERDEL     37
#define ADD_RLY1_MANRST       39

#define ADD_RLY2_LOWERSET     40
#define ADD_RLY2_LOWERDEL     42
#define ADD_RLY2_UPPERSET     44
#define ADD_RLY2_UPPERDEL     46
#define ADD_RLY2_MANRST       48

#define ADD_RLY3_LOWERSET     49
#define ADD_RLY3_LOWERDEL     51
#define ADD_RLY3_UPPERSET     53
#define ADD_RLY3_UPPERDEL     55
#define ADD_RLY3_MANRST       57

#define ADD_RLY4_LOWERSET     58
#define ADD_RLY4_LOWERDEL     60
#define ADD_RLY4_UPPERSET     62
#define ADD_RLY4_UPPERDEL     64
#define ADD_RLY4_MANRST       66

#define ADD_OUTCALZ           67
#define ADD_OUTCALS           69

#define ADD_SCAN_TIME         70
#define ADD_DATALOG_STS       71
#define ADD_DATALOG_TIME        72

#define ADD_TAGNO           73

#define ADD_SENSOR    74
#define ADD_OUTPUT    75
#define ADD_DISPSCROLL_TIME   76
#define ADD_DISPSCROLL_STS    77
#define ADD_SCREEN   78
#define WIFI_STATUS 79



#define ADD_KUSRNO      200
#define ADD_KUBATCHNO   220
#define ADD_TCSSRNO      240
#define ADD_TCSBATCHNO   260

#define ADD_MODEL      280
#define ADD_FIRMWARE   300

#define ADD_PASS_SU      320
#define ADD_PASS_ADMIN   340
#define ADD_PASS_USER    360
#define ADD_KUBATCHNO   380


#define ADD_ADC_CAL_0       600
#define ADD_ADC_CAL_1       604
#define ADD_ADC_CAL_2       608
#define ADD_ADC_CAL_3       612
#define ADD_ADC_CAL_4       616

#define ADD_DISP_CAL_0       620
#define ADD_DISP_CAL_1       624
#define ADD_DISP_CAL_2       628
#define ADD_DISP_CAL_3       632
#define ADD_DISP_CAL_4       636

#define CALPOINTS   2

#define BTAT A2

#define AT      1
#define NORMAL  0


#define SPI_CONFIGURATION 0

SdFat sd;
const uint8_t chipSelect = D2;
File myFile;


#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))




// Option 1: Hardware SPI - uses some analog pins, but much faster
Adafruit_SSD1351 tft = Adafruit_SSD1351(cs, dc, rst);
// Create IntervalTimer objects
IntervalTimer myTimer;



typedef struct Relay {
  String relayName;
  uint16_t lowerSet;
  uint16_t lowerDelay;
  uint16_t upperSet;
  uint16_t upperDelay;
  uint8_t manRst;
  uint8_t lowerCount = 0,upperCount = 0;
  boolean upperFlag,lowerFlag;

};

typedef struct Sector {
 uint16_t lowerSet;
 uint16_t upperSet;
 uint8_t color;
};


////////////////       Float Variables      ////////////////////////////////

float p = 3.1415926;

////////////////      Pin Declarations     ////////////////////////////////

////////////////    Volatile Boolean Flags ////////////////////////////////

volatile boolean menuState = LOW;
volatile boolean menuFlagL = LOW;
volatile boolean menuFlagH = LOW;
volatile boolean sp1FlagH = LOW;
volatile boolean sp1FlagL = LOW;
volatile boolean nextinc010 = LOW;


//////////////      Volatile Temporary Variables   //////////////////////////

volatile int inc0,inc1,inc2,inc3,inc4,pos;
uint16_t  tempVar[10];
volatile unsigned int seconds,prevSeconds;
uint8_t inc,next,tempNext;


//////////////  Variables to be stored in EEPROM Memory  ///////////////////

uint8_t pvUnit = 0,mode = 0;
Relay relay1,relay2,relay3,relay4;
Sector sector1,sector2,sector3,sector4;
uint8_t dataLogStatus = 0,dispScroll  = 1;
uint8_t  scanTime = 1,scrollTime = 10;
boolean  heartFlag = 0;


uint8_t DEBUG_LIVE=0;
uint8_t DEBUG_BLUETOOTH=0;

///////////////// Read Only Variables //////////////////////////////////////
char serialNo[10],batchNo[4],sensorType,outputType;

//////////////////////   Other Variables    /////////////////////////////////
uint16_t color,fgColor,bgColor,selColor,clockColor;
uint8_t tenthous,thous,hunds,ones,tens;
uint8_t i,j,k;
uint8_t dispChange = 0,displayMode = 0;
char charPos1[8] = {0,15,45,60,90,105};
char charPos2[8] = {0,15,30,45,60,75,90,105};
char charPos3[8] = {20,35,50,80};
uint8_t colon = 0;
uint8_t dispNow = 0;
uint16_t sensorValue = 0;  // variable to store the value coming from the sensor
float printValue;
volatile unsigned int setRelay,setDelay;
long  adcValue;
float sensorInput,displayValue;
long calDisp[5],calAdc[5];
long rangeLow = 0,rangeHigh = 600,zero ,span;
double vin;
float f1;
int outputValue,turnDownRatio;
uint8_t setScreen = 0;
String WifiSSID,WifiPASS;
uint8_t wifiStatus = 0;
uint8_t rtcSec,rtcPrevSec;
int count = 0;
uint16_t colorValues[4] = {0x07E0,0xFFE0,0xFB20, 0xF800}; //GREEN,YELLOW,ORANGE,RED
//////////////////////  String Declarations     /////////////////////////////////

char* menuNames[] = { "U n i t s           ",
                     "R e l a y s          ",
                     "C l o c k           ",
                     "D a t a   L o g        ",
                     "S e t u p          "
                   };
char* relayNames[] = { "R e l a y   1         ",
                      "R e l a y   2         ",
                      "R e l a y   3         ",
                      "R e l a y   4         "
                    };
char* timeNames[] = { "T i m e            ",
                     "D a t e            ",
                   };
char* datalogNames[] = { "D a t a   L o g        ",
                        "S c a n   T i m e       ",
                        "T r a n s f e r        "
                      };

char* calNames[] = { "N e x t   C a l   D u e  ",
                    "C a l i b e r a t e     "
                  };

char* relay1Names[] = { "L o w e r        ",
                       "U p p e r        ",
                       "M a n   R s t      "
                     };
char* relay2Names[] = { "S e t          ",
                       "D e l a y        ",
                     };
char* positionNames[] = { "O f f",
                       "O n   "
                     };
char* autoNames[] = { "M a n u a l",
                      "A u t o"
                       };
char* unitNames[] = { "bar",
                      "psi",
                      "Kg/cm2",
                      "MPa",
                     };
char* setupNames[] = { "I n f o               ",
                     "D i s p l a y   S c r o l l               ",
                     "S c r o l l   T i m e     ",
                     "H e l p      "};
char dotColon[2] = { ':',' '};
char* outputNames[] = { "4-20mA",
                       "0-5V"
                       };
char* modeNames[] = { "Pressure ",
                     "Force",
                     "Level",
                     "Differential Pressure",
                     "Flow",
                     "Temperature"
                    };
char* colorNames[] = { "Green ",
                     "Yellow",
                     "Orange",
                     "Red",
                    };

char* wlanSec[] = { "Unsecured",
                     "WEP",
                     "WPA",
                     "WPA2",
                    };
char* wlanCipher[] = { "AES",
                     "TKIP",
                    };
char* fileType[] = { "DATA_",
                     "SETT_",
                     "CONF_"
                     "LOGR_"
                    };
const unsigned char heart [] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x60, 0x1E, 0xF0, 0x3F, 0xF8, 0x7F, 0xFC, 0x7F, 0xFC,
0x7F, 0xFC, 0x3F, 0xF8, 0x1F, 0xF0, 0x0F, 0xE0, 0x07, 0xC0, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00
};

const unsigned char smallHeart [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x70, 0x3E, 0xF8,
  0x3F, 0xF8, 0x1F, 0xF0, 0x0F, 0xE0, 0x07, 0xC0, 0x03, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
  };

const unsigned char bluetooth [] PROGMEM = {
0x01, 0xC0, 0x01, 0xE0, 0x01, 0x30, 0x11, 0x18, 0x19, 0x18, 0x0D, 0x30, 0x07, 0x60, 0x03, 0xC0,
0x03, 0xC0, 0x07, 0x60, 0x0D, 0x30, 0x19, 0x18, 0x11, 0x18, 0x01, 0x30, 0x01, 0xE0, 0x01, 0xC0
};
const unsigned char pulse [] PROGMEM = {
0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x90, 0x01, 0xD0, 0x0B, 0x50,
0x0A, 0x78, 0x1E, 0x68, 0xF6, 0x6F, 0x00, 0x60, 0x00, 0x60, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40
};
const unsigned char wifi [] PROGMEM = {/*
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x38, 0x1C, 0x63, 0xC6, 0xCF, 0xF1, 0x9E, 0x7D,
0xF8, 0x1F, 0x63, 0xC6, 0x3F, 0xFC, 0x1E, 0x78, 0x0C, 0x30, 0x07, 0xE0, 0x03, 0xC0, 0x01, 0x80*/
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFC, 0x00, 0x0F, 0xFF, 0x00, 0x1C, 0x03, 0x80, 0x30,
0x00, 0xC0, 0x23, 0xFC, 0x40, 0x06, 0x06, 0x00, 0x0C, 0x03, 0x00, 0x08, 0x61, 0x00, 0x01, 0xF8,
0x00, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x60, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
