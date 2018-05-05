/*
 * Project Digital Pressure Controller Cum Tranmitter
 * Description:
 *
 * Author: Shounak Sharangpani
 * Date:
 */
#include <SparkIntervalTimer.h>
#include "Adafruit_mfGFX.h"
#include "Adafruit_SSD1351_Photon.h"
#include "Particle.h"
#include "application.h"
#include "fonts.h"
#include "Variables.h"
#include "SdFat.h"

/// System Mode functions
SYSTEM_MODE(MANUAL);
// Startup Functions
STARTUP(bluetoothMode(NORMAL));
STARTUP(WiFi.selectAntenna(ANT_INTERNAL)); // selects the u.FL antenna
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
// reset the system after 60 seconds if the application is unresponsive
ApplicationWatchdog wd(60000, System.reset);


/// Set up Function
void setup() {
//  readEEPROM();
  if(wifiStatus){
    WiFi.on();
    WiFi.connect();
    Particle.connect();
  }
  else{
    WiFi.off();
  }
  //resetLog();
  System.on(all_events, handle_all_the_events);
  Time.zone(+5.5);
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
  Serial.begin(115200);
  initMax7219();
  clearMax();
  Wire.begin();
  initMCP3424(0x68,0,3,0);    /// add, sr,pga,ch
/////////////////////////////   Initializing Variables  ////////////////////////////////////////
  color = GREEN;
  fgColor = WHITE;
  bgColor = BLACK;
  selColor = YELLOW;
  clockColor = YELLOW;
  //displayValue = 6000;
  //calAdc[0] = -900;
  //EEPROM.put(ADD_ADC_CAL_0,calAdc[0]);
  //calAdc[1] = 60800;
  //EEPROM.put(ADD_ADC_CAL_1,calAdc[1]);
  //calDisp[0] = 0;
  //calDisp[1] = 100;
  //EEPROM.put(ADD_DISP_CAL_0,calDisp[0]);
  //EEPROM.put(ADD_DISP_CAL_1,calDisp[1]);

  relay1.relayName = "R1";
  relay2.relayName = "R2";
  relay3.relayName = "R3";
  relay4.relayName = "R4";
/////////////////////////////   Serial Debug Initializing  //////////////////////////////////////
 serialDebugInit();
 /////////////////////////////   EEPROM Address Read     ////////////////////////////////////////

  /////////////////////////////   Wifi Status    ////////////////////////////////////////////////

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// loop() runs over and over again, as quickly as it can execute.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop(){
  // The core of your code will likely live here.
  if(wifiStatus){
    tft.drawBitmap(80, 110, wifi, 24, 24,WHITE);
    if(Particle.connected){
      Particle.process();
    }
  }
  else{
      tft.drawBitmap(80, 110, wifi, 24, 24,BLACK);
  }
  Particle.process();
  bluetoothEvent();
  debugEvent();
  rtcSec = Time.second() ;
 /////////////////////////////// Relay 1 //////////////////////////////////////
    if( (rtcSec- rtcPrevSec) == scanTime){
      if(dataLogStatus){
        Datalog();
      }
      rtcPrevSec = rtcSec;
    }
    if( seconds > prevSeconds){
    //displayBargraph(count);
    //displayMax(cyclicRotate(0x01),1);
    initMCP3424(0x68,3,1,0);    /// add, sr,pga,ch
    adcValue = MCP3421getLong(0x68,3); /// add sr
    displayValue = mapf(adcValue,calAdc[0],calAdc[1],calDisp[0],calDisp[1]);
    //displayValue = mapf(adcValue,-900,66600,0,100);
    printValue = printOLED((long)displayValue,pvUnit,setScreen);
    Particle.publish("Pressure",String(displayValue/10,1));
    checkRelayStatus(displayValue,&relay1,relay1Pin);
    checkRelayStatus(displayValue,&relay2,relay2Pin);
    checkRelayStatus(displayValue,&relay3,relay3Pin);
    checkRelayStatus(displayValue,&relay4,relay4Pin);
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    if(DEBUG_LIVE){
      Serial.print(modeNames[mode]);
      Serial.print("\t");
      Serial.print(unitNames[pvUnit]);
      Serial.print("\t\t");
      Serial.print(adcValue);
      Serial.print("\t\t\t");
      Serial.print(printValue);
      Serial.print("\t\t");
      Serial.print(relay1.upperFlag);
      Serial.print("\t\t");
      Serial.print(relay2.upperFlag);
      Serial.print("\t\t");
      Serial.print(relay3.upperFlag);
      Serial.print("\t\t");
      Serial.println(relay4.upperFlag);
    } ///// END DEBUG_LIVE
    prevSeconds = seconds;
    }
 wd.checkin(); // resets the AWDT count
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print all the things on OLED
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float printOLED(long printValue,int unit,uint8_t screen){
  float tempVar;
  if(unit > 3){
    unit = 0;
  }
  if(unit < 0){
    unit = 0;
  }
  if(screen == 0){
      return screen1(printValue,unit);
  }
   if(screen == 1){
      return screen2(printValue,unit);
  }
   if(screen == 2){
      return screen3(printValue,unit);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Screen 1
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float screen1(long value,int unit){
    float var;
    tft.setFont(ARIAL_36);
    if(unit == 0){
        var = value;
        hex2bcd((int)value);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(30,30,hunds,fgColor,bgColor,1);
        tft.drawChar(60,30,tens,fgColor,bgColor,1);
        drawPoint(90,59,4,fgColor);
        tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for BAR
    }
    if(unit == 1){
        var = (value*1.4503827640391);
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(33,30,hunds,fgColor,bgColor,1);
        tft.drawChar(66,30,tens,fgColor,bgColor,1);
        tft.drawChar(99,30,ones,fgColor,bgColor,1);///Print for PSI
    }
    if(unit == 2){
        var = (value * 1.01972);
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(30,30,hunds,fgColor,bgColor,1);
        tft.drawChar(60,30,tens,fgColor,bgColor,1);
        drawPoint(90,59,4,fgColor);
        tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for Kg/cm2
    }
    if(unit == 3){
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
   heartFunction();
   return var/10;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Screen 2
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float screen2(long value,int unit){
    float var;
    tft.setFont(ARIAL_36);
    if(unit == 0){
        var = value;
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(30,30,hunds,fgColor,bgColor,1);
        tft.drawChar(60,30,tens,fgColor,bgColor,1);
        drawPoint(90,59,4,fgColor);
        tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for BAR
    }
    if(unit == 1){
        var = (value*1.4503827640391);
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(33,30,hunds,fgColor,bgColor,1);
        tft.drawChar(66,30,tens,fgColor,bgColor,1);
        tft.drawChar(99,30,ones,fgColor,bgColor,1);///Print for PSI
    }
    if(unit == 2){
        var = (value * 1.01972);
        hex2bcd((int)var);
        tft.drawChar(0,30,thous,fgColor,bgColor,1);
        tft.drawChar(30,30,hunds,fgColor,bgColor,1);
        tft.drawChar(60,30,tens,fgColor,bgColor,1);
        drawPoint(90,59,4,fgColor);
        tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for Kg/cm2
    }
    if(unit == 3){
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
    heartFunction();
    printRelayStatus();
    return var/10;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Screen 3
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float screen3(long value,int unit){
    float var;
    tft.setFont(ARIAL_8_N);
    fgColor = WHITE;
    tft.setTextColor(WHITE);
    if(unit == 0){
        var = value;
        hex2bcd((int)var);
        tft.drawChar(0,0,thous,fgColor,bgColor,1);
        tft.drawChar(13,0,hunds,fgColor,bgColor,1);
        tft.drawChar(26,0,tens,fgColor,bgColor,1);
        tft.drawChar(39,0,'.',fgColor,bgColor,1);
        tft.drawChar(52,0,ones,fgColor,bgColor,1);///Print for BAR
    }
    if(unit == 1){
        var = (value*1.4503827640391);
        hex2bcd((int)var);
        tft.drawChar(0,0,thous,fgColor,bgColor,1);
        tft.drawChar(13,0,hunds,fgColor,bgColor,1);
        tft.drawChar(26,0,tens,fgColor,bgColor,1);
        tft.drawChar(39,0,ones,fgColor,bgColor,1);///Print for PSI
    }
    if(unit == 2){
        var = (value * 1.01972);
        hex2bcd((int)var);
        tft.drawChar(0,0,thous,fgColor,bgColor,1);
        tft.drawChar(13,0,hunds,fgColor,bgColor,1);
        tft.drawChar(26,0,tens,fgColor,bgColor,1);
        tft.drawChar(39,0,'.',fgColor,bgColor,1);
        tft.drawChar(52,0,ones,fgColor,bgColor,1);
    }
    if(unit == 3){
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
  displayScroll();
  heartFunction();
  tft.fillRect(77,0,128,40,printSector(var));
  return var/10;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Screen 4
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void screen4(long value,int unit){
  float var;
  tft.setFont(ARIAL_36);
  if(unit == 0){
      var = value;
      hex2bcd((int)value);
      tft.drawChar(0,30,thous,fgColor,bgColor,1);
      tft.drawChar(30,30,hunds,fgColor,bgColor,1);
      tft.drawChar(60,30,tens,fgColor,bgColor,1);
      drawPoint(90,59,4,fgColor);
      tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for BAR
  }
  if(unit == 1){
      var = (value*1.4503827640391);
      hex2bcd((int)var);
      tft.drawChar(0,30,thous,fgColor,bgColor,1);
      tft.drawChar(33,30,hunds,fgColor,bgColor,1);
      tft.drawChar(66,30,tens,fgColor,bgColor,1);
      tft.drawChar(99,30,ones,fgColor,bgColor,1);///Print for PSI
  }
  if(unit == 2){
      var = (value * 1.01972);
      hex2bcd((int)var);
      tft.drawChar(0,30,thous,fgColor,bgColor,1);
      tft.drawChar(30,30,hunds,fgColor,bgColor,1);
      tft.drawChar(60,30,tens,fgColor,bgColor,1);
      drawPoint(90,59,4,fgColor);
      tft.drawChar(100,30,ones,fgColor,bgColor,1);///Print for Kg/cm2
  }
  if(unit == 3){
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
 heartFunction();

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Print Color According to Sector
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int printSector(unsigned int value){
    unsigned int color;
    if((value >=sector1.lowerSet) && (value <= sector1.upperSet)){
        return colorValues[sector1.color];
    }
    else if((value > sector2.lowerSet) && (value <= sector2.upperSet)){
        return colorValues[sector2.color];
    }
    else if((value > sector3.lowerSet) && (value <= sector3.upperSet)){
        return colorValues[sector3.color];
    }
    else if((value > sector4.lowerSet) && (value <= sector4.upperSet)){
       return colorValues[sector4.color];
    }
    else{
      return RED;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Print Status of the Relays on the Front Screen
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void printRelayStatus(){
  int color;
  tft.setFont(ARIAL_12);
  if(relay1.upperFlag){
     color = RED;
  }
  else{
     color = GREEN;
  }
  tft.setCursor(5,85);
  tft.setTextColor(color);
  tft.print(" R1 ");
  if(relay2.upperFlag){
     color = RED;
  }
  else{
      color = GREEN;
  }
  tft.setCursor(35,85);
  tft.setTextColor(color);
  tft.print(" R2 ");
  if(relay3.upperFlag){
     color = RED;
  }
  else{
      color = GREEN;
  }
  tft.setCursor(65,85);
  tft.setTextColor(color);
  tft.print(" R3 ");
  if(relay4.upperFlag){
     color = RED;
  }
  else{
      color = GREEN;
  }
  tft.setCursor(95,85);
  tft.setTextColor(color);
  tft.print(" R4 ");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Hex to BCD Convertor
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void hex2bcd(int num){
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
void drawPoint(int x,int y, int pointSize,int pointColor){
 int i,j;
 for(i = 0; i<=pointSize; i++){
   for(j = 0;j<=pointSize;j++){
      tft.drawPixel(x+i,y+j,pointColor);
   }
 }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// LED Bar Graph Display Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initMax7219(){
  SPI.begin(CS_MAX); // wake up the SPI bus.
  SPI.setBitOrder(MSBFIRST);
  commandMax(0x00,0x09); // BCD mode for digit decoding
  commandMax(0x02,0x0A); // Segment luminosity intensity
  commandMax(0x07,0x0B);//Display refresh
  commandMax(0x01,0x0C);  // Turn on the display
  commandMax(0x00,0x0F); // No test
 }
////////////////////////////////////////////////////////////////////
void displayMax(int num,int pos){
  digitalWrite(CS_MAX,LOW);    // SELECT MAX
  SPI.transfer(pos);
  SPI.transfer(num);
  digitalWrite(CS_MAX,HIGH);        // DESELECT MAX
  delay(10);
}
////////////////////////////////////////////////////////////////////
 void commandMax(int num,int pos){
   digitalWrite(CS_MAX,LOW);    // SELECT MAX
   SPI.transfer(pos);
   SPI.transfer(num);
   digitalWrite(CS_MAX,HIGH);        // DESELECT MAX
   delay(10);
}
////////////////////////////////////////////////////////////////////
 void clearMax(){
   displayMax(0x00,1);
   displayMax(0x00,2);
   displayMax(0x00,3);
   displayMax(0x00,4);
   displayMax(0x00,5);
   displayMax(0x00,6);
   displayMax(0x00,7);
   displayMax(0x00,8);
 }
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 /// Display the bar graph on the LED's
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displayBargraph(int value){
  int temp;
  int row,col,rem,i,j;
  byte newByte;
  initMax7219();
  row = value / 8;
  rem = value % 8;
  if((row == 6) && (rem == 0)){
    displayMax(0x9F,7);
  }
  else if((row == 6) && (rem == 1)){
    displayMax(0xDF,7);
  }
   else if((row == 6) && (rem == 2)){
    displayMax(0xFF,7);
  }
  else{
    displayMax(0x9F,7);
  }
  displayMax(0xFF,8);
  if(value == 0){
    row = 0;
    col = 0;
    rem = 0;
  }
  for(i=0;i<row;i++){
    displayMax(0xff,i+1);
  }
  if(value > 0){
  temp  = (0x01<<rem)-1;
  }
  else{
    temp = 0;
  }
  if(row == 6){
    temp = ((0x01<<rem)-1) |0xFC;
  }
  for (i=0,j=7;i<8;){ // adjust values for nibbles
    bitWrite(newByte, i++, bitRead(temp, j--)); // adjust bitRead offset for nibbles
  }
  displayMax(cyclicRotate(newByte),row+1);
  for(i= row+1;i<6;i++){
    displayMax(0x00,i+1);
  }
}
////////////////////////////////////////////////////////////////////
byte cyclicRotate(byte tempByte){
    byte tempByte1;
    if(bitRead(tempByte,0))    {
        tempByte1 = tempByte >> 1 | 0x80;
    }
    else{
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
void readEEPROM(){
    //mode = EEPROM.read(ADD_MODE);
    pvUnit = EEPROM.read(ADD_UNIT);
    setScreen = EEPROM.read(ADD_SCREEN);
    EEPROM.get(ADD_RANGEH,rangeHigh);
    EEPROM.get(ADD_RANGEL,rangeLow);
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
    dataLogStatus = EEPROM.read(ADD_DATALOG_STS);
    scanTime = EEPROM.read(ADD_DATALOG_TIME);
    wifiStatus = EEPROM.read(WIFI_STATUS);
    EEPROM.get(ADD_ADC_CAL_0,calAdc[0]);
    EEPROM.get(ADD_ADC_CAL_1,calAdc[1]);
    EEPROM.get(ADD_DISP_CAL_0,calDisp[0]);
    EEPROM.get(ADD_DISP_CAL_1,calDisp[1]);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Check the status of the Relays
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void checkRelayStatus(long value,Relay* tempRelay,int relayPin){
  if( value >= tempRelay->upperSet){
    tempRelay->upperCount++;
    if(tempRelay->upperCount > tempRelay->upperDelay){
      tempRelay->upperCount = tempRelay->upperDelay + 1;
      tempRelay->upperFlag = HIGH;
      digitalWrite(relayPin,HIGH);
    }
  }
  if( (value <= tempRelay->lowerSet) && (tempRelay->upperFlag == HIGH)){
    tempRelay->lowerCount++;
    if(tempRelay->lowerCount > tempRelay->lowerDelay){
      tempRelay->lowerCount = 0;
      tempRelay->upperCount = 0;
      tempRelay->upperFlag = LOW;
      digitalWrite(relayPin,LOW);
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Functions for ADC Init and get value
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initMCP3424(uint8_t addr, uint8_t sr, uint8_t pga,uint8_t ch){
  uint8_t confWrite = 0;
  //sr = sr & 3;
  //pga = pga & 3;
  //ch = ch & 3;
  confWrite = confWrite | (ch << 5);
  confWrite = confWrite | (sr << 2);
  confWrite = confWrite | pga;
  bitWrite (confWrite,7,1);   // RDY
  bitWrite (confWrite,4,1);   // O/C 1
  Wire.beginTransmission(addr);
  Wire.write(confWrite);
  Wire.endTransmission();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
signed long MCP3421getLong(uint8_t addr,uint8_t sr){
  uint8_t b2,b3,b4,confwrite,confRead;
  signed long l1;
  if(sr < 3){
    Wire.requestFrom(addr, 3);
    b2 = Wire.read();
    b3 = Wire.read();
    confRead = Wire.read();
    Wire.endTransmission();
    l1= (256* b2 )+ b3;
  }
  else{
    Wire.requestFrom(addr, 4);
    b2 = Wire.read();
    b3 = Wire.read();
    b4 = Wire.read();
    confRead = Wire.read();
    Wire.endTransmission();
    l1= (long) b3*256;
    l1= l1 + b4;
    l1= l1+0x10000 * b2;
    if (b2 > 0x10){
      l1= l1 + 0xFF000000;
    }
  }
   return(l1);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Arduino map Function but as FLoat
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float mapf(float x, float in_min, float in_max, float out_min, float out_max){
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Bluetooth mode Selection
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void bluetoothMode(uint8_t modeSel){
  pinMode(BTAT,OUTPUT);
  if(modeSel == 0){
      digitalWrite(BTAT,LOW);
  }
  else{
     digitalWrite(BTAT,HIGH);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Initial screen for USB Debug
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void serialDebugInit(){
  Serial.println("");
  Serial.println(" _  __         _    _  _____ _______ _    _ ____  _    _    _    _ _______     ______   _____ ");
  Serial.println("| |/ /    /\  | |  | |/ ____|__   __| |  | |  _ \| |  | |  | |  | |  __ \ \   / / __ \ / ____|");
  Serial.println("| ' /    /  \ | |  | | (___    | |  | |  | | |_) | |__| |  | |  | | |  | \ \_/ / |  | | |  __ ");
  Serial.println("|  <    / /\ \| |  | |\___ \   | |  | |  | |  _ <|  __  |  | |  | | |  | |\   /| |  | | | |_ |");
  Serial.println("| . \  / ____ \ |__| |____) |  | |  | |__| | |_) | |  | |  | |__| | |__| | | | | |__| | |__| |");
  Serial.println("|_|\_\/_/    \_\____/|_____/   |_|   \____/|____/|_|  |_|   \____/|_____/  |_|  \____/ \_____|");
  Serial.println("");
  Serial.println("");
  Serial.println("Welcome to the Smart Pressure Transmitter Debug Console !!!!");
  Serial.println("");
  Serial.println("Press r to check the RELAY Settings.  ");
  Serial.println("Press s to check the SECTOR Settings.  ");
  Serial.println("Press t to check the TIME Settings.  ");
  Serial.println("Press d to check the DATALOG Settings.  ");
  Serial.println("Press b to turn on/off Bluetooth commands debugging.  ");
  Serial.println("Press l to turn on/off Live Data debugging.  ");
  Serial.println("Press w to check Wifi Settings.  ");
  Serial.println("Press a to see device parameters.  ");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Bluetooth event checks for incoming data via bluettoth for settings
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void bluetoothEvent(){
  String tempString,inString,string1,string2,string3,string4;
  char tempChar;
  int tempInt;
  int index[10];
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  // check incoming data
  if(Serial1.available()){
    inString = Serial1.readStringUntil('~'); /// ~ in end of line character
    /// If debugging is On send data to serial Port
    if(DEBUG_BLUETOOTH){
        Serial.println(inString);
      }
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  // If the mobile device sends Orion send back OK
  if(inString == "Orion"){
    Serial1.println("#OK~");
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  // If the mobile device sends Settings send all the Settings to mobile
  if(inString == "Setting"){
    sendDataBluetooth();
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Live
  if(inString.charAt(0) == 'L' ){
    Serial1.print("#");
    Serial1.print((float)displayValue/10);
    Serial1.println("~");
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Mode Mode, Unit, Image, Range High, Range low
  if(inString.charAt(0) == 'M' ){
    int length = inString.length();
    tempChar = inString.charAt(2) - 48;
    if(mode != tempChar){
      settingsChangeLog("Mode",String(mode),String((int)tempChar));
    }
    //EEPROM.put(ADD_MODE,mode);
    // Unit
    tempChar = inString.charAt(4) - 48;
    if(pvUnit != tempChar){
      settingsChangeLog("Unit",unitNames[pvUnit],unitNames[tempChar]);
    }
    pvUnit = tempChar;
    EEPROM.write(ADD_UNIT,pvUnit);
    // Screen Type
    tempChar = inString.charAt(6) - 48;
    if(setScreen != tempChar){
      settingsChangeLog("Screen",String(setScreen),String((int)tempChar));
    }
    setScreen = tempChar;
    EEPROM.write(ADD_SCREEN ,setScreen);
    // Range High
    index[0] = inString.indexOf(',',7);
    tempInt = inString.substring(3, index[0]).toInt() * 10;
    if(rangeHigh != tempInt){
      settingsChangeLog("Range Span Value ",String(rangeHigh),String(tempInt));
    }
    rangeHigh = tempInt;
    EEPROM.put(ADD_RANGEH,rangeHigh);
    // Range Low
    tempString = inString.substring(index[0]+1, length);
    index[0] = tempString.indexOf(',');
    tempInt = tempString.substring(0, index[0]).toInt() * 10;
    if(rangeLow != tempInt){
      settingsChangeLog("Range Zero Value ",String(rangeLow),String(tempInt));
    }
    rangeLow = tempInt;
    EEPROM.put(ADD_RANGEL,rangeLow);
    tft.fillRect(0,0,128,128,BLACK);
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Relay Settings
  if(inString.charAt(0) == 'R'){
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
    if(inString.charAt(1) == '1'){
      if(relay1.upperSet != tempRelay[0]){
        settingsChangeLog("Relay 1 Upper Set Value ",String(relay1.upperSet),String(tempRelay[0]));
      }
      relay1.upperSet = tempRelay[0];
      EEPROM.put(ADD_RLY1_UPPERSET,relay1.upperSet);
      if(relay1.lowerSet != tempRelay[1]){
        settingsChangeLog("Relay 1 Lower Set Value ",String(relay1.lowerSet),String(tempRelay[1]));
      }
      relay1.lowerSet = tempRelay[1];
      EEPROM.put(ADD_RLY1_LOWERSET,relay1.lowerSet);
      if(relay1.upperDelay != tempRelay[2]){
        settingsChangeLog("Relay 1 Upper Delay Value ",String(relay1.upperDelay),String(tempRelay[2]));
      }
      relay1.upperDelay = tempRelay[2];
      EEPROM.put(ADD_RLY1_UPPERDEL,relay1.upperDelay);
      if(relay1.lowerDelay != tempRelay[3]){
        settingsChangeLog("Relay 1 Lower Delay Value ",String(relay1.lowerDelay),String(tempRelay[3]));
      }
      relay1.lowerDelay = tempRelay[3];
      EEPROM.put(ADD_RLY1_LOWERDEL,relay1.lowerDelay);
    }
    /////////////////////////////////////   RELAY 2   /////////////////////////////////////////////////////
    if(inString.charAt(1) == '2'){
      if(relay2.upperSet != tempRelay[0]){
        settingsChangeLog("Relay 2 Upper Set Value ",String(relay2.upperSet),String(tempRelay[0]));
      }
        relay2.upperSet = tempRelay[0];
        EEPROM.put(ADD_RLY2_UPPERSET,relay2.upperSet);
      if(relay2.lowerSet != tempRelay[1]){
        settingsChangeLog("Relay 2 Lower Set Value ",String(relay2.lowerSet),String(tempRelay[1]));
      }
      relay2.lowerSet = tempRelay[1];
      EEPROM.put(ADD_RLY2_LOWERSET,relay2.lowerSet);
      if(relay2.upperDelay != tempRelay[2]){
        settingsChangeLog("Relay 2 Upper Delay Value ",String(relay2.upperDelay),String(tempRelay[2]));
      }
      relay2.upperDelay = tempRelay[2];
      EEPROM.put(ADD_RLY2_UPPERDEL,relay2.upperDelay);
      if(relay2.lowerDelay != tempRelay[3]){
        settingsChangeLog("Relay 2 Lower Delay Value ",String(relay2.lowerDelay),String(tempRelay[3]));
      }
      relay2.lowerDelay = tempRelay[3];
      EEPROM.put(ADD_RLY2_LOWERDEL,relay2.lowerDelay);
    }
    /////////////////////////////////////   RELAY 3   /////////////////////////////////////////////////////
    if(inString.charAt(1) == '3'){
      if(relay3.upperSet != tempRelay[0]){
        settingsChangeLog("Relay 3 Upper Set Value ",String(relay3.upperSet),String(tempRelay[0]));
      }
      relay3.upperSet = tempRelay[0];
      EEPROM.put(ADD_RLY3_UPPERSET,relay3.upperSet);
      if(relay3.lowerSet != tempRelay[1]){
        settingsChangeLog("Relay 3 Lower Set Value ",String(relay3.lowerSet),String(tempRelay[1]));
      }
      relay3.lowerSet = tempRelay[1];
      EEPROM.put(ADD_RLY3_LOWERSET,relay3.lowerSet);
      if(relay3.upperDelay != tempRelay[2]){
        settingsChangeLog("Relay 3 Upper Delay Value ",String(relay3.upperDelay),String(tempRelay[2]));
      }
      relay3.upperDelay = tempRelay[2];
      EEPROM.put(ADD_RLY3_UPPERDEL,relay3.upperDelay);
      if(relay3.lowerDelay != tempRelay[3]){
        settingsChangeLog("Relay 3 Lower Delay Value ",String(relay3.lowerDelay),String(tempRelay[3]));
      }
      relay3.lowerDelay = tempRelay[3];
      EEPROM.put(ADD_RLY3_LOWERDEL,relay3.lowerDelay);
    }
    /////////////////////////////////////   RELAY 4   /////////////////////////////////////////////////////
    if(inString.charAt(1) == '4'){
      if(relay4.upperSet != tempRelay[0]){
        settingsChangeLog("Relay 4 Upper Set Value ",String(relay4.upperSet),String(tempRelay[0]));
      }
      relay4.upperSet = tempRelay[0];
      EEPROM.put(ADD_RLY4_UPPERSET,relay4.upperSet);
      if(relay4.lowerSet != tempRelay[1]){
        settingsChangeLog("Relay 4 Lower Set Value ",String(relay4.lowerSet),String(tempRelay[1]));
      }
      relay4.lowerSet = tempRelay[1];
      EEPROM.put(ADD_RLY4_LOWERSET,relay4.lowerSet);
      if(relay4.upperDelay != tempRelay[2]){
        settingsChangeLog("Relay 4 Upper Delay Value ",String(relay4.upperDelay),String(tempRelay[2]));
      }
      relay4.upperDelay = tempRelay[2];
      EEPROM.put(ADD_RLY4_UPPERDEL,relay4.upperDelay);
      if(relay4.lowerDelay != tempRelay[3]){
        settingsChangeLog("Relay 4 Lower Delay Value ",String(relay4.lowerDelay),String(tempRelay[3]));
      }
      relay4.lowerDelay = tempRelay[3];
      EEPROM.put(ADD_RLY4_LOWERDEL,relay4.lowerDelay);
    }
  }
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sector Settings
  if(inString.charAt(0) == 'G'){
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
    if(inString.charAt(1) == '1'){
      if(sector1.upperSet != tempRelay[0]){
        settingsChangeLog("Sector 1 Upper Set Value ",String(sector1.upperSet),String(tempRelay[0]));
      }
      sector1.upperSet = tempRelay[0];
      EEPROM.put(ADD_SEC1_UPPERSET,sector1.upperSet);
      if(sector1.lowerSet != tempRelay[1]){
        settingsChangeLog("Sector 1 Lower Set Value ",String(sector1.lowerSet),String(tempRelay[1]));
      }
      sector1.lowerSet = tempRelay[1];
      EEPROM.put(ADD_SEC1_LOWERSET,sector1.lowerSet);
      if(sector1.color != tempRelay[2]){
        settingsChangeLog("Sector 1 Color Value ",colorNames[sector1.color],colorNames[tempRelay[2]]);
      }
      sector1.color = tempRelay[2];
      EEPROM.put(ADD_SEC1_COLOR,sector1.color);
    }
    /////////////////////////////////////   SECTOR 2   /////////////////////////////////////////////////////
    if(inString.charAt(1) == '2'){
      if(sector2.upperSet != tempRelay[0]){
        settingsChangeLog("Sector 2 Upper Set Value ",String(sector2.upperSet),String(tempRelay[0]));
      }
      sector2.upperSet = tempRelay[0];
      EEPROM.put(ADD_SEC2_UPPERSET,sector2.upperSet);
      if(sector2.lowerSet != tempRelay[1]){
        settingsChangeLog("Sector 2 Lower Set Value ",String(sector2.lowerSet),String(tempRelay[1]));
      }
      sector2.lowerSet = tempRelay[1];
      EEPROM.put(ADD_SEC2_LOWERSET,sector2.lowerSet);
      if(sector2.color != tempRelay[2]){
        settingsChangeLog("Sector 2 Color Value ",colorNames[sector2.color],colorNames[tempRelay[2]]);
      }
      sector2.color = tempRelay[2];
      EEPROM.put(ADD_SEC2_COLOR,sector2.color);
    }
    /////////////////////////////////////   SECTOR 3   /////////////////////////////////////////////////////
    if(inString.charAt(1) == '3'){
      if(sector3.upperSet != tempRelay[0]){
        settingsChangeLog("Sector 3 Upper Set Value ",String(sector3.upperSet),String(tempRelay[0]));
      }
      sector3.upperSet = tempRelay[0];
      EEPROM.put(ADD_SEC3_UPPERSET,sector3.upperSet);
      if(sector3.lowerSet != tempRelay[1]){
        settingsChangeLog("Sector 3 Lower Set Value ",String(sector3.lowerSet),String(tempRelay[1]));
      }
      sector3.lowerSet = tempRelay[1];
      EEPROM.put(ADD_SEC3_LOWERSET,sector3.lowerSet);
      if(sector3.color != tempRelay[2]){
        settingsChangeLog("Sector 3 Color Value ",colorNames[sector3.color],colorNames[tempRelay[2]]);
      }
      sector3.color = tempRelay[2];
      EEPROM.put(ADD_SEC3_COLOR,sector3.color);
    }
    /////////////////////////////////////  SECTOR 4   /////////////////////////////////////////////////////
    if(inString.charAt(1) == '4'){
      if(sector4.upperSet != tempRelay[0]){
        settingsChangeLog("Sector 4 Upper Set Value ",String(sector4.upperSet),String(tempRelay[0]));
      }
      sector4.upperSet = tempRelay[0];
      EEPROM.put(ADD_SEC4_UPPERSET,sector4.upperSet);
      if(sector4.lowerSet != tempRelay[1]){
        settingsChangeLog("Sector 4 Lower Set Value ",String(sector4.lowerSet),String(tempRelay[1]));
      }
      sector4.lowerSet = tempRelay[1];
      EEPROM.put(ADD_SEC4_LOWERSET,sector4.lowerSet);
      if(sector4.color != tempRelay[2]){
        settingsChangeLog("Sector 4 Color Value ",colorNames[sector4.color],colorNames[tempRelay[2]]);
      }
      sector4.color = tempRelay[2];
      EEPROM.put(ADD_SEC4_COLOR,sector4.color);
    }
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Transmitter
  if(inString.charAt(0) == 'T'){


  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Time
  if(inString.charAt(0) == 't'){
    Serial1.println(Time.timeStr());
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Data Logger
  if(inString.charAt(0) == 'D'){
    int length = inString.length();
    index[0] = inString.indexOf(',',2);
    tempInt = inString.substring(2, index[0]).toInt();
    if(scanTime  != tempInt){
      settingsChangeLog("Data Log Scan time ",String(scanTime) ,String(tempInt));
    }
    scanTime = tempInt;
    EEPROM.write(ADD_DATALOG_TIME,scanTime);
    tempInt = inString.substring(index[0]+1,length).toInt();
    if(dataLogStatus != tempInt){
      settingsChangeLog("Data Log Status ",positionNames[dataLogStatus] ,positionNames[tempInt]);
    }
    dataLogStatus = tempInt;
    EEPROM.write(ADD_DATALOG_STS,dataLogStatus);
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Calibration
  if(inString.charAt(0) == 'C'){
    int tempRelay[4];
    int length = inString.length();
    index[0] = inString.indexOf(',',2);
    tempRelay[0] = inString.substring(2, index[0]).toInt();
    tempString = inString.substring(index[0]+1, length);
    index[0] = tempString.indexOf(',');
    tempRelay[1] = tempString.substring(0, index[0]).toInt() * 10;
    tempString = tempString.substring(index[0]+1,length);
    index[0] = tempString.indexOf(',');
    tempRelay[2] = tempString.substring(0, index[0]).toInt();
    tempRelay[3] = tempString.substring(index[0]+1,length).toInt() *10;
    calAdc[0] = tempRelay[0];
    calDisp[0] = tempRelay[1];
    calAdc[1] = tempRelay[2];
    calDisp[1] = tempRelay[3];
    EEPROM.put(ADD_ADC_CAL_0,calAdc[0]);
    EEPROM.put(ADD_ADC_CAL_1,calAdc[1]);
    EEPROM.put(ADD_DISP_CAL_0,calDisp[0]);
    EEPROM.put(ADD_DISP_CAL_1,calDisp[1]);
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //  Device Settings
  if(inString.charAt(0) == 'P'){


  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //  WiFi
  if(inString.charAt(0) == 'W'){
    int length = inString.length();
    index[0] = inString.indexOf(',',2);
    WifiSSID = inString.substring(2, index[0]);
    tempString = inString.substring(index[0]+1,length);
    index[1] = tempString.indexOf(',');
    WifiPASS = tempString.substring(0, index[1]);
    if(!WifiSSID.equals(String(WiFi.SSID()))){
      settingsChangeLog("Wifi SSID changed ",WiFi.SSID(),WifiSSID);
    }
    if((WifiSSID.toInt() != 0) && (WifiPASS.toInt() != 0)){
      WiFi.setCredentials(WifiSSID,WifiPASS);
    }
    tempInt = tempString.substring(index[1]+1,length).toInt();
    if(wifiStatus != tempInt){
      settingsChangeLog("Wifi Status ",positionNames[wifiStatus] ,positionNames[tempInt]);
    }
    wifiStatus = tempInt;
    EEPROM.write(WIFI_STATUS,wifiStatus);
    WiFi.setCredentials(WifiSSID,WifiPASS);
    if(wifiStatus){
      WiFi.on();
      WiFi.connect();
      Particle.connect();
    }
    else{
      WiFi.off();
    }
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Device Caliberation
  if(inString.charAt(0) == 'U'){
    char inputChar;
    byte calFlag = 0;
    tft.fillRect(0,0,128,128,BLACK);
    tft.setTextWrap(HIGH);
    tft.setFont(GLCDFONT);
    tft.setCursor(0,0);
    tft.println("Device Calibration");
    while(1){
      if(wifiStatus == 1){
        Particle.process();
      }
      if(Serial1.available()){
          inputChar = Serial1.read();
          if(inputChar == 's'){
            EEPROM.put(ADD_ADC_CAL_0,calAdc[0]);
            calFlag = 1;

          }
          if(inputChar == 'u'){
            tft.fillRect(0,0,128,128,BLACK);
            EEPROM.put(ADD_ADC_CAL_1,calAdc[1]);
            break;
          }
          if(calFlag == 0){
            if(inputChar == 'c'){
              calAdc[0]+=100;
            }
            if(inputChar == 'f'){
              calAdc[0]+=10;
            }
            if(inputChar == 'C'){
              calAdc[0]-=100;
            }
            if(inputChar == 'F'){
              calAdc[0]-=10;
            }
          }// end calFlag == 0
          if(calFlag == 1){
            if(inputChar == 'c'){
              calAdc[1]+=100;
            }
            if(inputChar == 'f'){
              calAdc[1]+=10;
            }
            if(inputChar == 'C'){
              calAdc[1]-=100;
            }
            if(inputChar == 'F'){
              calAdc[1]-=10;
            }
          }// end calFlag == 1
      }// end Serial available
      tft.setFont(ARIAL_36);
      initMCP3424(0x68,3,1,0);    /// add, sr,pga,ch
      adcValue = MCP3421getLong(0x68,3); /// add sr
      displayValue = mapf(adcValue,calAdc[0],calAdc[1],calDisp[0],calDisp[1]);
      //displayValue = mapf(adcValue,(float)calAdc[0],(float)calAdc[1],(float)calDisp[0],(float)calDisp[1]);
      hex2bcd(displayValue);
      tft.drawChar(0,30,thous,fgColor,bgColor,1);
      tft.drawChar(30,30,hunds,fgColor,bgColor,1);
      tft.drawChar(60,30,tens,fgColor,bgColor,1);
      drawPoint(90,59,4,fgColor);
      tft.drawChar(100,30,ones,fgColor,bgColor,1);
    }//end while 1
  }// end instring U
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Read File from Bluetooth
  if(inString.charAt(0) == 'F'){
    String fileName;
    int length = inString.length();
    fileName = inString.substring(2, length);
    //readFile(fileName);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug Screen on Serial Console
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void debugEvent(){
  String inString,tempString;
  char tempChar;
  int tempInt;
  int index[10];
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  if(Serial.available()){
    inString = Serial.readStringUntil('~');
  }
  ///////////////////////////////////RELAY///////////////////////////////////////
  if(inString.charAt(0) == 'r'){
    Serial.println("------------   RELAY SETTINGS    -------------------");
    Serial.println("-----------------  RELAY 1 -------------------");
    Serial.print("Relay1 Upper Set:    ");
    Serial.println(relay1.upperSet);
    Serial.print("Relay1 Upper Delay:  ");
    Serial.println(relay1.upperDelay);
    Serial.print("Relay1 Lower Set:    ");
    Serial.println(relay1.lowerSet);
    Serial.print("Relay1 Lower Delay:  ");
    Serial.println(relay1.lowerDelay);
    Serial.println("-----------------  RELAY 2 -------------------");
    Serial.print("Relay 2 Upper Set:   ");
    Serial.println(relay2.upperSet);
    Serial.print("Relay 2 Upper Delay: ");
    Serial.println(relay2.upperDelay);
    Serial.print("Relay 2 Lower Set:   ");
    Serial.println(relay2.lowerSet);
    Serial.print("Relay 2 Lower Delay: ");
    Serial.println(relay2.lowerDelay);
    Serial.println("-----------------  RELAY 3 -------------------");
    Serial.print("Relay 3 Upper Set:   ");
    Serial.println(relay3.upperSet);
    Serial.print("Relay3 Upper Delay:  ");
    Serial.println(relay3.upperDelay);
    Serial.print("Relay 3 Lower Set:   ");
    Serial.println(relay3.lowerSet);
    Serial.print("Relay 3 Lower Delay: ");
    Serial.println(relay3.lowerDelay);
    Serial.println("-----------------  RELAY 4 -------------------");
    Serial.print("Relay 4 Upper Set:   ");
    Serial.println(relay4.upperSet);
    Serial.print("Relay 4 Upper Delay: ");
    Serial.println(relay4.upperDelay);
    Serial.print("Relay 4 Lower Set:   ");
    Serial.println(relay4.lowerSet);
    Serial.print("Relay 4 Lower Delay: ");
    Serial.println(relay4.lowerDelay);
    Serial.println("-------------------------------------------");
  }
  ///////////////////////////////////SECTOR///////////////////////////////////////
  if(inString.charAt(0) == 's'){
    Serial.println("------------   SECTOR SETTINGS    -------------------");
    Serial.println("-----------------  SECTOR 1 -------------------");
    Serial.print("Sector 1 Lower Set:  ");
    Serial.println(sector1.lowerSet);
    Serial.print("Sector 1 Upper Set:  ");
    Serial.println(sector1.upperSet);
    Serial.print("Sector 1 Color:      ");
    Serial.println(colorNames[sector1.color]);
    Serial.println("-----------------  SECTOR 2 -------------------");
    Serial.print("Sector 2 Lower Set:  ");
    Serial.println(sector2.lowerSet);
    Serial.print("Sector 2 Upper Set:  ");
    Serial.println(sector2.upperSet);
    Serial.print("Sector 2 Color:      ");
    Serial.println(colorNames[sector2.color]);
    Serial.println("-----------------  SECTOR 3 -------------------");
    Serial.print("Sector 3 Lower Set:  ");
    Serial.println(sector3.lowerSet);
    Serial.print("Sector 3 Upper Set:  ");
    Serial.println(sector3.upperSet);
    Serial.print("Sector 3 Color:      ");
    Serial.println(colorNames[sector3.color]);
    Serial.println("-----------------  SECTOR 4 -------------------");
    Serial.print("Sector 4 Lower Set:  ");
    Serial.println(sector4.lowerSet);
    Serial.print("Sector 4 Upper Set:  ");
    Serial.println(sector4.upperSet);
    Serial.print("Sector 4 Color:      ");
    Serial.println(colorNames[sector4.color]);
    Serial.println("-------------------------------------------");
  }
  ///////////////////////////////////LIVE///////////////////////////////////////
  if(inString.charAt(0) == 'l'){
    if (DEBUG_LIVE){
      DEBUG_LIVE = 0;
      Serial.println("Live value debug stopped....");
    }
    else{
      DEBUG_LIVE = 1;
      Serial.println("Live value debug start....");
      Serial.println("------------   LIVE VARIABLE SATUS    -------------------");
      Serial.println("Mode\t\tUnit\t\tADC Reading\t\tValue\t\tRelay 1\tRelay 2\tRelay 3\tRelay4 \t");
    }
  }
  ///////////////////////////////////BLUETOOTH///////////////////////////////////////
  if(inString.charAt(0) == 'b'){
    if (DEBUG_BLUETOOTH){
      DEBUG_BLUETOOTH = 0;
      Serial.println("Bluetooth debug stopped....");
    }
    else{
      DEBUG_BLUETOOTH = 1;
      Serial.println("Bluetooth debug started....");
    }
  }
  ///////////////////////////////////WIFI STATUS///////////////////////////////////////
  if(inString.charAt(0) == 'w'){
    if(wifiStatus){
      if(WiFi.ready()){
        Serial.println("Wifi is turned ON");
        Serial.print("Wifi is Connected to Network :  ");
        Serial.println(WiFi.SSID());
      }
    }
    else{
      Serial.println("Wifi is turned OFF");
    }
    Serial.println("Following Access Points Stored in the Device :");
    WiFiAccessPoint ap[5];
    int found = WiFi.getCredentials(ap, 5);
    Serial.print("  SSID  ");
    Serial.print("\t");
    Serial.print("SECURITY");
    Serial.print("\t");
    Serial.println(" CIPHER ");
    for (int i = 0; i < found; i++){
      Serial.print(ap[i].ssid);
      // security is one of WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA, WLAN_SEC_WPA2
      Serial.print("\t");
      Serial.print(wlanSec[ap[i].security]);
      // cipher is one of WLAN_CIPHER_AES, WLAN_CIPHER_TKIP
      Serial.print("\t");
      Serial.println(wlanCipher[ap[i].cipher]);
    }
    byte mac[6];
    Serial.print("Wifi MAC Address : ");
    WiFi.macAddress(mac);
    for (int i=0; i<6; i++){
      Serial.printf("%02x%s", mac[i], i != 5 ? ":" : "");
    }
    Serial.println(" ");
    Serial.print("Wifi received Signal Strength :  ");
    Serial.print(WiFi.RSSI());
    Serial.println("   dB ");
  }// End If Wifi
///////////////////////////////////TIME///////////////////////////////////////
  if(inString.charAt(0) == 't'){
    Serial.println(Time.timeStr());
  }// End Time
///////////////////////////////////WIFI PROGRAM///////////////////////////////////////
  if(inString.charAt(0) == 'W' ){
    int length = inString.length();
    index[0] = inString.indexOf(',',2);
    WifiSSID = inString.substring(2, index[0]);
    tempString = inString.substring(index[0]+1,length);
    index[1] = tempString.indexOf(',');
    WifiPASS = tempString.substring(0, index[1]);
    wifiStatus = tempString.substring(index[1]+1,length).toInt();
    EEPROM.write(WIFI_STATUS,wifiStatus);
    if((WifiSSID.toInt() != 0) && (WifiPASS.toInt() != 0)){
      WiFi.setCredentials(WifiSSID,WifiPASS);
      Serial.print("Connecting to ");
      Serial.println(WifiSSID);
    }
    if(wifiStatus){
      Serial.println("Turning on Wifi...");
      WiFi.on();
      Serial.println("Connecting....");
      WiFi.connect();
      Serial.println("Connected!");
      Serial.println("Connecting to Particle....");
      Particle.connect();
    }
    else{
      WiFi.off();
      Serial.println("Wifi is Off.");
    }
  }
///////////////////////////////////ABOUT///////////////////////////////////////
  if(inString.charAt(0) == 'a' ){
    String testString;
    EEPROM.get(ADD_KUSRNO,testString);
    Serial.print("Serial No : ");
    Serial.println(testString);
    EEPROM.get(ADD_KUBATCHNO,testString);
    Serial.print("Batch No : ");
    Serial.println(testString);
    EEPROM.get(ADD_MODEL,testString);
    Serial.print("Model No : ");
    Serial.println(testString);
    EEPROM.get(ADD_FIRMWARE,testString);
    Serial.print("Firmware Version : ");
    Serial.println(testString);
    Serial.printlnf("System version: %s", System.version().c_str());
    Serial.print("Device ID : ");
    Serial.println(System.deviceID());
    Serial.print("Range : From  ");
    Serial.print(calDisp[0]);
    Serial.print("  to  ");
    Serial.println(calDisp[1]);
  }
  if(inString.charAt(0) == 'd'){
    if(dataLogStatus == 0){
      dataLogStatus = 1;
      Serial.println("Datalogging Started");
    }
    else{
      dataLogStatus = 0;
      Serial.println("Datalogging Stopped");
    }
  }
  if(inString.charAt(0) == 'f'){
    String fileName;
    int length = inString.length();
    fileName = inString.substring(2, length);
    Serial.println(fileName);
    readFile(fileName);
  }
///////////////////////////////////ABOUT///////////////////////////////////////
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Send all the saved data from the device to mobile
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendDataBluetooth(){
  Serial1.println("KSNO=KU12345,KBNO=170120,TSNO=2072756,TBNO=161225,NEWDEVICE=1,PASSSUPER=abc123,PASSADMIN=abc123,PASSUSER=abc123,RANGEL=0000,RANGEH=6000,VALUE=INT,SENSOR=1,OUTPUT=3,WIFISSID=skynet,WIFIPASS=password,MODE=1,DIA=,DNST=,UNIT=2,SCREEN=2,ZONE1L=ml,ZONE1H=INT,ZONE1CL=INT,ZONE2L=INT,ZONE2H=INT,ZONE2CL=INT,ZONE3L=INT,ZONE3H=INT,ZONE3CL=INT,ZONE4L=INT,ZONE4H=INT,ZONE4CL=INT,R1LS=INT,R1LD=INT,R1US=INT,R1UD=INT,R1MR=INT,R2LS=INT,R2LD=INT,R2US=INT,R2UD=INT,R2MR=INT,R3LS=INT,R3LD=INT,R3US=INT,R3UD=INT,R3MR=INT,R4LS=INT,R4LD=INT,R4US=INT,R4UD=INT,R4MR=INT,OUTCALZ=INT,OUTCALS=INT,SCANTIME=INT,DATALOGSTS=BOOLEAN,DATALOGTIME=INT,DISPSCROLLTIME=INT,DISPSCROLLSTS=BOOLEAN,CALDUE=STRING,TAGNO=INT,MODEL=STRING,FIRMWARE=STRING,~");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  DataLogging writes data to DATA_DDMMYYYY.txt file
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Datalog(){
  char day[2],month[2];
  String fileName;
  sprintf(day,"%02d",Time.day());
  sprintf(month,"%02d",Time.month());
  fileName = "DATA_" + String(day) + String(month) + String(Time.year()) + ".txt";
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)){
    sd.initErrorHalt();
    return;
  }
  if (!myFile.open(fileName, O_RDWR |O_AT_END)) {
    myFile.open(fileName, O_RDWR | O_CREAT | O_AT_END);
    myFile.println("Date,Time,Mode,Unit,Process Value,Relay 1,Relay 2,Relay 3,Relay 4");
  }
  // if the file opened okay, write to it:
  myFile.print(Time.day());
  myFile.print("-");
  myFile.print(Time.month());
  myFile.print("-");
  myFile.print(Time.year());
  myFile.print(",");
  myFile.print(Time.hour());
  myFile.print(":");
  myFile.print(Time.minute());
  myFile.print(":");
  myFile.print(Time.second());
  myFile.print(",");
  myFile.print(modeNames[mode]);
  myFile.print(",");
  myFile.print(unitNames[pvUnit]);
  myFile.print(",");
  myFile.print((float)displayValue/10);
  myFile.print(",");
  myFile.print(relay1.upperFlag);
  myFile.print(",");
  myFile.print(relay2.upperFlag);
  myFile.print(",");
  myFile.print(relay3.upperFlag);
  myFile.print(",");
  myFile.println(relay4.upperFlag);
  myFile.close();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Reset File Logging. Log after every Reset
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resetLog(){
  String fileName = "ResetLog.txt";
  String reason;
  if (System.resetReason() == RESET_REASON_USER){
    reason = "Reset button";
  }
  if (System.resetReason() == RESET_REASON_POWER_MANAGEMENT){
    reason = "Low-power management reset";
  }
  if (System.resetReason() == RESET_REASON_POWER_DOWN){
    reason = "Power-down reset";
  }
  if (System.resetReason() == RESET_REASON_POWER_BROWNOUT){
    reason = "Brownout reset";
  }
  if (System.resetReason() == RESET_REASON_WATCHDOG){
    reason = "Hardware watchdog reset";
  }
  if (System.resetReason() == RESET_REASON_UPDATE){
    reason = "Successful firmware update";
  }
  if (System.resetReason() == RESET_REASON_UPDATE_TIMEOUT){
    reason = "Firmware update timeout";
  }
  if (System.resetReason() == RESET_REASON_FACTORY_RESET){
    reason = "Factory reset requested";
  }
  if (System.resetReason() == RESET_REASON_SAFE_MODE){
    reason = " Safe mode requested";
  }
  if (System.resetReason() == RESET_REASON_DFU_MODE){
    reason = "DFU mode requested";
  }
  if (System.resetReason() == RESET_REASON_PANIC){
    reason = "System panic";
  }
  if (System.resetReason() == RESET_REASON_USER){
    reason = "User-requested reset";
  }
  if (System.resetReason() == RESET_REASON_UNKNOWN){
    reason = "Unspecified reset reason";
  }
  if (System.resetReason() == RESET_REASON_NONE){
    reason = "Information is not available";
  }
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)){
    sd.initErrorHalt();
    return;
  }
  if (!myFile.open(fileName, O_RDWR |O_AT_END)){
    myFile.open(fileName, O_RDWR | O_CREAT | O_AT_END);
    myFile.println("Reset Log File : Pressure Controller Version ");
    myFile.println("----------------------------------------------");
  }
  // if the file opened okay, write to it:
  myFile.print(Time.day());
  myFile.print("-");
  myFile.print(Time.month());
  myFile.print("-");
  myFile.print(Time.year());
  myFile.print("\t");
  myFile.print(Time.hour());
  myFile.print(":");
  myFile.print(Time.minute());
  myFile.print(":");
  myFile.print(Time.second());
  myFile.print("\t");
  myFile.print("  ---  ");
  myFile.print("Previous reset reason - ");
  myFile.println(reason);
  myFile.close();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Event Handler Save events to SD Card
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handle_all_the_events(system_event_t event, int param, void*)
{
  String fileName;
  int e = event;
  int p = param;
  String eventDesc,paramDesc;
  if(e == 2){
    eventDesc = "Device has entered setup mode";
    paramDesc = " ";
  }
  if(e == 4){
    eventDesc = "The device is still in setup mode - ";
    paramDesc = "since " + String(p) + "ms";
  }
  if(e == 8){
    eventDesc = "Setup mode was exited";
    paramDesc = "since " + String(p) + "ms";
  }
  if(e == 16){
    eventDesc = "Network credentials were - ";
    if(p == 0){
      paramDesc = "Added";
    }
    if(p == 1){
      paramDesc = "Cleared";
    }
  }
  if(e ==32){
    eventDesc = "Network connection status - ";
    if(p == 4){
      paramDesc = "Powering On";
    }
    if(p == 5){
      paramDesc = "On";
    }
    if(p == 6){
      paramDesc = "Connecting";
    }
    if(p == 7){
      paramDesc = "Connected";
    }
    if(p == 10){
      paramDesc = "Powering Off";
    }
    if(p == 11){
      paramDesc = "Off";
    }
  }
  if(e == 64){
    eventDesc = "Cloud connection status - ";
    if(p == 0){
      paramDesc = "Disconnected";
    }
    if(p == 1){
      paramDesc = "Connecting";
    }
    if(p == 8){
      paramDesc = "Connected";
    }
    if(p == 9){
      paramDesc = "Disconnecting";
    }
  }
  if(e == 128){
    eventDesc = "Button pressed for ";
    paramDesc = String(p) + "ms";

  }
  if(e == 256){
    eventDesc = "Firmware update status - ";
    if(p == 0){
      paramDesc = "Beginning";
    }
    if(p == 1){
      paramDesc = "Complete";
    }
    if(p == 2){
      paramDesc = "In Progress";
    }
    if(p == 3){
      paramDesc = "Failed";
    }
  }
  if(e == 512){
    eventDesc = "Firmware update is available";
    paramDesc = " ";
  }
  if(e == 1024){
    eventDesc = "The system would like to reset";
    paramDesc = " ";
  }
  if(e == 2048){
    eventDesc = "System will reset once the application has completed handling this event";
    paramDesc = " ";
  }
  if(e == 4096){
    eventDesc = "Setup button is clicked - ";
    paramDesc = String(p) + "times";
  }
  if(e == 8192){
    eventDesc = "Setup button is clicked - ";
    paramDesc = String(p) + "times";
  }
  if(e == 16384){
    eventDesc = "Device time changed - ";
    if(p == 0){
      paramDesc = "Manually";
    }
    if(p == 1){
      paramDesc = "Synch";
    }
  }
  if(e == 32768){
    eventDesc = "Low Battery"; "";
    paramDesc = "";
  }
  fileName = "EventLog.txt";
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)){
    sd.initErrorHalt();
    return;
  }
  if (!myFile.open(fileName, O_RDWR |O_AT_END)){
    myFile.open(fileName, O_RDWR | O_CREAT | O_AT_END);
    myFile.println("Event Log File : Pressure Controller Version ");
    myFile.println("----------------------------------------------");
  }
  // if the file opened okay, write to it:
  myFile.print(Time.day());
  myFile.print("-");
  myFile.print(Time.month());
  myFile.print("-");
  myFile.print(Time.year());
  myFile.print("\t");
  myFile.print(Time.hour());
  myFile.print(":");
  myFile.print(Time.minute());
  myFile.print(":");
  myFile.print(Time.second());
  myFile.print("\t");
  myFile.print("  ---  ");
  myFile.print(" Event :  ");
  myFile.print(eventDesc);
  myFile.println(paramDesc);
  myFile.close();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  DataLogging changed setting values to SChangeLog.txt
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void settingsChangeLog(String setting,String prevValue,String currValue){
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)){
    sd.initErrorHalt();
    return;
  }
  if (!myFile.open("SettingChangeLog.txt", O_RDWR |O_AT_END)) {
    myFile.open("SettingChangeLog.txt", O_RDWR | O_CREAT | O_AT_END);
    myFile.println("Settings Change Log File Pressure Controller Version ");
    myFile.println("--------------------------------------------------------------------");
  }
  // if the file opened okay, write to it:
  myFile.print(Time.day());
  myFile.print("-");
  myFile.print(Time.month());
  myFile.print("-");
  myFile.print(Time.year());
  myFile.print("\t");
  myFile.print(Time.hour());
  myFile.print(":");
  myFile.print(Time.minute());
  myFile.print(":");
  myFile.print(Time.second());
  myFile.print("\t---");
  myFile.print("\tSettings changed  of :\t");
  myFile.print(setting);
  myFile.print("\tfrom\t");
  myFile.print(prevValue);
  myFile.print("\tto\t");
  myFile.println(currValue);
  myFile.close();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Read file from SD card
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readFile(String fileName){
  // re-open the file for reading:
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)){
    sd.initErrorHalt();
    return;
    }
    if (!myFile.open(fileName, O_RDWR |O_AT_END)) {
    }
  myFile.close();
  if (!myFile.open(fileName, O_READ)){
    sd.errorHalt("opening file for read failed");
    return;
    }
  long filesize = myFile.fileSize();
  // read from the file until there's nothing else in it:
  while (filesize !=0) {
    char readChar = myFile.read();
    Serial1.write(readChar);
    Serial.write(readChar);
    filesize--;
    }
  // close the file:
  myFile.close();
  return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Heart Function
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void heartFunction(){
  tft.fillCircle(116, 116, 9, WHITE);
  if(!heartFlag){
    tft.drawBitmap(109, 110, heart, 16, 16,RED);
  }
  else{
    tft.drawBitmap(109, 110, smallHeart, 16, 16,RED);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Display Scroll Fucntion
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displayScroll(){
  if(dispScroll){
    if((seconds % scrollTime) == 0){
      tft.fillRect(0,55,128,128,BLACK);
      dispNow++;
      if(dispNow > 3){
        dispNow = 0;
      }
    }
    if(dispNow == 0){
      displayRelayValues(&relay1);
    }
    if(dispNow == 1){
      displayRelayValues(&relay2);
    }
    if(dispNow == 2){
      displayRelayValues(&relay3);
    }
    if(dispNow == 3){
      displayRelayValues(&relay4);
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Display Relay values in screen 3
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displayRelayValues(Relay* relay){
  tft.setFont(ARIAL_12);
  tft.setCursor(0, 110);
  tft.print(relay->relayName);
  tft.drawChar(115,88,'s',fgColor,bgColor,1);
  tft.drawChar(115,58,'s',fgColor,bgColor,1);
  tft.setFont(ARIAL_12_N);
  ////// Relay Upper
  hex2bcd(relay->upperSet);
  tft.drawChar(0,58,thous,fgColor,bgColor,1);
  tft.drawChar(13,58,hunds,fgColor,bgColor,1);
  tft.drawChar(26,58,tens,fgColor,bgColor,1);
  tft.drawChar(34,58,'.',fgColor,bgColor,1);
  tft.drawChar(45,58,ones,fgColor,bgColor,1);
  ////// Relay Upper delay
  hex2bcd(relay->upperDelay);
  tft.drawChar(84,58,tens,fgColor,bgColor,1);
  tft.drawChar(97,58,ones,fgColor,bgColor,1);
  ////// Lower
  hex2bcd(relay->lowerSet);
  tft.drawChar(0,88,thous,fgColor,bgColor,1);
  tft.drawChar(13,88,hunds,fgColor,bgColor,1);
  tft.drawChar(26,88,tens,fgColor,bgColor,1);
  tft.drawChar(34,88,'.',fgColor,bgColor,1);
  tft.drawChar(45,88,ones,fgColor,bgColor,1);
  ////// Lower delay
  hex2bcd(relay->lowerDelay);
  tft.drawChar(84,88,tens,fgColor,bgColor,1);
  tft.drawChar(97,88,ones,fgColor,bgColor,1);
  tft.setFont(GLCDFONT);
  tft.drawFastVLine(74, 0, 40, GRAY);
  tft.drawFastVLine(73, 0, 40, GRAY);
  tft.drawChar(62,54,0x18,RED,bgColor,2);
  tft.drawChar(62,86,0x19,RED,bgColor,2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Interrupts
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void timerISR(void)
{
 seconds++;
 heartFlag = !heartFlag;
 if(seconds > 100)
 {
     seconds = 0;
     prevSeconds = 0;
     count = 0;
 }
}
