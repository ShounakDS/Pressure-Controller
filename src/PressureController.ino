// This #include statement was automatically added by the Particle IDE.
#include "MCP3424.h"


/*
 * Project DPC_Ver1
 * Description: Bluetooth Serial Read and write
 * Live value relays
 * Author:
 * Date:
 */

#include <SparkIntervalTimer.h>
#include "Adafruit_mfGFX.h"
#include "Adafruit_SSD1351_Photon.h"
#include "Particle.h"
#include "application.h"
#include "fonts.h"
#include "Variables.h"


int count = 0;


void setup() {
  // Put initialization like pinMode and begin functions here.
  ////////////////////////////////    Pin Declarations   /////////////////////////////////////////////////////
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  pinMode(relay3Pin, OUTPUT);
  pinMode(relay4Pin, OUTPUT);


  digitalWrite(relay1Pin,LOW);
  digitalWrite(relay2Pin,LOW);
  digitalWrite(relay3Pin,LOW);
  digitalWrite(relay4Pin,LOW);
 ////////////////////////////////    Int Declarations   /////////////////////////////////////////////////////

/////////////////////////////   Initializing Peripherals  //////////////////////////////////////////////////
  myTimer.begin(timerISR, 2000, hmSec);
  tft.begin();
  tft.fillRect(0,0,128,128,BLACK);
  tft.setCursor(0,0);
  tft.setTextWrap(LOW);
  Serial1.begin(9600);
  Serial.begin(9600);
  Serial.println("Test Start!!");
  initMax7219();
  clearMax();
  MCP.begin();
  MCP.configuration(0,16,0,1); // Channel 1, 16 bits resolution, one-shot mode, amplifier gain = 1

  /////////////////////////////   Initializing Variables  //////////////////////////////////////////////////

  color = GREEN;
  fgColor = WHITE;
  bgColor = BLACK;
  selColor = YELLOW;
  clockColor = YELLOW;
  displayValue = 6000;

 /////////////////////////////   EEPROM Address Read     //////////////////////////////////////////////////
  readEEPROM();
  /////////////////////////////   RTC Data    //////////////////////////////////////////////////

}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  // The core of your code will likely live here.
 serialEvent();
 /////////////////////////////// Relay 1 //////////////////////////////////////


    if( seconds > prevSeconds)
    {

        if(seconds%5 == 0)
        {
            displayValue = random(0,6000);
        }
         //displayBargraph(count);
        //displayMax(cyclicRotate(0x01),1);
        printValue = printOLED(displayValue,pvUnit,setScreen);
        MCP.newConversion(); // New conversion is initiated

        float Voltage=MCP.measure(); // Measure, note that the library waits for a complete conversion

        Serial.print("Voltage = "); // print result
        Serial.print(Voltage);
        Serial.println(" microVolt");

        checkRelayStatus(displayValue,&relay1,relay1Pin);
        checkRelayStatus(displayValue,&relay2,relay2Pin);
        checkRelayStatus(displayValue,&relay3,relay3Pin);
        checkRelayStatus(displayValue,&relay4,relay4Pin);


        //////////////////////////////////////////////////////////////////////////////////////////////////////
         prevSeconds = seconds;
    }

}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float printOLED(long printValue,int unit,uint8_t screen)
{

  float tempVar;
  if(unit > 3)
  {
    unit = 0;
  }
  if(unit < 0)
  {
    unit = 0;
  }
  if(screen == 0)
  {
      return screen1(printValue,unit);
  }
   if(screen == 1)
  {
      return screen2(printValue,unit);
  }
   if(screen == 2)
  {
      return screen3(printValue,unit);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Screen 1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float screen1(long value,int unit)
{
    float var;
    tft.setFont(ARIAL_36);
    if(unit == 0)
    {
        var = value;
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(30,30,hunds,fgColor,bgColor,1);
        tft.drawChar(60,30,tens,fgColor,bgColor,1);
        drawPoint(90,59,4,fgColor);
        tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for BAR
    }
    if(unit == 1)
    {
        var = (value*1.4503827640391);
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(33,30,hunds,fgColor,bgColor,1);
        tft.drawChar(66,30,tens,fgColor,bgColor,1);
        tft.drawChar(99,30,ones,fgColor,bgColor,1);///Print for PSI
    }
    if(unit == 2)
    {
        var = (value * 1.01972);
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(30,30,hunds,fgColor,bgColor,1);
        tft.drawChar(60,30,tens,fgColor,bgColor,1);
        drawPoint(90,59,4,fgColor);
        tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for Kg/cm2
    }
    if(unit == 3)
    {
        var = value * 0.1;
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(30,30,hunds,fgColor,bgColor,1);
        tft.drawChar(60,30,tens,fgColor,bgColor,1);
        drawPoint(90,59,4,fgColor);
        tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for MPa
    }
   tft.drawFastHLine(0,75,128,selColor);
   tft.drawFastHLine(0,76,128,selColor);
   tft.setFont(ARIAL_8);
   tft.setCursor(50,85);
   tft.setTextColor(WHITE);
   tft.print(unitNames[unit]);
   tft.fillRect(0,0,128,20,printSector(var));
   return var/10;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Screen 2
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float screen2(long value,int unit)
{
    float var;
    tft.setFont(ARIAL_36);
    if(unit == 0)
    {
        var = value;
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(30,30,hunds,fgColor,bgColor,1);
        tft.drawChar(60,30,tens,fgColor,bgColor,1);
        drawPoint(90,59,4,fgColor);
        tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for BAR
    }
    if(unit == 1)
    {
        var = (value*1.4503827640391);
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(33,30,hunds,fgColor,bgColor,1);
        tft.drawChar(66,30,tens,fgColor,bgColor,1);
        tft.drawChar(99,30,ones,fgColor,bgColor,1);///Print for PSI
    }
    if(unit == 2)
    {
        var = (value * 1.01972);
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(30,30,hunds,fgColor,bgColor,1);
        tft.drawChar(60,30,tens,fgColor,bgColor,1);
        drawPoint(90,59,4,fgColor);
        tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for Kg/cm2
    }
    if(unit == 3)
    {
        var = value * 0.1;
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(30,30,hunds,fgColor,bgColor,1);
        tft.drawChar(60,30,tens,fgColor,bgColor,1);
        drawPoint(90,59,4,fgColor);
        tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for MPa
    }
    tft.drawFastHLine(0,75,128,selColor);
    tft.drawFastHLine(0,76,128,selColor);
    tft.setFont(ARIAL_12);
    tft.setCursor(0,0);
    tft.setTextColor(WHITE);
    tft.print(unitNames[unit]);
    tft.fillRect(60,0,128,20,printSector(var));
    printRelayStatus();
    return var/10;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Screen 3
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float screen3(long value,int unit)
{
    float var;
    tft.setFont(ARIAL_8_N);
    fgColor = WHITE;
    tft.setTextColor(WHITE);
    if(unit == 0)
    {
        var = value;
        hex2bcd((int)var);
        tft.drawChar(0,0,thous,fgColor,bgColor,1);
        tft.drawChar(13,0,hunds,fgColor,bgColor,1);
        tft.drawChar(26,0,tens,fgColor,bgColor,1);
        tft.drawChar(39,0,'.',fgColor,bgColor,1);
        tft.drawChar(52,0,ones,fgColor,bgColor,1);///Print for BAR
    }
    if(unit == 1)
    {
        var = (value*1.4503827640391);
        hex2bcd((int)var);
        tft.drawChar(0,0,thous,fgColor,bgColor,1);
        tft.drawChar(13,0,hunds,fgColor,bgColor,1);
        tft.drawChar(26,0,tens,fgColor,bgColor,1);
        tft.drawChar(39,0,ones,fgColor,bgColor,1);///Print for PSI
    }
    if(unit == 2)
    {
        var = (value * 1.01972);
        hex2bcd((int)var);
        tft.drawChar(0,0,thous,fgColor,bgColor,1);
        tft.drawChar(13,0,hunds,fgColor,bgColor,1);
        tft.drawChar(26,0,tens,fgColor,bgColor,1);
        tft.drawChar(39,0,'.',fgColor,bgColor,1);
        tft.drawChar(52,0,ones,fgColor,bgColor,1);
    }
    if(unit == 3)
    {
        var = value * 0.1;
        hex2bcd((int)var);
        tft.drawChar(0,0,thous,fgColor,bgColor,1);
        tft.drawChar(13,0,hunds,fgColor,bgColor,1);
        tft.drawChar(26,0,tens,fgColor,bgColor,1);
        tft.drawChar(39,0,'.',fgColor,bgColor,1);
        tft.drawChar(52,0,ones,fgColor,bgColor,1);;///Print for MPa
    }
   ////// Temperature

  tft.setFont(ARIAL_12);
  tft.setCursor(0, 30);
  tft.print(unitNames[unit]);
  tft.setCursor(0, 110);
  tft.print("R1");

  ////// Relay Upper
  hex2bcd(relay1.upperSet);
  tft.drawChar(0,58,thous,fgColor,bgColor,1);
  tft.drawChar(13,58,hunds,fgColor,bgColor,1);
  tft.drawChar(26,58,tens,fgColor,bgColor,1);
  tft.drawChar(39,58,'.',fgColor,bgColor,1);
  tft.drawChar(45,58,ones,fgColor,bgColor,1);
  ////// Relay Upper delay
  hex2bcd(relay1.upperDelay);
  tft.drawChar(84,58,tens,fgColor,bgColor,1);
  tft.drawChar(97,58,ones,fgColor,bgColor,1);
  tft.drawChar(115,58,'s',fgColor,bgColor,1);
  ////// Lower
  hex2bcd(relay1.lowerSet);
  tft.drawChar(0,88,thous,fgColor,bgColor,1);
  tft.drawChar(13,88,hunds,fgColor,bgColor,1);
  tft.drawChar(26,88,tens,fgColor,bgColor,1);
  tft.drawChar(39,88,'.',fgColor,bgColor,1);
  tft.drawChar(45,88,ones,fgColor,bgColor,1);
  ////// Lower delay
  hex2bcd(relay1.lowerDelay);
  tft.drawChar(84,88,tens,fgColor,bgColor,1);
  tft.drawChar(97,88,ones,fgColor,bgColor,1);
  tft.drawChar(115,88,'s',fgColor,bgColor,1);

  tft.setFont(GLCDFONT);
  tft.drawFastVLine(74, 0, 40, GRAY);
  tft.drawFastVLine(73, 0, 40, GRAY);

  tft.drawChar(62,54,0x18,RED,bgColor,2);
  tft.drawChar(62,86,0x19,RED,bgColor,2);
  tft.fillCircle(116, 116, 9, WHITE);
  tft.drawBitmap(109, 110, heart, 16, 16,RED);
  tft.fillRect(77,0,128,40,printSector(var));
  return var/10;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Screen 4
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float screen4()
{


}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Print Color According to Sector
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int printSector(unsigned int value)
{
    unsigned int color;
    if((value > 0) && (value <= 1500))
    {
        return GREEN;
    }
    if((value > 1500) && (value <= 3000))
    {
        return YELLOW;
    }
    if((value > 3000) && (value <= 4500))
    {
        return ORANGE;
    }
    if((value > 4500) && (value <= 6000))
    {
       return RED;
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Print Status of the Relays on the Front Screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void printRelayStatus()
{
    int color;
    tft.setFont(ARIAL_12);
    if(relay1.upperFlag)
    {
       color = RED;
    }
    else
    {
       color = GREEN;
    }
    tft.setCursor(5,85);
    tft.setTextColor(color);
    tft.print(" R1 ");
    if(relay2.upperFlag)
    {
       color = RED;
    }
    else
    {
        color = GREEN;
    }
    tft.setCursor(35,85);
    tft.setTextColor(color);
    tft.print(" R2 ");
    if(relay3.upperFlag)
    {
       color = RED;
    }
    else
    {
        color = GREEN;
    }
    tft.setCursor(65,85);
    tft.setTextColor(color);
    tft.print(" R3 ");
    if(relay4.upperFlag)
    {
       color = RED;
    }
    else
    {
        color = GREEN;
    }
    tft.setCursor(95,85);
    tft.setTextColor(color);
    tft.print(" R4 ");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Hex to BCD Convertor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void hex2bcd(int num)
{
    int a1,a2,a3,a4;
    tenthous = (num/10000)+48;
    a1 = num % 10000;
    thous = (a1/1000) + 48;
    a2 = a1%1000;
    hunds = (a2/100)+ 48;
    a3 = a2%100;
    tens = (a3/10) + 48;
    ones = (a3%10) + 48;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Draw Point
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void drawPoint(int x,int y, int pointSize,int pointColor)
{
 int i,j;
 for(i = 0; i<=pointSize; i++)
 {
   for(j = 0;j<=pointSize;j++)
   {
      tft.drawPixel(x+i,y+j,pointColor);
   }
 }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// LED Bar Graph Display Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initMax7219()
 {
  SPI.begin(CS_MAX); // wake up the SPI bus.
  SPI.setBitOrder(MSBFIRST);
  commandMax(0x00,0x09); // BCD mode for digit decoding
  commandMax(0x02,0x0A); // Segment luminosity intensity
  commandMax(0x07,0x0B);//Display refresh
  commandMax(0x01,0x0C);  // Turn on the display
  commandMax(0x00,0x0F); // No test
 }
////////////////////////////////////////////////////////////////////
void displayMax(int num,int pos)
{

    digitalWrite(CS_MAX,LOW);    // SELECT MAX
    SPI.transfer(pos);
    SPI.transfer(num);
    digitalWrite(CS_MAX,HIGH);        // DESELECT MAX
    delay(10);
 }
////////////////////////////////////////////////////////////////////
 void commandMax(int num,int pos)
{

    digitalWrite(CS_MAX,LOW);    // SELECT MAX
    SPI.transfer(pos);
    SPI.transfer(num);
    digitalWrite(CS_MAX,HIGH);        // DESELECT MAX
    delay(10);
 }
////////////////////////////////////////////////////////////////////
 void clearMax()
 {
    displayMax(0x00,1);
    displayMax(0x00,2);
    displayMax(0x00,3);
    displayMax(0x00,4);
    displayMax(0x00,5);
    displayMax(0x00,6);
    displayMax(0x00,7);
    displayMax(0x00,8);

 }
////////////////////////////////////////////////////////////////////
void displayBargraph(int value)
{
  int temp;
  int row,col,rem,i,j;
  byte newByte;
  initMax7219();
  row = value / 8;
  rem = value % 8;

  if((row == 6) && (rem == 0))
  {
    displayMax(0x9F,7);
  }
  else if((row == 6) && (rem == 1))
  {
    displayMax(0xDF,7);
  }

   else if((row == 6) && (rem == 2))
  {
    displayMax(0xFF,7);
  }
  else
  {
    displayMax(0x9F,7);
  }
  displayMax(0xFF,8);
   if(value == 0)
   {
      row = 0;
      col = 0;
      rem = 0;
    }
  for(i=0;i<row;i++)
  {
     displayMax(0xff,i+1);
  }
   if(value > 0)
  {
  temp  = (0x01<<rem)-1;
  }
  else
  {
    temp = 0;
  }
  if(row == 6)
  {
    temp = ((0x01<<rem)-1) |0xFC;
  }
  for (i=0,j=7;i<8;)
  { // adjust values for nibbles
    bitWrite(newByte, i++, bitRead(temp, j--)); // adjust bitRead offset for nibbles
  }
  displayMax(cyclicRotate(newByte),row+1);
  for(i= row+1;i<6;i++)
  {
    displayMax(0x00,i+1);
  }



}

////////////////////////////////////////////////////////////////////
byte cyclicRotate(byte tempByte)
{
    byte tempByte1;
    if(bitRead(tempByte,0))
    {
        tempByte1 = tempByte >> 1 | 0x80;
    }
    else
    {
        tempByte1 = tempByte >> 1 ;
    }


    /*for(int i =0;i<6;i++)
    {
        bitWrite(tempByte1,6-i,bitRead(tempByte,i));
    }*/
    return tempByte1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Read EEprom
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readEEPROM()
{
    //mode = EEPROM.read(ADD_MODE);
    pvUnit = EEPROM.read(ADD_UNIT);
    setScreen = EEPROM.read(ADD_SCREEN);

    EEPROM.get(ADD_SEC1_UPPERSET,sector1.upperSet);
    EEPROM.get(ADD_SEC1_LOWERSET,sector1.lowerSet);
    EEPROM.get(ADD_SEC1_COLOR,sector1.color);
    EEPROM.get(ADD_SEC2_UPPERSET,sector2.upperSet);
    EEPROM.get(ADD_SEC2_LOWERSET,sector2.lowerSet);
    EEPROM.get(ADD_SEC2_COLOR,sector2.color);
    EEPROM.get(ADD_SEC3_UPPERSET,sector3.upperSet);
    EEPROM.get(ADD_SEC3_LOWERSET,sector3.lowerSet);
    EEPROM.get(ADD_SEC3_COLOR,sector3.color);
    EEPROM.get(ADD_SEC4_UPPERSET,sector4.upperSet);
    EEPROM.get(ADD_SEC4_LOWERSET,sector4.lowerSet);
    EEPROM.get(ADD_SEC4_COLOR,sector4.color);

    EEPROM.get(ADD_RLY1_UPPERSET,relay1.upperSet);
    EEPROM.get(ADD_RLY1_UPPERDEL,relay1.upperDelay);
    EEPROM.get(ADD_RLY1_LOWERSET,relay1.lowerSet);
    EEPROM.get(ADD_RLY1_LOWERDEL,relay1.lowerDelay);
    EEPROM.get(ADD_RLY2_UPPERSET,relay2.upperSet);
    EEPROM.get(ADD_RLY2_UPPERDEL,relay2.upperDelay);
    EEPROM.get(ADD_RLY2_LOWERSET,relay2.lowerSet);
    EEPROM.get(ADD_RLY2_LOWERDEL,relay2.lowerDelay);
    EEPROM.get(ADD_RLY3_UPPERSET,relay3.upperSet);
    EEPROM.get(ADD_RLY3_UPPERDEL,relay3.upperDelay);
    EEPROM.get(ADD_RLY3_LOWERSET,relay3.lowerSet);
    EEPROM.get(ADD_RLY3_LOWERDEL,relay3.lowerDelay);
    EEPROM.get(ADD_RLY4_UPPERSET,relay4.upperSet);
    EEPROM.get(ADD_RLY4_UPPERDEL,relay4.upperDelay);
    EEPROM.get(ADD_RLY4_LOWERSET,relay4.lowerSet);
    EEPROM.get(ADD_RLY4_LOWERDEL,relay4.lowerDelay);


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Read EEprom
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void checkRelayStatus(long value,Relay* tempRelay,int relayPin)
{
    if( value > tempRelay->upperSet)
        {
          tempRelay->upperCount++;
              if(tempRelay->upperCount > tempRelay->upperDelay)
              {
                tempRelay->upperCount = tempRelay->upperDelay + 1;
                tempRelay->upperFlag = HIGH;
                digitalWrite(relayPin,HIGH);
              }

        }
        if( (value < tempRelay->lowerSet) && (tempRelay->upperFlag == HIGH) )
        {
          tempRelay->lowerCount++;
              if(tempRelay->lowerCount > tempRelay->lowerDelay)
              {
                tempRelay->lowerCount = 0;
                tempRelay->upperCount = 0;
                tempRelay->upperFlag = LOW;
                digitalWrite(relayPin,LOW);
              }
        }


}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Serial Event detects character P from the serial Port
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void serialEvent()
{
      String inString,tempString;
      char tempChar;
      int tempInt;
      int index[10];
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      if(Serial1.available())
      {
        inString = Serial1.readStringUntil('~');
        //tft.println(inString);
      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      if(inString == "Orion")
      {
        Serial1.println("#OK~");
      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      if(inString == "Setting")
      {

        Serial1.println("KSNO=KU12345,KBNO=170120,TSNO=2072756,TBNO=161225,NEWDEVICE=1,PASSSUPER=abc123,PASSADMIN=abc123,PASSUSER=abc123,RANGEL=0000,RANGEH=6000,VALUE=INT,SENSOR=1,OUTPUT=3,WIFISSID=skynet,WIFIPASS=password,MODE=1,DIA=,DNST=,UNIT=2,SCREEN=2,ZONE1L=ml,ZONE1H=INT,ZONE1CL=INT,ZONE2L=INT,ZONE2H=INT,ZONE2CL=INT,ZONE3L=INT,ZONE3H=INT,ZONE3CL=INT,ZONE4L=INT,ZONE4H=INT,ZONE4CL=INT,R1LS=INT,R1LD=INT,R1US=INT,R1UD=INT,R1MR=INT,R2LS=INT,R2LD=INT,R2US=INT,R2UD=INT,R2MR=INT,R3LS=INT,R3LD=INT,R3US=INT,R3UD=INT,R3MR=INT,R4LS=INT,R4LD=INT,R4US=INT,R4UD=INT,R4MR=INT,OUTCALZ=INT,OUTCALS=INT,SCANTIME=INT,DATALOGSTS=BOOLEAN,DATALOGTIME=INT,DISPSCROLLTIME=INT,DISPSCROLLSTS=BOOLEAN,CALDUE=STRING,TAGNO=INT,MODEL=STRING,FIRMWARE=STRING,~");

      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Live
      if(inString.charAt(0) == 'L' )
      {
        Serial1.print("#");
        Serial1.print(printValue);
        Serial1.println("~");
        Serial.print("#");
        Serial.print(printValue);
        Serial.println("~");


      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Mode
      if(inString.charAt(0) == 'M' )
      {
        tempChar = inString.charAt(2);
        mode = tempChar - 48;
        //EEPROM.put(ADD_MODE,mode);

        tempChar = inString.charAt(4);
        pvUnit = tempChar - 48;
        EEPROM.write(ADD_UNIT,pvUnit);

        tempChar = inString.charAt(6);
        setScreen = tempChar - 48;
        EEPROM.write(ADD_SCREEN ,setScreen);
        tft.fillRect(0,0,128,128,BLACK);

      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Switch
      if(inString.charAt(0) == 'R' )
      {
        int tempRelay[4];
        int length = inString.length();

        index[0] = inString.indexOf(',',3);
        tempRelay[0] = inString.substring(3, index[0]).toInt() * 10;
        tempString = inString.substring(index[0]+1, length);

        index[0] = tempString.indexOf(',');
        tempRelay[1] = tempString.substring(0, index[0]).toInt() * 10;
        tempString = tempString.substring(index[0]+1,length);


        index[0] = tempString.indexOf(',');
        tempRelay[2] = tempString.substring(0, index[0]).toInt();
        tempString = tempString.substring(index[0]+1,length);


        index[0] = tempString.indexOf(',');
        tempRelay[3] = tempString.substring(0, index[0]).toInt();

        /////////////////////////////////////   RELAY 1   /////////////////////////////////////////////////////
        if(inString.charAt(1) == '1')
        {
            relay1.upperSet = tempRelay[0];
            relay1.lowerSet = tempRelay[1];
            relay1.upperDelay = tempRelay[2];
            relay1.lowerDelay = tempRelay[3];
            EEPROM.put(ADD_RLY1_UPPERSET,relay1.upperSet);
            EEPROM.put(ADD_RLY1_LOWERSET,relay1.lowerSet);
            EEPROM.put(ADD_RLY1_LOWERDEL,relay1.lowerDelay);
            EEPROM.put(ADD_RLY1_UPPERDEL,relay1.upperDelay);
        }
        /////////////////////////////////////   RELAY 2   /////////////////////////////////////////////////////
        if(inString.charAt(1) == '2')
        {
            relay2.upperSet = tempRelay[0];
            relay2.lowerSet = tempRelay[1];
            relay2.upperDelay = tempRelay[2];
            relay2.lowerDelay = tempRelay[3];
            EEPROM.put(ADD_RLY2_UPPERSET,relay2.upperSet);
            EEPROM.put(ADD_RLY2_LOWERSET,relay2.lowerSet);
            EEPROM.put(ADD_RLY2_LOWERDEL,relay2.lowerDelay);
            EEPROM.put(ADD_RLY2_UPPERDEL,relay2.upperDelay);
        }
        /////////////////////////////////////   RELAY 3   /////////////////////////////////////////////////////
        if(inString.charAt(1) == '3')
        {
            relay3.upperSet = tempRelay[0];
            relay3.lowerSet = tempRelay[1];
            relay3.upperDelay = tempRelay[2];
            relay3.lowerDelay = tempRelay[3];
            EEPROM.put(ADD_RLY3_UPPERSET,relay3.upperSet);
            EEPROM.put(ADD_RLY3_LOWERSET,relay3.lowerSet);
            EEPROM.put(ADD_RLY3_LOWERDEL,relay3.lowerDelay);
            EEPROM.put(ADD_RLY3_UPPERDEL,relay3.upperDelay);
        }
        /////////////////////////////////////   RELAY 4   /////////////////////////////////////////////////////
        if(inString.charAt(1) == '4')
        {
            relay4.upperSet = tempRelay[0];
            relay4.lowerSet = tempRelay[1];
            relay4.upperDelay = tempRelay[2];
            relay4.lowerDelay = tempRelay[3];
            EEPROM.put(ADD_RLY4_UPPERSET,relay4.upperSet);
            EEPROM.put(ADD_RLY4_LOWERSET,relay4.lowerSet);
            EEPROM.put(ADD_RLY4_LOWERDEL,relay4.lowerDelay);
            EEPROM.put(ADD_RLY4_UPPERDEL,relay4.upperDelay);

        }

      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Sector
      if(inString.charAt(0) == 'G' )
      {
        int tempRelay[4];
        int length = inString.length();

        index[0] = inString.indexOf(',',3);
        tempRelay[0] = inString.substring(3, index[0]).toInt() * 10;
        tempString = inString.substring(index[0]+1, length);

        index[0] = tempString.indexOf(',');
        tempRelay[1] = tempString.substring(0, index[0]).toInt() * 10;
        tempString = tempString.substring(index[0]+1,length);

        index[0] = tempString.indexOf(',');
        tempRelay[2] = tempString.substring(0, index[0]).toInt();

        /////////////////////////////////////   SECTOR 1   /////////////////////////////////////////////////////
        if(inString.charAt(1) == '1')
        {
            sector1.upperSet = tempRelay[0];
            sector1.lowerSet = tempRelay[1];
            sector1.color = tempRelay[2];
            EEPROM.put(ADD_SEC1_UPPERSET,sector1.upperSet);
            EEPROM.put(ADD_SEC1_LOWERSET,sector1.lowerSet);
            EEPROM.put(ADD_SEC1_COLOR,sector1.color);
        }
        /////////////////////////////////////   SECTOR 2   /////////////////////////////////////////////////////
        if(inString.charAt(1) == '2')
        {
            sector2.upperSet = tempRelay[0];
            sector2.lowerSet = tempRelay[1];
            sector2.color = tempRelay[2];
            EEPROM.put(ADD_SEC2_UPPERSET,sector2.upperSet);
            EEPROM.put(ADD_SEC2_LOWERSET,sector2.lowerSet);
            EEPROM.put(ADD_SEC2_COLOR,sector2.color);
        }
        /////////////////////////////////////   SECTOR 3   /////////////////////////////////////////////////////
        if(inString.charAt(1) == '3')
        {
            sector3.upperSet = tempRelay[0];
            sector3.lowerSet = tempRelay[1];
            sector3.color = tempRelay[2];
            EEPROM.put(ADD_SEC3_UPPERSET,sector3.upperSet);
            EEPROM.put(ADD_SEC3_LOWERSET,sector3.lowerSet);
            EEPROM.put(ADD_SEC3_COLOR,sector3.color);
        }
        /////////////////////////////////////  SECTOR 4   /////////////////////////////////////////////////////
        if(inString.charAt(1) == '4')
        {
            sector4.upperSet = tempRelay[0];
            sector4.lowerSet = tempRelay[1];
            sector4.color = tempRelay[2];
            EEPROM.put(ADD_SEC4_UPPERSET,sector4.upperSet);
            EEPROM.put(ADD_SEC4_LOWERSET,sector4.lowerSet);
            EEPROM.put(ADD_SEC4_COLOR,sector4.color);
        }

      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Transmitter
      if(inString.charAt(0) == 'T' )
      {


      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Data Logger
      if(inString.charAt(0) == 'D' )
      {


      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Calibration
      if(inString.charAt(0) == 'C' )
      {


      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      //Device Settings
      if(inString.charAt(0) == 'P' )
      {


      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      //WiFI
      if(inString.charAt(0) == 'W' )
      {
        int length = inString.length();
        index[0] = inString.indexOf(',',2);
        WifiSSID = inString.substring(2, index[0]);
        tempString = inString.substring(index[0]+1,length);

        index[1] = tempString.indexOf(',');
        WifiPASS = tempString.substring(0, index[1]);


        tempString = tempString.substring(index[1]+1,length);

        //wifiOn = tempString.toInt();

        //tft.println(WifiSSID);
        //tft.println(WifiPASS);

        WiFi.setCredentials(WifiSSID,WifiPASS);

      }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Interrupts
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void timerISR(void)
{
 seconds++;
 if(seconds > 100)
 {
     seconds = 0;
     prevSeconds = 0;
     count = 0;
 }
}
