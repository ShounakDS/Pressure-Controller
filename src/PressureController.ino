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

SYSTEM_MODE(MANUAL);

void setup() {
  Time.zone(+5.5);
  // Put initialization like pinMode and begin functions here.
    ////////////////////////////////    Pin Declarations   /////////////////////////////////////////////////////
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);
  pinMode(relay3Pin, OUTPUT);
  pinMode(relay4Pin, OUTPUT);

  pinMode(BTAT,OUTPUT);

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
  initMax7219();
  clearMax();
<<<<<<< HEAD
  Wire.begin();
  initMCP3424(0x68,0,3,0);    /// add, sr,pga,ch
  bluetoothMode(AT);
=======
  MCP.begin();
  MCP.configuration(0,16,1,1); // Channel 1, 16 bits resolution, one-shot mode, amplifier gain = 1

>>>>>>> parent of 2c76189... Deleted  MCP header File
  /////////////////////////////   Initializing Variables  //////////////////////////////////////////////////

  color = GREEN;
  fgColor = WHITE;
  bgColor = BLACK;
  selColor = YELLOW;
  clockColor = YELLOW;
  displayValue = 6000;

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




 /////////////////////////////   EEPROM Address Read     //////////////////////////////////////////////////
  readEEPROM();
  /////////////////////////////   RTC Data    //////////////////////////////////////////////////
  if(wifiStatus)
  {
    WiFi.on();
    WiFi.connect();
    Particle.connect();
  }
  else
  {
    WiFi.off();
  }
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  // The core of your code will likely live here.
  if(wifiStatus)
  {
    if(Particle.connected)
    {
      Particle.process();
    }
  }


 serialEvent();
 debugEvent();

 /////////////////////////////// Relay 1 //////////////////////////////////////


    if( seconds > prevSeconds)
    {
<<<<<<< HEAD
        //displayBargraph(count);
        //displayMax(cyclicRotate(0x01),1);
        initMCP3424(0x68,3,3,0);    /// add, sr,pga,ch
        adcValue = MCP3421getLong(0x68,3); /// add sr
        displayValue = mapf(adcValue,-4270,46531,0,100);
        printValue = printOLED((long)displayValue,pvUnit,setScreen);
=======

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

>>>>>>> parent of 2c76189... Deleted  MCP header File
        checkRelayStatus(displayValue,&relay1,relay1Pin);
        checkRelayStatus(displayValue,&relay2,relay2Pin);
        checkRelayStatus(displayValue,&relay3,relay3Pin);
        checkRelayStatus(displayValue,&relay4,relay4Pin);
        //////////////////////////////////////////////////////////////////////////////////////////////////////
        if(DEBUG_LIVE)
        {
          Serial.print(modeNames[mode]);
          Serial.print("\t");
          Serial.print(unitNames[pvUnit]);
          Serial.print("\t");
          Serial.print("\t");
          Serial.print(adcValue);
          Serial.print("\t");
          Serial.print("\t");
          Serial.print("\t");
          Serial.print(printValue);
          Serial.print("\t");
          Serial.print("\t");
          Serial.print(relay1.upperFlag);
          Serial.print("\t");
          Serial.print("\t");
          Serial.print(relay2.upperFlag);
          Serial.print("\t");
          Serial.print("\t");
          Serial.print(relay3.upperFlag);
          Serial.print("\t");
          Serial.print("\t");
          Serial.println(relay4.upperFlag);
        } ///// END DEBUG_LIVE
        prevSeconds = seconds;
    }

}






///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug Screen on Serial Console
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void debugEvent()
{
  String inString,tempString;
  char tempChar;
  int tempInt;
  int index[10];
  //////////////////////////////////////////////////////////////////////////////////////////////////////////
  if(Serial.available())
  {
    inString = Serial.readString();
  }
  ///////////////////////////////////RELAY///////////////////////////////////////
  if(inString.charAt(0) == 'r')
  {
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
  if(inString.charAt(0) == 's')
  {
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
  if(inString.charAt(0) == 'l')
  {
    if (DEBUG_LIVE)
    {
      DEBUG_LIVE = 0;
      Serial.println("Live value debug stopped....");
    }
    else
    {
      DEBUG_LIVE = 1;
      Serial.println("Live value debug start....");
      Serial.println("------------   LIVE VARIABLE SATUS    -------------------");
      Serial.println("Mode\t\tUnit\t\tADC Reading\t\tValue\t\tRelay 1\tRelay 2\tRelay 3\tRelay4 \t");
    }
  }
  ///////////////////////////////////BLUETOOTH///////////////////////////////////////
  if(inString.charAt(0) == 'b')
  {
    if (DEBUG_BLUETOOTH)
    {
      DEBUG_BLUETOOTH = 0;
      Serial.println("Bluetooth debug stopped....");
    }
    else
    {
      DEBUG_BLUETOOTH = 1;
      Serial.println("Bluetooth debug started....");
    }
  }
  ///////////////////////////////////WIFI STATUS///////////////////////////////////////
  if(inString.charAt(0) == 'w')
  {
    if(wifiStatus)
    {
      if(WiFi.ready())
      {
        Serial.println("Wifi is turned ON");
        Serial.print("Wifi is Connected to Network :  ");
        Serial.println(WiFi.SSID());
      }
    }
    else
    {
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
    for (int i = 0; i < found; i++)
    {
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
    for (int i=0; i<6; i++)
    {
      Serial.printf("%02x%s", mac[i], i != 5 ? ":" : "");
    }
    Serial.println(" ");
    Serial.print("Wifi received Signal Strength :  ");
    Serial.print(WiFi.RSSI());
    Serial.println("   dB ");
  }// End If Wifi
///////////////////////////////////TIME///////////////////////////////////////
  if(inString.charAt(0) == 't')
  {
    Serial.println(Time.format(Time.local(), TIME_FORMAT_DEFAULT));
  }// End Time
///////////////////////////////////WIFI PROGRAM///////////////////////////////////////
  if(inString.charAt(0) == 'W' )
  {
    int length = inString.length();
    index[0] = inString.indexOf(',',2);
    WifiSSID = inString.substring(2, index[0]);
    tempString = inString.substring(index[0]+1,length);

    index[1] = tempString.indexOf(',');
    WifiPASS = tempString.substring(0, index[1]);

    wifiStatus = tempString.substring(index[1]+1,length).toInt();
    EEPROM.write(WIFI_STATUS,wifiStatus);
    if((WifiSSID.toInt() != 0) && (WifiPASS.toInt() != 0))
    {
      WiFi.setCredentials(WifiSSID,WifiPASS);
      Serial.print("Connecting to ");
      Serial.println(WifiSSID);
    }
    if(wifiStatus)
    {
      Serial.println("Turning on Wifi...");
      WiFi.on();
      Serial.println("Connecting....");
      WiFi.connect();
      Serial.println("Connected!");
      Serial.println("Connecting to Particle....");
      Particle.connect();
    }
    else
    {
      WiFi.off();
      Serial.println("Wifi is Off.");
    }

  }
///////////////////////////////////ABOUT///////////////////////////////////////
  if(inString.charAt(0) == 'a' )
  {
    Serial.printlnf("System version: %s", System.version().c_str());
    Serial.print("Device ID : ");
    Serial.println(System.deviceID());
    Serial.print("Range : From - ");
    Serial.print(rangeLow);
    Serial.print("  to - ");
    Serial.println(rangeHigh);
  }
///////////////////////////////////ABOUT///////////////////////////////////////
}


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
    if((value >=sector1.lowerSet) && (value <= sector1.upperSet))
    {
        return colorValues[sector1.color];
    }
    else if((value > sector2.lowerSet) && (value <= sector2.upperSet))
    {
        return colorValues[sector2.color];
    }
    else if((value > sector3.lowerSet) && (value <= sector3.upperSet))
    {
        return colorValues[sector3.color];
    }
    else if((value > sector4.lowerSet) && (value <= sector4.upperSet))
    {
       return colorValues[sector4.color];
    }
    else
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

    wifiStatus = EEPROM.read(WIFI_STATUS);




}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Read EEprom
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void checkRelayStatus(long value,Relay* tempRelay,int relayPin)
{
    if( value >= tempRelay->upperSet)
        {
          tempRelay->upperCount++;
              if(tempRelay->upperCount > tempRelay->upperDelay)
              {
                tempRelay->upperCount = tempRelay->upperDelay + 1;
                tempRelay->upperFlag = HIGH;
                digitalWrite(relayPin,HIGH);
              }

        }
        if( (value <= tempRelay->lowerSet) && (tempRelay->upperFlag == HIGH) )
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
        if(DEBUG_BLUETOOTH)
          {
            Serial.println(inString);
          }

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
        //// Mode, Unit, Image, Range High, Range low
        int length = inString.length();
        tempChar = inString.charAt(2);
        mode = tempChar - 48;
        //EEPROM.put(ADD_MODE,mode);

        tempChar = inString.charAt(4);
        pvUnit = tempChar - 48;
        EEPROM.write(ADD_UNIT,pvUnit);

        tempChar = inString.charAt(6);
        setScreen = tempChar - 48;
        EEPROM.write(ADD_SCREEN ,setScreen);

        index[0] = inString.indexOf(',',7);
        rangeHigh = inString.substring(3, index[0]).toInt() * 10;
        tempString = inString.substring(index[0]+1, length);
        index[0] = tempString.indexOf(',');
        rangeLow = tempString.substring(0, index[0]).toInt() * 10;

        EEPROM.put(ADD_RANGEH,rangeHigh);
        EEPROM.put(ADD_RANGEL,rangeLow);

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
        if((WifiSSID.toInt() != 0) && (WifiPASS.toInt() != 0))
        {
          WiFi.setCredentials(WifiSSID,WifiPASS);
        }
        wifiStatus = tempString.substring(index[1]+1,length).toInt();
        EEPROM.write(WIFI_STATUS,wifiStatus);

        if(wifiStatus)
        {
          WiFi.on();
          WiFi.connect();
          Particle.connect();
        }
        else
        {
          WiFi.off();
        }

      }
}

<<<<<<< HEAD
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Functions for ADC Init and get value
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initMCP3424(uint8_t addr, uint8_t sr, uint8_t pga,uint8_t ch)
{
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
signed long MCP3421getLong(uint8_t addr,uint8_t sr)
{
  uint8_t b2,b3,b4,confwrite,confRead;
  signed long l1;
  if(sr < 3)
  {
      Wire.requestFrom(addr, 3);
      b2 = Wire.read();
      b3 = Wire.read();
      confRead = Wire.read();
      Wire.endTransmission();
      //Serial.println(confRead,HEX);
      l1= (256* b2 )+ b3;
  }
  else
  {
       Wire.requestFrom(addr, 4);
       b2 = Wire.read();
       b3 = Wire.read();
       b4 = Wire.read();
       confRead = Wire.read();
       Wire.endTransmission();
       l1= (long) b3*256;
       l1= l1 + b4;
       l1= l1+0x10000 * b2;
       if (b2 > 0x10)
       {
        l1= l1 + 0xFF000000;
       }
  }
   return(l1);
}
float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Bluetooth mode Selection
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void bluetoothMode(uint8_t modeSel)
{
    if(modeSel == 0)
    {
        digitalWrite(BTAT,LOW);
    }
    else
    {
       digitalWrite(BTAT,HIGH);
    }

}
=======

>>>>>>> parent of 2c76189... Deleted  MCP header File
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
