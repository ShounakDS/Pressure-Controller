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
SYSTEM_MODE(AUTOMATIC);
// Startup Functions

STARTUP(WiFi.selectAntenna(ANT_INTERNAL)); // selects the u.FL antenna
STARTUP(System.enableFeature(FEATURE_RESET_INFO));
// reset the system after 60 seconds if the application is unresponsive
//ApplicationWatchdog wd(10000, System.reset, 1536);


/// Set up Function
void setup() {
  tft.begin();
  tft.fillRect(0,0,128,128,BLACK);
  tft.setCursor(0,0);
  tft.setTextWrap(LOW);
  readEEPROM();
  printValue = printOLED(displayValue,pvUnit,setScreen);
  wifiStatus = 1;
  bluetoothCon();
  Time.zone(+5.5);
  resetLog();
  System.on(all_events, handle_all_the_events);
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
  Serial1.begin(115200);
  Serial.begin(115200);
  Wire.begin();
  initMCP3424(0x68,0,3,0);    /// add, sr,pga,ch
  delay(10);
  displayBargraph(0);
/////////////////////////////   Initializing Variables  ////////////////////////////////////////
  color = GREEN;
  fgColor = WHITE;
  bgColor = BLACK;
  selColor = CYAN;
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
  if(wifiStatus == 1){
    WiFi.on();
  }
  else{
    WiFi.off();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// loop() runs over and over again, as quickly as it can execute.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop(){
  //wd.checkin();
  if(wifiStatus  == 1){
    WiFi.connect();
    Particle.connect();
  }
  // The core of your code will likely live here.
  while(menuState == LOW){
    //wd.checkin();
    if(wifiStatus == 1){
      tft.drawBitmap(80, 110, wifi, 24, 24,WHITE);
      //tft.drawBitmap(40, 110, cloud, 24, 24,WHITE);
      if(Particle.connected){
        Particle.process();
      }
    }
    else{
        tft.drawBitmap(80, 110, wifi, 24, 24,BLACK);

    }
    bluetoothCon();
    bluetoothEvent();
    debugEvent();
    rtcSec = Time.second();
   /////////////////////////////// Relay 1 //////////////////////////////////////
    if( (rtcSec- rtcPrevSec) == scanTime){
      if(dataLogStatus){
        Datalog();
      }
      rtcPrevSec = rtcSec;
    }
    if(seconds > prevSeconds){
      initMCP3424(0x68,3,1,0);    /// add, sr,pga,ch
      adcValue = MCP3421getLong(0x68,3); /// add sr
      Serial.println(adcValue);
    //  Serial.println(adcValue);
        Particle.publish("Adc Value",String(adcValue));
        displayValue = mapf(adcValue,1150/*calAdc[0]*/,61228/*calAdc[1]*/,0,100); //displayValue = mapf(adcValue,-900,66600,0,100);
      // z 58381
      //
        bargraphValue = mapf(displayValue,0,100,0,50);
        displayBargraph(bargraphValue);
        if(checkKeypress() == ENTER){
          menuDelay++;
          if(menuDelay == 2){
            if(menuState == LOW){
              menuState = HIGH;
              menuDelay = 0;
            }
            else if(menuState == HIGH){
             menuState = LOW;
             menuDelay = 0;
            }
          }
       }
       if(checkKeypress() == UP){
          manrstDelay++;
           if(manrstDelay == 2){
             tft.fillRect(0,0,128,128,BLACK);
             while(1){
               tft.setFont(ARIAL_8);
               tft.setTextColor(fgColor);
               tft.setTextSize(1);
               tft.setCursor(0, 0);
               tft.print("M A N U A L  R E S E T");
               tft.drawLine(0,13,128,13,fgColor);
               ///////////////////////   Button Read Condition   ///////////////////////////
               if(checkKeypress() == DOWN){
                 delay(400);
               }
               if(checkKeypress() == UP){
                  delay(400);
               }
               if(checkKeypress() == RIGHT){
                  delay(400);
               }
               if(checkKeypress() == LEFT){
                   delay(400);
                   tft.fillRect(0,0,128,128,BLACK);
                   break;
               }
             }
           }
        }
      printValue = printOLED(displayValue,pvUnit,setScreen);
      String pubString = "D," + String(displayValue/10,1)+","+String(unitNames[pvUnit])+","+String(relay1.upperFlag)+","+String(relay2.upperFlag)+","+String(relay3.upperFlag)+","+String(relay4.upperFlag);
      Particle.publish("Pressure",pubString);
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
    }/// End seconds if Loop
  }
  tft.fillRect(0, 0, 128, 128, bgColor);
  menuFlagL = LOW;
  menuFlagH = LOW;
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  ///Menu State High. OLED diplays menu Screen
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  while(menuState == HIGH){
    tft.fillRect(0, 0, 128, 128, bgColor);
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////   Next = 0   First Menu Display
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    while( (menuState == HIGH) && (next == 0)){
    //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor);
      tft.setTextSize(1);
      tft.setCursor(0, 0);
      tft.print("M E N U");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        /// Display list with Selected String
        // variables passed : FONT,Display List,Selected Item, max values in string,Selection Color,Foreground Color, BackGround Color
        selectMenuString(ARIAL_8,menuNames,inc0,7,selColor,fgColor,bgColor);

        menuFlagH = HIGH;
      }
      //wd.checkin();
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == DOWN){
        delay(400);
         inc0++;
         menuFlagH = LOW;
         if(inc0 > 6){
           inc0 = 6;
         }
      }
      if(checkKeypress() == UP){
         delay(400);
         inc0--;
         menuFlagH = LOW;
         if(inc0 < 0){
           inc0 = 0;
         }
      }
      if(checkKeypress() == RIGHT){
         delay(400);
         next++;
         menuFlagH = LOW;
         if(next == 5){
           next = 0;
         }
      }
      if(checkKeypress() == LEFT){
          delay(400);
          inc0 = 0;
          if(menuState == LOW){
            menuState = HIGH;
          }
          else if(menuState == HIGH){
            menuState = LOW;
          }
       }
      /////////////////////////////////////////////////////////////////////////////////
    }//// while next = 0
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // resets the AWDT count
    tft.fillRect(0, 0, 128, 128, bgColor);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////   Next = 1    go one step inside the menu
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    while((menuState == HIGH) && (next == 1)){
      //wd.checkin();
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print(menuNames[inc0]);
      tft.drawLine(0,13,128,13,fgColor);
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //   INC  =
      while(inc0 == 0){
        //wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        if(menuFlagH == LOW){
          selectMenuString(ARIAL_8,modeNames,inc1,2,selColor,fgColor,bgColor);
          menuFlagH = HIGH;
         }
         ///////////////////////   Button Read Condition   ///////////////////////////
         if(checkKeypress() == DOWN){
           delay(400);
            inc1++;
            menuFlagH = LOW;
            if(inc1 > 1){
              inc1 = 1;
            }
         }
         if(checkKeypress() == UP){
            delay(400);
            inc1--;
            menuFlagH = LOW;
            if(inc1 <= 0){
              inc1 = 0;
            }
         }
         if(checkKeypress() == LEFT){
            delay(400);
            next--;
            menuFlagH = LOW;
            if(next < 0){
              next = 0;
            }
            break;
         }
         if(checkKeypress() == RIGHT){
            delay(400);
            next++;
            menuFlagH = LOW;
            if(next == 5){
              next = 0;
            }
            break;
         }
      }/// while inc0 == 0
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////   INC  = 1  Displays Relay names
      while(inc0 == 1)
      {
      //  wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        if(menuFlagH == LOW){
          selectMenuString(ARIAL_8,relayNames,inc1,4,selColor,fgColor,bgColor);
          menuFlagH = HIGH;
         }
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == DOWN){
          delay(400);
           inc1++;
           menuFlagH = LOW;
           if(inc1 > 3){
             inc1 = 3;
           }
        }
        if(checkKeypress() == UP){
           delay(400);
           inc1--;
           menuFlagH = LOW;
           if(inc1 <= 0){
             inc1 = 0;
           }
        }
        if(checkKeypress() == RIGHT){
           delay(400);
           next++;
           menuFlagH = LOW;
           if(next == 5){
             next = 0;
           }
           break;
        }
        if(checkKeypress() == LEFT){
           delay(400);
           next--;
           inc1 = 0;
           menuFlagH = LOW;
           if(next == 5){
             next = 0;
           }
           break;
        }
      }/// while inc0 == 1
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////   INC  = 2 Displays sector Names
      while(inc0 == 2)
      {
        //wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
         if(menuFlagH == LOW){
          selectMenuString(ARIAL_8,sectorNames,inc1,4,selColor,fgColor,bgColor);
          menuFlagH = HIGH;
         }
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == DOWN){
          delay(400);
           inc1++;
           menuFlagH = LOW;
           if(inc1 > 3){
             inc1 = 3;
           }
        }
        if(checkKeypress() == UP){
           delay(400);
           inc1--;
           menuFlagH = LOW;
           if(inc1 <= 0){
             inc1 = 0;
           }
        }
        if(checkKeypress() == RIGHT){
           delay(400);
           next++;
           menuFlagH = LOW;
           if(next == 5){
             next = 0;
           }
           break;
        }
        if(checkKeypress() == LEFT){
           delay(400);
           next--;
           menuFlagH = LOW;
           if(next == 5){
             next = 0;
           }
           inc1 = 0;
           break;
        }
      }/// while inc0 == 2
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////   INC  = 3 Diplays Transmitter Menu inside
      while(inc0 == 3)
      {
      //  wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        if(menuFlagH == LOW){
          selectMenuString(ARIAL_8,outputNames,inc1,2,selColor,fgColor,bgColor);
          menuFlagH = HIGH;
         }
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == DOWN){
          delay(400);
           inc1++;
           menuFlagH = LOW;
           if(inc1 > 1){
             inc1 = 1;
           }
        }
        if(checkKeypress() == UP){
           delay(400);
           inc1--;
           menuFlagH = LOW;
           if(inc1 <= 0){
             inc1 = 0;
           }
        }
        if(checkKeypress() == RIGHT){
           delay(400);
           next++;
           menuFlagH = LOW;
           if(next == 5){
             next = 0;
           }
           break;
        }
        if(checkKeypress() == LEFT){
           delay(400);
           next--;
           menuFlagH = LOW;
           if(next == 5){
             next = 0;
           }
           inc1 = 0;
           break;
        }
      }/// while inc0 == 3
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////   INC  = 4
      //////  Diplays date and time
      while(inc0 == 4)
      {
        //wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        if(menuFlagH == LOW){
        selectMenuString(ARIAL_8,timeNames,inc1,2,selColor,fgColor,bgColor);
        menuFlagH = HIGH;
        }
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == DOWN){
          delay(400);
           inc1++;
           menuFlagH = LOW;
           if(inc1 > 1){
             inc1 = 1;
           }
        }
        if(checkKeypress() == UP){
           delay(400);
           inc1--;
           menuFlagH = LOW;
           if(inc1 <= 0){
             inc1 = 0;
           }
        }
        if(checkKeypress() == RIGHT){
           delay(400);
           next++;
           menuFlagH = LOW;
           if(next == 5){
             next = 0;
           }
           break;
        }
        if(checkKeypress() == LEFT){
           delay(400);
           next--;
           menuFlagH = LOW;
           if(next == 5){
             next = 0;
           }
           inc1 = 0;
           break;
        }
      }/// while inc0 == 4
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////   INC  = 5  Diplays datalog
      while(inc0 == 5){
      //  wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        if(menuFlagH == LOW){
        selectMenuString(ARIAL_8,datalogNames,inc1,3,selColor,fgColor,bgColor);
        menuFlagH = HIGH;
        }
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == DOWN){
          delay(400);
           inc1++;
           menuFlagH = LOW;
           if(inc1 > 2){
             inc1 = 2;
           }
        }
        if(checkKeypress() == UP){
           delay(400);
           inc1--;
           menuFlagH = LOW;
           if(inc1 <= 0){
             inc1 = 0;
           }
        }
        if(checkKeypress() == RIGHT){
           delay(400);
           next++;
           menuFlagH = LOW;
           if(next == 5){
             next = 0;
           }
           break;
        }
        if(checkKeypress() == LEFT){
           delay(400);
           next--;
           menuFlagH = LOW;
           if(next == 5){
             next = 0;
           }
           inc1 = 0;
           break;
        }
      }/// while inc0 == 5
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////   INC  = 6 Diplays settins Menu
      while(inc0 == 6){
        //wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        if(menuFlagH == LOW){
         selectMenuString(ARIAL_8,settingNames,inc1,7,selColor,fgColor,bgColor);
         menuFlagH = HIGH;
        }
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == DOWN){
         delay(400);
          inc1++;
          menuFlagH = LOW;
          if(inc1 > 6){
            inc1 = 6;
          }
        }
        if(checkKeypress() == UP){
          delay(400);
          inc1--;
          menuFlagH = LOW;
          if(inc1 <= 0){
            inc1 = 0;
          }
        }
        if(checkKeypress() == RIGHT){
          delay(400);
          next++;
          menuFlagH = LOW;
          if(next == 5){
            next = 0;
          }
          break;
        }
        if(checkKeypress() == LEFT){
          delay(400);
          next--;
          inc1 = 0;
          menuFlagH = LOW;
          if(next == 5){
            next = 0;
          }
          break;
        }
      }/// while inc0 == 6
    }//// while next = 1
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    tft.fillRect(0, 0, 128, 128, bgColor);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NEXT  = 2
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    while((menuState == HIGH) && (next == 2)){
      //wd.checkin();
    //  inc2  = pvUnit;
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // menu for Mode
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //// Display Inside Mode and Unit
      while((inc0 == 0) && (inc1 == 0)){
        //wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        if(menuFlagH == LOW){
          tft.setFont(ARIAL_8);
          tft.setTextColor(CYAN,bgColor);
          tft.setTextSize(1);
          tft.setCursor(80,0);
          tft.print(unitNames[pvUnit]);
          tft.setTextColor(fgColor,bgColor);
          tft.setCursor(0,0);
          tft.print("U n i t s ");
          tft.drawLine(0,13,128,13,fgColor);
          menuFlagH = HIGH;
        }
        tft.setFont(ARIAL_12);
        tft.setCursor(10,54);
        tft.print(unitNames[inc2]);
        tft.setFont(GLCDFONT);
        tft.drawChar(charPos2[2],25,30,fgColor,bgColor,3);
        tft.drawChar(charPos2[2],85,31,fgColor,bgColor,3);
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == UP){
          delay(400);
          inc2++;
          if(inc2 > 3){
            inc2 = 3;
          }
        }
        if(checkKeypress() == DOWN){
          delay(400);
          inc2--;
          if(inc2 <= 0){
            inc2 = 0;
          }
        }
        if(checkKeypress() == ENTER){
          tft.setTextColor(BLACK);
          tft.setCursor(80,0);
          tft.print(unitNames[pvUnit]);
          pvUnit = inc2;
          tft.setTextColor(CYAN);
          tft.print(unitNames[pvUnit]);
          tft.setCursor(0,110);
          tft.setTextColor(WHITE);
          tft.print("Saving data...");
          EEPROM.write(ADD_UNIT,pvUnit);
          delay(2000);
          menuFlagH = LOW;
          inc1 = 0;
          next--;
          break;
        }
        if(checkKeypress() == LEFT){
          delay(400);
          next--;
          inc1 = 0;
          menuFlagH = LOW;
          break;
        }
      }//while inc0 == 1   inc1 == 0
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //// Display Inside  Screen
        //inc2  = setScreen;
      while((inc0 == 0) && (inc1 == 1)){
        //wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        if(menuFlagH == LOW){
          inc2 = 1;
          tft.setFont(ARIAL_8);
          tft.setTextColor(CYAN,bgColor);
          tft.setTextSize(1);
          tft.setCursor(80,0);
          tft.print(setScreen + 1);
          tft.setTextColor(fgColor,bgColor);
          tft.setCursor(0,0);
          tft.print("S c r e e n");
          tft.drawLine(0,13,128,13,fgColor);
          menuFlagH = HIGH;
        }
        tft.setFont(ARIAL_12_N);
        tft.setCursor(charPos2[2],54);
        tft.print(inc2);
        tft.setFont(GLCDFONT);
        tft.drawChar(charPos2[2],25,30,fgColor,bgColor,3);
        tft.drawChar(charPos2[2],85,31,fgColor,bgColor,3);
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == UP){
          delay(400);
          inc2++;
          if(inc2 > 3){
            inc2 = 3;
          }
        }
        if(checkKeypress() == DOWN){
          delay(400);
          inc2--;
          if(inc2 <= 1){
            inc2 = 1;
          }
        }
        if(checkKeypress() == ENTER){
          tft.setTextColor(BLACK);
          tft.setCursor(80,0);
          tft.print(setScreen);
          setScreen = inc2 - 1;
          tft.setTextColor(CYAN);
          tft.print(setScreen + 1);
          tft.setCursor(0,110);
          tft.setTextColor(WHITE);
          tft.print("Saving data...");
          EEPROM.write(ADD_SCREEN,setScreen);
          delay(2000);
          menuFlagH = LOW;
          inc1 = 0;
          next--;
          break;
        }
        if(checkKeypress() == LEFT){
          delay(400);
          next--;
          inc1 = 0;
          inc2 = 0;
          menuFlagH = LOW;
          break;
        }

      }//while inc0 == 1   inc1 == 1
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // menu for Relays
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //// Display Inside Relay Relay 1, Relay 2, Relay 3, Relay 4
      //  inc2  = 0;
      while((inc0 == 1) && (inc1 < 4)){
        //wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        tft.setFont(ARIAL_8);
        tft.setTextColor(fgColor,bgColor);
        tft.setTextSize(1);
        tft.setCursor(0,0);
        tft.print(relayNames[inc1]);
        tft.drawLine(0,13,128,13,fgColor);
        if(menuFlagH == LOW){
          selectMenuString(ARIAL_8,relay1Names,inc2,3,selColor,fgColor,bgColor);
          menuFlagH = HIGH;
        }
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == DOWN){
          delay(400);
          inc2++;
          menuFlagH = LOW;
          if(inc2 > 2){
            inc2 = 2;
          }
        }
        if(checkKeypress() == UP){
          delay(400);
          inc2--;
          menuFlagH = LOW;
          if(inc2 <= 0){
            inc2 = 0;
          }
        }
        if(checkKeypress() == RIGHT){
          menuFlagH = LOW;
          next++;
          if(next == 5){
            next = 0;
          }
          break;
        }
        if(checkKeypress() == LEFT){
          delay(400);
          next--;
          inc2 = 0;
          menuFlagH = LOW;
          break;
        }
      }//while inc0 == 1   inc1 == 0
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // menu for Sectors
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //// Display Inside Sector Sector1, Sector 2, Sector 3 ,Sector 4
      //inc2 = 0;
      while((inc0 == 2) && (inc1 < 4)){
        //wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        tft.setFont(ARIAL_8);
        tft.setTextColor(fgColor,bgColor);
        tft.setTextSize(1);
        tft.setCursor(0,0);
        tft.print(sectorNames[inc1]);
        tft.drawLine(0,13,128,13,fgColor);
        if(menuFlagH == LOW){
          selectMenuString(ARIAL_8,sector1Names,inc2,3,selColor,fgColor,bgColor);
          menuFlagH = HIGH;
        }
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == DOWN){
          delay(400);
          inc2++;
          menuFlagH = LOW;
          if(inc2 > 2){
            inc2 = 2;
          }
        }
        if(checkKeypress() == UP){
          delay(400);
          inc2--;
          menuFlagH = LOW;
          if(inc2 <= 0){
            inc2 = 0;
          }
        }
        if(checkKeypress() == RIGHT){
          menuFlagH = LOW;
          next++;
          if(next == 5){
            next = 0;
          }
          break;
        }
        if(checkKeypress() == LEFT){
          delay(400);
          next--;
          inc2 = 0;
          menuFlagH = LOW;
          break;
        }
      }//while inc0 == 2   inc1 == 0
      ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
      // menu for Time and Date
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////    TIME     ////////////////////////////////////////////////////////
      //get the time from real time Clock
      tempVar[0] = Time.hour();
      tempVar[1] = Time.minute();
      tempVar[2] = Time.second();
      pos = 0;
      while((inc0 == 4) && (inc1 == 0)){
        //wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        tft.setFont(ARIAL_8);
        tft.setTextColor(fgColor,bgColor);
        tft.setTextSize(1);
        tft.setCursor(0,0);
        tft.print("T i m e       ");
        tft.drawLine(0,13,128,13,fgColor);
        tft.setFont(ARIAL_8_N);
        tft.print(Time.hour());
        tft.print(":");
        tft.print(Time.minute());
        tft.print(":");
        tft.print(Time.second());
        if(menuFlagH == LOW){
          tft.setFont(ARIAL_12_N);
          hex2bcd(tempVar[0]);
          tft.drawChar(0,54,tens,fgColor,bgColor,1);
          tft.drawChar(15,54,ones,fgColor,bgColor,1);
          tft.drawChar(30,54,':',fgColor,bgColor,1);
          hex2bcd(tempVar[1]);
          tft.drawChar(45,54,tens,fgColor,bgColor,1);
          tft.drawChar(60,54,ones,fgColor,bgColor,1);
          tft.drawChar(75,54,':',fgColor,bgColor,1);
          hex2bcd(tempVar[2]);
          tft.drawChar(90,54,tens,fgColor,bgColor,1);
          tft.drawChar(105,54,ones,fgColor,bgColor,1);
          tft.setFont(GLCDFONT);
          for(i=0; i < pos  ; i++){
             tft.drawChar( charPosDateTime[i],24,30,bgColor,bgColor,2);
             tft.drawChar( charPosDateTime[i],85,31,bgColor,bgColor,2);
          }
          tft.drawChar( charPosDateTime[pos],24,30,fgColor,bgColor,2);
          tft.drawChar( charPosDateTime[pos],85,31,fgColor,bgColor,2);
          for(i=pos + 1  ; i < 3 ; i++){
            tft.drawChar( charPosDateTime[i],24,30,bgColor,bgColor,2);
            tft.drawChar( charPosDateTime[i],85,31,bgColor,bgColor,2);
          }
          menuFlagH = HIGH;
        }
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == UP){
          delay(400);
          tempVar[pos]++;
          if(pos == 0){
            if(tempVar[pos] > 23) tempVar[pos] = 23;
          }
          if((pos == 1) || (pos == 2)){
            if(tempVar[pos] > 59) tempVar[pos] = 59;
          }
          menuFlagH = LOW;
        }
        if(checkKeypress() == DOWN){
          delay(400);
          tempVar[pos]--;
          menuFlagH = LOW;
          if( tempVar[pos] < 0){
           tempVar[pos] = 0;
          }

        }
        if(checkKeypress() == ENTER){
          menuFlagH = LOW;
          delay(200);
          menuFlagH = LOW;
          //Time.setTime(getTimestamp(Time.year(), Time.month(), Time.day(), tempVar[0], tempVar[1], tempVar[1]));
          next--;
          break;
        }
        if(checkKeypress() == LEFT){
          delay(400);
          menuFlagH = LOW;
          if(pos > 0){
            pos--;
          }
          if(pos == 0){
            next--;
            inc2 = 0;
            menuFlagH = LOW;
            break;
          }
        }
        if(checkKeypress() == RIGHT){
          delay(400);
          menuFlagH = LOW;
          if(pos < 3){
            pos++;
          }
          else{
            pos = 3;
          }
        }
        /////////////////////////////////////////////////////////////////////////////////;
      }//while inc0 == 4   inc1 == 0 Time
      /////////////////////    TIME     ////////////////////////////////////////////////////////
      //get the time from real time Clock
      tempVar[0] = Time.day();
      tempVar[1] = Time.month();
      tempVar[2] = Time.year();
      pos = 0;
      while((inc0 == 4) && (inc1 == 1)){
      //  wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        tft.setFont(ARIAL_8);
        tft.setTextColor(fgColor,bgColor);
        tft.setTextSize(1);
        tft.setCursor(0,0);
        tft.print("D a t e      ");
        tft.drawLine(0,13,128,13,fgColor);
        tft.setFont(ARIAL_8_N);
        tft.print(Time.day());
        tft.print("-");
        tft.print(Time.month());
        tft.print("-");
        tft.print(Time.year());
        if(menuFlagH == LOW){
          tft.setFont(ARIAL_12_N);
          hex2bcd(tempVar[0]);
          tft.drawChar(0,54,tens,fgColor,bgColor,1);
          tft.drawChar(15,54,ones,fgColor,bgColor,1);
          tft.drawChar(30,54,'-',fgColor,bgColor,1);
          hex2bcd(tempVar[1]);
          tft.drawChar(45,54,tens,fgColor,bgColor,1);
          tft.drawChar(60,54,ones,fgColor,bgColor,1);
          tft.drawChar(75,54,'-',fgColor,bgColor,1);
          hex2bcd(tempVar[2]);
          tft.drawChar(90,54,tens,fgColor,bgColor,1);
          tft.drawChar(105,54,ones,fgColor,bgColor,1);
          tft.setFont(GLCDFONT);
          for(i=0; i < pos  ; i++){
             tft.drawChar( charPosDateTime[i],24,30,bgColor,bgColor,2);
             tft.drawChar( charPosDateTime[i],85,31,bgColor,bgColor,2);
          }
          tft.drawChar( charPosDateTime[pos],24,30,fgColor,bgColor,2);
          tft.drawChar( charPosDateTime[pos],85,31,fgColor,bgColor,2);
          for(i=pos + 1  ; i < 3 ; i++){
            tft.drawChar( charPosDateTime[i],24,30,bgColor,bgColor,2);
            tft.drawChar( charPosDateTime[i],85,31,bgColor,bgColor,2);
          }
          menuFlagH = HIGH;
        }
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == UP){
          delay(400);
          tempVar[pos]++;
          if(pos == 0){
            if(tempVar[pos] > 31) tempVar[pos] = 31;
          }
          if(pos == 1){
            if(tempVar[pos] > 12) tempVar[pos] = 12;
          }
          if(pos == 2){
            if(tempVar[pos] > 99) tempVar[pos] = 99;
          }
          menuFlagH = LOW;
        }
        if(checkKeypress() == DOWN){
          delay(400);
          tempVar[pos]--;
          menuFlagH = LOW;
          if( tempVar[pos] < 1){
           tempVar[pos] = 1;
          }

        }
        if(checkKeypress() == ENTER){
          menuFlagH = LOW;
          delay(200);
          menuFlagH = LOW;
          //Time.setTime(getTimestamp(tempVar[0], tempVar[1], tempVar[1],Time.hour(),Time.minute(),Time.second() ));
          next--;
          break;
        }
        if(checkKeypress() == LEFT){
          delay(400);
          menuFlagH = LOW;
          if(pos > 0){
            pos--;
          }
          if(pos == 0){
            next--;
            inc2 = 0;
            menuFlagH = LOW;
            break;
          }
        }
        if(checkKeypress() == RIGHT){
          delay(400);
          menuFlagH = LOW;
          if(pos < 3){
            pos++;
          }
          else{
            pos = 3;
          }
        }
        /////////////////////////////////////////////////////////////////////////////////;
      }//while inc0 == 4   inc1 == 1 Date
      /////////////////////    Data Log on and off    ////////////////////////////////////////////////////////
      //inc2 = dataLogStatus;
      while( (inc0 == 5) && (inc1 == 0)){
        //wd.checkin();
        if(Particle.connected){
          Particle.process();
        }
        tft.setFont(ARIAL_8);
        tft.setTextColor(fgColor,bgColor);
        tft.setTextSize(1);
        tft.setCursor(0,0);
        tft.print("L o g  S t a t u s      ");
        tft.print(positionNames[dataLogStatus]);
        tft.drawLine(0,13,128,13,fgColor);
        if(menuFlagH == LOW){
          tft.setFont(ARIAL_12);
          tft.setTextColor(fgColor,bgColor);
          tft.setTextSize(1);
          tft.setCursor(10,54);
          tft.print(positionNames[inc2]);
          tft.setFont(GLCDFONT);
          tft.drawChar(charPos2[3],35,30,fgColor,bgColor,2);
          tft.drawChar(charPos2[3],75,31,fgColor,bgColor,2);
           menuFlagH = HIGH;
        }
        ///////////////////////   Button Read Condition   ///////////////////////////
        if(checkKeypress() == UP){
          delay(400);
          inc2++;
          if(inc2 > 1){
            inc2 = 1;
          }
          menuFlagH = LOW;
        }
        if(checkKeypress() == DOWN){
          delay(400);
          inc2--;
          menuFlagH = LOW;
          if( inc2 < 0){
           inc2 = 0;
          }
        }
        if(checkKeypress() == ENTER){
          menuFlagH = LOW;
          delay(400);
          dataLogStatus = inc2;
          EEPROM.write(ADD_DATALOG_STS,dataLogStatus);
          next--;
          break;
        }
        if(checkKeypress() == LEFT){
          delay(400);
          menuFlagH = LOW;
          next--;
          inc2 = 0;
          break;
        }
        if(checkKeypress() == RIGHT){
          delay(400);
          menuFlagH = LOW;
          if(pos < 3){
            pos++;
          }
          else{
            pos = 3;
          }
        }
      /////////////////////////////////////////////////////////////////////////////////;
    }//while inc0 == 5  inc1 == 0
    /////////////////////    Data Log Set Scan Time    ////////////////////////////////////////////////////////
    hex2bcd(scanTime);
    tempVar[0] = hunds - 48;
    tempVar[1] = tens - 48;
    tempVar[2] = ones - 48;
    pos = 0;
    while( (inc0 == 5) && (inc1 == 1)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("S c a n  T i m e      ");
      tft.print(scanTime);
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
      /// selectTimeMenu(uint8_t fontName,char* displayString,char* charPositions,int positions,int foreColor, int backColor)
       selectTimeMenu(ARIAL_12_N,tempVar,charPos2,pos,fgColor, bgColor);
       menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == UP){
        delay(400);
        tempVar[pos+1]++;
        menuFlagH = LOW;
        if( tempVar[pos+1] > 9){
         tempVar[pos+1] = 9;
        }
        menuFlagH = LOW;
      }
      if(checkKeypress() == DOWN){
        delay(400);
        tempVar[pos+1]--;
        menuFlagH = LOW;
        if( tempVar[pos+1] < 0){
         tempVar[pos+1] = 0;
        }
        menuFlagH = LOW;
      }
      if(checkKeypress() == ENTER){
        delay(400);
        menuFlagH = LOW;
        scanTime = (tempVar[1] * 10)  + tempVar[2];
        EEPROM.write(ADD_SCAN_TIME,scanTime);
        next--;
        break;
      }
      if(checkKeypress() == LEFT){
        delay(400);
        pos--;
        menuFlagH = LOW;
        if(pos < 0){
          next--;
          menuFlagH = LOW;
          break;
        }
      }
      if(checkKeypress() == RIGHT){
        delay(400);
        pos++;
        menuFlagH = LOW;
        if(pos > 1){
          pos = 1;
        }
      }
    /////////////////////////////////////////////////////////////////////////////////;
    }//while inc0 == 5 inc1 == 1
    /////////////////////    Calibration Status   ////////////////////////////////////////////
    while( (inc0 == 6) && (inc1 == 0)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("C a l   D u e   D a t e   ");
      tft.drawLine(0,13,128,13,fgColor);
      tft.setCursor(0,30);
      tft.setFont(ARIAL_12);
      tft.print("DD-MM-YYYY");
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == ENTER){
        delay(400);
        menuFlagH = LOW;
        next--;
        break;
      }
      if(checkKeypress() == LEFT){
        delay(400);
        menuFlagH = LOW;
        next--;
        break;
      }
    }//while inc0 == 6 inc1 == 0
    //inc2 = wifiStatus;
    /////////////////////    Wi fi Status on and off    /////////////////////////////////
    while( (inc0 == 6) && (inc1 == 1)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("W i F i  S t a t u s  ");
      tft.print(positionNames[wifiStatus]);
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        tft.setFont(ARIAL_12);
        tft.setTextColor(fgColor,bgColor);
        tft.setTextSize(1);
        tft.setCursor(10,54);
        tft.print(positionNames[inc2]);
        tft.setFont(GLCDFONT);
        tft.drawChar(charPos2[3],35,30,fgColor,bgColor,2);
        tft.drawChar(charPos2[3],75,31,fgColor,bgColor,2);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == UP){
        delay(400);
        inc2++;
        if(inc2 > 1){
          inc2 = 1;
        }
        menuFlagH = LOW;
      }
      if(checkKeypress() == DOWN){
        delay(400);
        inc2--;
        menuFlagH = LOW;
        if( inc2 < 0){
         inc2 = 0;
        }
      }
      if(checkKeypress() == ENTER){
        menuFlagH = LOW;
        delay(400);
        wifiStatus = inc2;
        EEPROM.write(ADD_WIFI_STATUS,wifiStatus);
        if(wifiStatus == 1){
          WiFi.on();
          WiFi.connect();
          Particle.connect();
        }
        else{
          WiFi.off();
        }
        next--;
        break;
      }
      if(checkKeypress() == LEFT){
        delay(400);
        menuFlagH = LOW;
        next--;
        inc2 = 0;
        break;
      }
    }//while inc0 == 6  inc1 == 1
    /////////////////////    Change Pin to be made later///////////////////////////////
    while( (inc0 == 6) && (inc1 == 2)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("C h a n g e  P I N");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        tft.setFont(ARIAL_12);
        tft.setTextColor(fgColor,bgColor);
        tft.setTextSize(1);
        tft.setCursor(10,54);
        tft.print("* * * *");
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == ENTER){
        delay(400);
        menuFlagH = LOW;
        next--;
        break;
      }
      if(checkKeypress() == LEFT){
        delay(400);
        menuFlagH = LOW;
        next--;
        break;
      }
    }//while inc0 == 6  inc1 == 2
    //////////////   Display Menu  /////////////////////////////////////
    while((inc0 == 6) && (inc1 == 3)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("D i s p l a y");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        selectMenuString(ARIAL_8,displayNames,inc2,2,selColor,fgColor,bgColor);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == DOWN){
        delay(400);
        inc2++;
        menuFlagH = LOW;
        if(inc2 > 1){
          inc2 = 1;
        }
      }
      if(checkKeypress() == UP){
        delay(400);
        inc2--;
        menuFlagH = LOW;
        if(inc2 <= 0){
          inc2 = 0;
        }
      }
      if(checkKeypress() == RIGHT){
        menuFlagH = LOW;
        next++;
        if(next == 5){
          next = 0;
        }
        break;
      }
      if(checkKeypress() == LEFT){
        delay(400);
        next--;
        inc2 = 0;
        menuFlagH = LOW;
        break;
      }
    }//while inc0 == 2   inc1 == 3
    /////////////////////    Setup INFORMATION about the Device////////////////////////////////////////////
    while( (inc0 == 6) && (inc1 == 4)){
    //  wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("D e v i c e   I n f o   ");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        tft.setFont(ARIAL_8);
        tft.setTextSize(1);
        tft.setCursor(0,25);
        tft.print("Serial No. ");
        tft.setCursor(50,25);
        tft.print(kuSrno);
        tft.setCursor(0,40);
        tft.print("Batch No. ");
        tft.setCursor(50,40);
        tft.print(kuBno);
        tft.setCursor(0,55);
        tft.print("Model No. ");
        tft.setCursor(50,55);
        tft.print(modelNo);
        tft.setCursor(0,70);
        tft.print("Firmware ");
        tft.setCursor(50,70);
        tft.print(firmware);
        tft.setCursor(0,85);
        tft.print("Output");
        tft.setCursor(60,85);
        tft.print(outputNames[outputType]);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == ENTER){
        delay(400);
        menuFlagH = LOW;
        next--;
        break;
      }
      if(checkKeypress() == LEFT){
        delay(400);
        menuFlagH = LOW;
        next--;
        break;
      }
    /////////////////////////////////////////////////////////////////////////////////;
  }//while inc0 == 6  inc1 == 4
    /////////////////////    help ////////////////////////////////////////////
    while( (inc0 == 6) && (inc1 == 5)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("H e l p  ");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        selectMenuString(ARIAL_8,helpNames,inc2,3,selColor,fgColor,bgColor);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == DOWN){
        delay(400);
        inc2++;
        menuFlagH = LOW;
        if(inc2 > 2){
          inc2 = 2;
        }
      }
      if(checkKeypress() == UP){
        delay(400);
        inc2--;
        menuFlagH = LOW;
        if(inc2 <= 0){
          inc2 = 0;
        }
      }
      if(checkKeypress() == RIGHT){
        menuFlagH = LOW;
        next++;
        if(next == 5){
          next = 0;
        }
        break;
      }
      if(checkKeypress() == LEFT){
        delay(400);
        next--;
        inc2 = 0;
        menuFlagH = LOW;
        break;
      }
    /////////////////////////////////////////////////////////////////////////////////;
  }//while inc0 == 6  inc1 == 5
    /////////////////////    Fa ]ctory Reset ////////////////////////////////////////////
    while( (inc0 == 6) && (inc1 ==  6)){
    //  wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("F a c t o r y   R e s e t");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        tft.setFont(ARIAL_8);
        tft.setTextWrap(HIGH);
        tft.setTextSize(1);
        tft.setCursor(0,25);
        tft.print("Press Enter to perform    Factory Reset ");
        tft.setTextWrap(LOW);
        tft.setCursor(0,60);
        tft.print("Press Back to cancel");
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == ENTER){
        delay(400);
        tft.fillScreen(BLACK);
        tft.setFont(ARIAL_8);
        tft.setTextWrap(HIGH);
        tft.setTextSize(1);
        tft.setCursor(0,25);
        tft.print("Resetting all data...");
        delay(2000);
        factoryReset();
        menuFlagH = LOW;
        next--;
        break;
      }
      if(checkKeypress() == LEFT){
        delay(400);
        menuFlagH = LOW;
        next--;
        break;
      }
    /////////////////////////////////////////////////////////////////////////////////;
    }//while inc0 == 6  inc1 == 4
  } // while next = 2
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  tft.fillRect(0, 0, 128, 128, bgColor);
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // NEXT  = 3
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  inc3= 0;
  while((menuState == HIGH) && (next == 3)){
    //wd.checkin();
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //// Display Inside Relay Lower Upper and Manual Reset
    while( (inc0 == 1) && (inc1 < 4) && (inc2 < 2)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print(relay1Names[inc2]);
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        selectMenuString(ARIAL_8,relay2Names,inc3,2,selColor,fgColor,bgColor);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == DOWN){
        delay(400);
        inc3++;
        menuFlagH = LOW;
        if(inc3 > 2){
          inc3 = 0;
        }
      }
      if(checkKeypress() == UP){
        delay(400);
        inc3--;
        menuFlagH = LOW;
        if(inc3 <= 0){
          inc3 = 0;
        }
      }
      if(checkKeypress() == RIGHT){
        delay(400);
        next++;
        menuFlagH = LOW;
        if(next == 5){
         next = 0;
        }
        break;
      }
      if(checkKeypress() == LEFT){
        delay(400);
        menuFlagH = LOW;
        if(next > 0){
          next--;
        }
        break;
      }
    }//while inc0 == 1   inc1 == 0 to 4   inc2 = 0 to 1
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// Display Inside Relay 1 Manual Reset
    while( (inc0 == 1) && (inc1 < 4) && (inc2 == 2)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("M a n u a l   R s t");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        tft.setFont(ARIAL_12);
        tft.setTextColor(fgColor,bgColor);
        tft.setTextSize(1);
        tft.setCursor(10,54);
        tft.print(positionNames[inc3]);
        tft.setFont(GLCDFONT);
        tft.drawChar(charPos2[3],35,30,fgColor,bgColor,2);
        tft.drawChar(charPos2[3],75,31,fgColor,bgColor,2);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == UP){
         delay(400);
         inc3++;
         menuFlagH = LOW;
         if(inc3 > 1){
           inc3 = 0;
         }
      }
      if(checkKeypress() == DOWN){
         delay(400);
         inc3++;
         menuFlagH = LOW;
         if(inc3 > 1){
           inc3 = 0;
         }
      }
      if(checkKeypress() == LEFT){
         delay(400);
         next--;
         menuFlagH = LOW;
         inc3 = 0;
         break;
      }
      if(checkKeypress() == ENTER){
        delay(400);
        if(inc1 == 0){relay1.manRst = inc3;EEPROM.write(ADD_RLY1_MANRST,relay1.manRst);}
        if(inc1 == 1){relay2.manRst = inc3;EEPROM.write(ADD_RLY2_MANRST,relay2.manRst);}
        if(inc1 == 2){relay3.manRst = inc3;EEPROM.write(ADD_RLY3_MANRST,relay3.manRst);}
        if(inc1 == 3){relay4.manRst = inc3;EEPROM.write(ADD_RLY4_MANRST,relay4.manRst);}
        inc3 = 0;
        next--;
        menuFlagH = LOW;
        break;
       }
    /////////////////////////////////////////////////////////////////////////////////
    }//while end manual reset for all relays
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// Sector Lower and Upper Value Set
    if(inc1 == 0){
      if(inc2 == 0)hex2bcd(sector1.lowerSet);
      if(inc2 == 1)hex2bcd(sector1.upperSet);
    }
    if(inc1 == 1){
      if(inc2 == 0)hex2bcd(sector2.lowerSet);
      if(inc2 == 1)hex2bcd(sector2.upperSet);
    }
    if(inc1 == 2){
      if(inc2 == 0)hex2bcd(sector3.lowerSet);
      if(inc2 == 1)hex2bcd(sector3.upperSet);
    }
    if(inc1 == 3){
      if(inc2 == 0)hex2bcd(sector4.lowerSet);
      if(inc2 == 1)hex2bcd(sector4.upperSet);
    }
    tempVar[0] = thous - 48;
    tempVar[1] = hunds - 48;
    tempVar[2] = tens - 48;
    tempVar[3] = ones - 48;
    pos = 0;
    while( (inc0 == 2) && (inc1 < 4) && (inc2 < 2)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print(sector1Names[inc2]);
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        selectSpMenu(ARIAL_12_N,tempVar,charPos3,pos,fgColor,bgColor);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == UP){
          delay(400);
          tempVar[pos]++;
          menuFlagH = LOW;
          if( tempVar[pos] > 9){
            tempVar[pos] = 9;
          }
       }
       if(checkKeypress() == DOWN){
           delay(400);
           tempVar[pos]--;
           menuFlagH = LOW;
           if( tempVar[pos]< 0){
             tempVar[pos] = 0;
           }
        }
        if(checkKeypress() == RIGHT){
           delay(400);
           pos++;
           menuFlagH = LOW;
           if(pos > 3){
             pos = 3;
           }
        }
        if(checkKeypress() == LEFT){
           delay(400);
           pos--;
           menuFlagH = LOW;
           if(pos < 0){
             next--;
             break;
           }
        }
        if(checkKeypress() == ENTER){
          delay(400);
          if(inc1 == 0){
            if(inc2 == 0){
              sector1.lowerSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_SEC1_LOWERSET,sector1.lowerSet);
            }
            if(inc2 == 1){
              sector1.upperSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_SEC1_UPPERSET,sector1.upperSet);
            }
          }
          if(inc1 == 1){
            if(inc2 == 0){
              sector2.lowerSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_SEC2_LOWERSET,sector2.lowerSet);
            }
            if(inc2 == 1){
              sector2.upperSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_SEC2_UPPERSET,sector2.upperSet);
            }
          }
          if(inc1 == 2){
            if(inc2 == 0){
              sector3.lowerSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_SEC3_LOWERSET,sector3.lowerSet);
            }
            if(inc2 == 1){
              sector3.upperSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_SEC3_UPPERSET,sector3.upperSet);
            }
          }
          if(inc1 == 3){
            if(inc2 == 0){
              sector4.lowerSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_SEC4_LOWERSET,sector4.lowerSet);
            }
            if(inc2 == 1){
              sector4.upperSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_SEC4_UPPERSET,sector4.upperSet);
            }
          }
          next--;
          menuFlagH = LOW;
          break;
        }
    }//while inc0 == 1   inc1 == 2   inc2 = 0  inc 3 = 0
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// Sector colour Set
    while( (inc0 == 2) && (inc1 < 4) && (inc2 == 2)){
    //  wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("Colour       ");
      if(inc1 == 0)tft.print(colorNames[sector1.color]);
      if(inc1 == 1)tft.print(colorNames[sector2.color]);
      if(inc1 == 2)tft.print(colorNames[sector3.color]);
      if(inc1 == 3)tft.print(colorNames[sector4.color]);
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        tft.setFont(ARIAL_12);
        tft.setTextColor(fgColor,bgColor);
        tft.setTextSize(1);
        tft.setCursor(10,54);
        tft.print(colorNames[inc3]);
        tft.setFont(GLCDFONT);
        tft.drawChar(charPos2[3],35,30,fgColor,bgColor,2);
        tft.drawChar(charPos2[3],75,31,fgColor,bgColor,2);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == UP){
         delay(400);
         inc3++;
         menuFlagH = LOW;
         if(inc3 > 3){
           inc3 = 3;
         }
      }
      if(checkKeypress() == DOWN){
         delay(400);
         inc3--;
         menuFlagH = LOW;
         if(inc3 < 0){
           inc3 = 0;
         }
      }
      if(checkKeypress() == LEFT){
         delay(400);
         next--;
         menuFlagH = LOW;
         inc3 = 0;
         break;
      }
      if(checkKeypress() == ENTER){
        delay(400);
        if(inc1 == 0){
          sector1.color = inc3;
          EEPROM.put(ADD_SEC1_COLOR,sector1.color);
        }
        if(inc1 == 1){
          sector2.color = inc3;
          EEPROM.put(ADD_SEC2_COLOR,sector2.color);
        }
        if(inc1 == 2){
          sector3.color = inc3;
          EEPROM.put(ADD_SEC3_COLOR,sector3.color);
        }
        if(inc1 == 3){
          sector4.color = inc3;
          EEPROM.put(ADD_SEC4_COLOR,sector4.color);
        }
        inc3 = 0;
        next--;
        menuFlagH = LOW;
        break;
       }
    /////////////////////////////////////////////////////////////////////////////////
    }//while end Color Set for Sector
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// Display scroll
    //inc3 = dispScroll;
    while( (inc0 == 6) && (inc1 == 3) && (inc2 == 0)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("D i s p l a y  S c r o l l");
      tft.print(autoNames[dispScroll]);
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        tft.setFont(ARIAL_12);
        tft.setTextColor(fgColor,bgColor);
        tft.setTextSize(1);
        tft.setCursor(10,54);
        tft.print(autoNames[inc3]);
        tft.setFont(GLCDFONT);
        tft.drawChar(charPos2[3],35,30,fgColor,bgColor,2);
        tft.drawChar(charPos2[3],75,31,fgColor,bgColor,2);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == UP){
        delay(400);
        inc3++;
        if(inc3 > 1){
          inc3 = 1;
        }
        menuFlagH = LOW;
      }
      if(checkKeypress() == DOWN){
        delay(400);
        inc3--;
        menuFlagH = LOW;
        if(inc3 < 0){
         inc3 = 0;
        }
      }
      if(checkKeypress() == ENTER){
        menuFlagH = LOW;
        delay(400);
        dispScroll = inc3;
        EEPROM.write(ADD_DISPSCROLL_STS,dispScroll);
        next--;
        break;
      }
      if(checkKeypress() == LEFT){
        delay(400);
        menuFlagH = LOW;
        next--;
        inc3 = 0;
        break;
      }
    /////////////////////////////////////////////////////////////////////////////////
    }//while end Color Set for Sector
    //// Display scroll time
    hex2bcd(scrollTime);
    tempVar[0] = hunds - 48;
    tempVar[1] = tens - 48;
    tempVar[2] = ones - 48;
    pos = 0;
    while( (inc0 == 6) && (inc1 == 3) && (inc2 == 1)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("S c r o l l  T i m e  ");
      tft.print(scrollTime);
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
       /// selectTimeMenu(uint8_t fontName,char* displayString,char* charPositions,int positions,int foreColor, int backColor)
        selectTimeMenu(ARIAL_12_N,tempVar,charPos2,pos,fgColor, bgColor);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == UP){
       delay(400);
       tempVar[pos+1]++;
       menuFlagH = LOW;
           if( tempVar[pos+1] > 9)
           {
            tempVar[pos+1] = 9;
           }
       menuFlagH = LOW;
      }
      if(checkKeypress() == DOWN){
         delay(400);
         tempVar[pos+1]--;
         menuFlagH = LOW;
         if( tempVar[pos+1] < 0){
          tempVar[pos+1] = 0;
         }
      }
      if(checkKeypress() == ENTER){
       delay(400);
       menuFlagH = LOW;
       scrollTime = (tempVar[1] * 10)  + tempVar[2];
       EEPROM.write(ADD_SCAN_TIME,scrollTime);
       next--;
       break;
      }
      if(checkKeypress() == LEFT){
       delay(400);
       pos--;
       menuFlagH = LOW;
       if(pos < 0){
         next--;
         menuFlagH = LOW;
         break;
       }
      }
      if(checkKeypress() == RIGHT){
       delay(400);
       pos++;
       menuFlagH = LOW;
       if(pos > 1){
         pos = 1;
       }
      }
    /////////////////////////////////////////////////////////////////////////////////
    }//while end Display scroll time
    //// Help User Manual
    while( (inc0 == 6) && (inc1 == 5) && (inc2 == 0)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("U s e r  M a n u a l");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        tft.fillRect(15,15,90,90,WHITE);
        tft.drawBitmap(20, 20, BLEQR, 80, 80,BLACK);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == LEFT){
        delay(400);
        next--;
        menuFlagH = LOW;
        break;
       }
    /////////////////////////////////////////////////////////////////////////////////
    }//while end user Manual
    //// Help Operations Manual
    while( (inc0 == 6) && (inc1 == 5) && (inc2 == 1)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("O p e r  M a n u a l");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        tft.fillRect(15,15,90,90,WHITE);
        tft.drawBitmap(20, 20, BLEQR, 80, 80,BLACK);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == LEFT){
        delay(400);
        next--;
        menuFlagH = LOW;
        break;
       }
    /////////////////////////////////////////////////////////////////////////////////
    }//while end user Manual
    //// Help datasheet
    while( (inc0 == 6) && (inc1 == 5) && (inc2 == 2)){
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("D a t a s h e e t");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        tft.fillRect(15,15,90,90,WHITE);
        tft.drawBitmap(20, 20, BLEQR, 80, 80,BLACK);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == LEFT){
        delay(400);
        next--;
        menuFlagH = LOW;
        break;
       }
    /////////////////////////////////////////////////////////////////////////////////
    }//while end user Manual
  } /// while next == 3

  ///////////////////////////////////  End of next  = 3  /////////////////////////////////////////////////////////
  tft.fillRect(0, 0, 128, 128, bgColor);
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // NEXT  = 4
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  while( (menuState == HIGH) && (next == 4)  ){
    //wd.checkin();
    /////  Display inside the relay upper or lower set
    if(inc1 == 0){
      if(inc2 == 0)hex2bcd(relay1.lowerSet);
      if(inc2 == 1)hex2bcd(relay1.upperSet);
    }
    if(inc1 == 1){
      if(inc2 == 0)hex2bcd(relay2.lowerSet);
      if(inc2 == 1)hex2bcd(relay2.upperSet);
    }
    if(inc1 == 2){
      if(inc2 == 0)hex2bcd(relay3.lowerSet);
      if(inc2 == 1)hex2bcd(relay3.upperSet);
    }
    if(inc1 == 3){
      if(inc2 == 0)hex2bcd(relay4.lowerSet);
      if(inc2 == 1)hex2bcd(relay4.upperSet);
    }
    tempVar[0] = thous - 48;
    tempVar[1] = hunds - 48;
    tempVar[2] = tens - 48;
    tempVar[3] = ones - 48;
    pos = 0;
    while( (inc0 == 1) && (inc1 < 4) && (inc2 <2) && (inc3 == 0)){
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("S e t");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        selectSpMenu(ARIAL_12_N,tempVar,charPos3,pos,fgColor,bgColor);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == UP){
          delay(400);
          tempVar[pos]++;
          menuFlagH = LOW;
          if( tempVar[pos] > 9){
            tempVar[pos] = 9;
          }
       }
       if(checkKeypress() == DOWN){
           delay(400);
           tempVar[pos]--;
           menuFlagH = LOW;
           if( tempVar[pos]< 0){
             tempVar[pos] = 0;
           }
        }
        if(checkKeypress() == RIGHT){
           delay(400);
           pos++;
           menuFlagH = LOW;
           if(pos > 3){
             pos = 3;
           }
        }
        if(checkKeypress() == LEFT){
           delay(400);
           pos--;
           menuFlagH = LOW;
           if(pos < 0){
             next--;
             break;
           }
        }
        if(checkKeypress() == ENTER){
          delay(400);
          if(inc1 == 0){
            if(inc2 == 0){
              relay1.lowerSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_RLY1_LOWERSET,relay1.lowerSet);
            }
            if(inc2 == 1){
              relay1.upperSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_RLY1_UPPERSET,relay1.upperSet);
            }
          }
          if(inc1 == 1){
            if(inc2 == 0){
              relay2.lowerSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_RLY2_LOWERSET,relay2.lowerSet);
            }
            if(inc2 == 1){
              relay2.upperSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_RLY2_UPPERSET,relay2.upperSet);
            }
          }
          if(inc1 == 2){
            if(inc2 == 0){
              relay3.lowerSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_RLY3_LOWERSET,relay3.lowerSet);
            }
            if(inc2 == 1){
              relay3.upperSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_RLY3_UPPERSET,relay3.upperSet);
            }
          }
          if(inc1 == 3){
            if(inc2 == 0){
              relay4.lowerSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_RLY4_LOWERSET,relay4.lowerSet);
            }
            if(inc2 == 1){
              relay4.upperSet = (tempVar[0] * 1000) + (tempVar[1] * 100) + (tempVar[2] * 10) + tempVar[3];
              EEPROM.put(ADD_RLY4_UPPERSET,relay4.upperSet);
            }
          }
          next--;
          menuFlagH = LOW;
          break;
        }
    }//while all relay upper and lower set
    //////////////////////////////////////////////////////////////////////////////////////////////
    ////    Display Inside Relay lower or upper delay
    if(inc1 == 0){
      if(inc2 == 0)hex2bcd(relay1.lowerDelay);
      if(inc2 == 1)hex2bcd(relay1.upperDelay);
    }
    if(inc1 == 1){
      if(inc2 == 0)hex2bcd(relay2.lowerDelay);
      if(inc2 == 1)hex2bcd(relay2.upperDelay);
    }
    if(inc1 == 2){
      if(inc2 == 0)hex2bcd(relay3.lowerDelay);
      if(inc2 == 1)hex2bcd(relay3.upperDelay);
    }
    if(inc1 == 3){
      if(inc2 == 0)hex2bcd(relay4.lowerDelay);
      if(inc2 == 1)hex2bcd(relay4.upperDelay);
    }
    tempVar[0] = hunds - 48;
    tempVar[1] = tens - 48;
    tempVar[2] = ones - 48;
    pos = 0;
    while( (inc0 == 1) && (inc1 < 4) && (inc2 <2) && (inc3 == 1)){
      //wd.checkin();
      if(Particle.connected){
        Particle.process();
      }
      tft.setFont(ARIAL_8);
      tft.setTextColor(fgColor,bgColor);
      tft.setTextSize(1);
      tft.setCursor(0,0);
      tft.print("D e l a y");
      tft.drawLine(0,13,128,13,fgColor);
      if(menuFlagH == LOW){
        selectTimeMenu(ARIAL_12_N,tempVar,charPos2,pos,fgColor, bgColor);
        menuFlagH = HIGH;
      }
      ///////////////////////   Button Read Condition   ///////////////////////////
      if(checkKeypress() == UP){
          delay(400);
          tempVar[pos+1]++;
          menuFlagH = LOW;
          if( tempVar[pos+1] > 9){
            tempVar[pos+1] = 9;
          }
       }
       if(checkKeypress() == DOWN){
           delay(400);
           tempVar[pos+1]--;
           menuFlagH = LOW;
           if(tempVar[pos+1]< 0){
             tempVar[pos+1] = 0;
           }
        }
        if(checkKeypress() == RIGHT){
           delay(400);
           pos++;
           menuFlagH = LOW;
           if(pos > 1){
             pos = 1;
           }
        }
        if(checkKeypress() == LEFT){
           delay(400);
           pos--;
           menuFlagH = LOW;
           if(pos < 0){
             next--;
             break;
           }
        }
        if(checkKeypress() == ENTER){
          delay(400);
          if(inc1 == 0){
            if(inc2 == 0){
              relay1.lowerDelay = (tempVar[1] * 10) + tempVar[2];
              EEPROM.put(ADD_RLY1_LOWERDEL,relay1.lowerDelay);
            }
            if(inc2 == 1){
              relay1.upperDelay = (tempVar[1] * 10) + tempVar[2];
              EEPROM.put(ADD_RLY1_UPPERDEL,relay1.upperDelay);
            }
          }
          if(inc1 == 1){
            if(inc2 == 0){
              relay2.lowerDelay = (tempVar[1] * 10) + tempVar[2];
              EEPROM.put(ADD_RLY2_LOWERDEL,relay2.lowerDelay);
            }
            if(inc2 == 1){
              relay2.upperDelay = (tempVar[1] * 10) + tempVar[2];
              EEPROM.put(ADD_RLY2_UPPERDEL,relay2.upperDelay);
            }
          }
          if(inc1 == 2){
            if(inc2 == 0){
              relay3.lowerDelay = (tempVar[1] * 10) + tempVar[2];
              EEPROM.put(ADD_RLY3_LOWERDEL,relay3.lowerDelay);
            }
            if(inc2 == 1){
              relay3.upperDelay = (tempVar[1] * 10) + tempVar[2];
              EEPROM.put(ADD_RLY3_UPPERDEL,relay3.upperDelay);
            }
          }
          if(inc1 == 3){
            if(inc2 == 0){
              relay4.lowerDelay = (tempVar[1] * 10) + tempVar[2];
              EEPROM.put(ADD_RLY4_LOWERDEL,relay4.lowerDelay);
            }
            if(inc2 == 1){
              relay4.upperDelay = (tempVar[1] * 10) + tempVar[2];
              EEPROM.put(ADD_RLY4_UPPERDEL,relay4.upperDelay);
            }
          }
          next--;
          menuFlagH = LOW;
          break;
        }
      }//while all relay upper and lower delay
     }/// end while next = 4
  ///////////////////////////////////  End of  Menu  /////////////////////////////////////////////////////////
  }// while menuState = HIGH


} ///  End of code
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bar Graph Functions.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displayBargraph(uint8_t value){
  Wire.beginTransmission(0x6C);
  Wire.write(value);
  Wire.write(20);
  Wire.endTransmission();
      // wait 5 seconds for next scan
  Wire.beginTransmission(0x6C);
  Wire.endTransmission();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Key press Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t checkKeypress(){
  uint8_t c = 0;
  Wire.beginTransmission(0x6C);
  Wire.requestFrom(0x6C,1);    // request 6 bytes from slave device #2
  while(Wire.available()){
    c= Wire.read();    // receive a byte as character
  }
  Wire.endTransmission();
  if(c == 0) return 0;
  if (c & ENTER_BUT) { /*Serial.println("Enter Key Pressed");*/return 1 ;}
  if (c & RIGHT_BUT) { /*Serial.println("Right Key Pressed");*/return 2;}
  if (c & LEFT_BUT) {/*Serial.println("Left Key Pressed");*/return 3;}
  if (c & DOWN_BUT){/*Serial.println("Down Key Pressed");*/return 4;}
  if (c & UP_BUT) {/*Serial.println("Up Key Pressed");*/return 5;}
}
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
   tft.drawFastHLine(0,75,128,HomeScreenColor);
   tft.drawFastHLine(0,76,128,HomeScreenColor);
   tft.setFont(ARIAL_12);
   tft.setCursor(30,85);
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
    tft.drawFastHLine(0,75,128,HomeScreenColor);
    tft.drawFastHLine(0,76,128,HomeScreenColor);
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
    tft.setFont(ARIAL_12_N);
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
 tft.drawFastHLine(0,75,128,HomeScreenColor);
 tft.drawFastHLine(0,76,128,HomeScreenColor);
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
  if((value >= sector1.lowerSet) && (value <= sector1.upperSet)){
    return colorValues[sector1.color];
  }
  else if((value >= sector2.lowerSet) && (value <= sector2.upperSet)){
      return colorValues[sector2.color];
  }
  else if((value >= sector3.lowerSet) && (value <= sector3.upperSet)){
    return colorValues[sector3.color];
  }
  else if((value >= sector4.lowerSet) && (value <= sector4.upperSet)){
    return colorValues[sector4.color];
  }
  else{
    return BLACK;
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
    wifiStatus = EEPROM.read(ADD_WIFI_STATUS);
    EEPROM.get(ADD_ADC_CAL_0,calAdc[0]);
    EEPROM.get(ADD_ADC_CAL_1,calAdc[1]);
    EEPROM.get(ADD_DISP_CAL_0,calDisp[0]);
    EEPROM.get(ADD_DISP_CAL_1,calDisp[1]);
    EEPROM.get(ADD_KUSRNO,kuSrno);
    EEPROM.get(ADD_KUBATCHNO,kuBno);
    EEPROM.get(ADD_TCSSRNO,tcsSrno);
    EEPROM.get(ADD_TCSBATCHNO,tcsBno);
    EEPROM.get(ADD_PASS_ADMIN,adminPass);
    EEPROM.get(ADD_PASS_USER,userPass);
    EEPROM.get(ADD_MODELNO,modelNo);
    EEPROM.get(ADD_FIRMWARE,firmware);
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
    tempInt = inString.substring(3, index[0]).toInt();
    if(rangeHigh != tempInt){
      settingsChangeLog("Range Span Value ",String(rangeHigh),String(tempInt));
    }
    rangeHigh = tempInt;
    EEPROM.put(ADD_RANGEH,rangeHigh);
    // Range Low
    tempString = inString.substring(index[0]+1, length);
    index[0] = tempString.indexOf(',');
    tempInt = tempString.substring(0, index[0]).toInt();
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
    tempRelay[0] = inString.substring(3, index[0]).toInt();
    tempString = inString.substring(index[0]+1, length);
    index[0] = tempString.indexOf(',');
    tempRelay[1] = tempString.substring(0, index[0]).toInt();
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
    tempRelay[0] = inString.substring(3, index[0]).toInt();
    tempString = inString.substring(index[0]+1, length);
    index[0] = tempString.indexOf(',');
    tempRelay[1] = tempString.substring(0, index[0]).toInt();
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
      EEPROM.put(ADD_SEC1_COLOR,(uint8_t)sector1.color);
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
      EEPROM.put(ADD_SEC2_COLOR,(uint8_t)sector2.color);
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
      EEPROM.put(ADD_SEC3_COLOR,(uint8_t)sector3.color);
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
      EEPROM.put(ADD_SEC4_COLOR,(uint8_t)sector4.color);
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
  //Edit Device Master
  if(inString.charAt(0) == 'E'){
    int length = inString.length();
    index[0] = inString.indexOf(',',2);
    string1 = inString.substring(2, index[0]);
    string1.toCharArray(kuSrno, 10);
    tempString = inString.substring(index[0]+1, length);
    index[0] = tempString.indexOf(',');
    string1  = tempString.substring(0, index[0]);
    string1.toCharArray(kuBno, 10);
    tempString = tempString.substring(index[0]+1,length);
    index[0] = tempString.indexOf(',');
    string1  = tempString.substring(0, index[0]);
    string1.toCharArray(tcsSrno, 10);
    tempString = tempString.substring(index[0]+1,length);
    index[0] = tempString.indexOf(',');
    string1 = tempString.substring(0, index[0]);
    string1.toCharArray(tcsBno, 10);
    tempString = tempString.substring(index[0]+1,length);
    index[0] = tempString.indexOf(',');
    string1  = tempString.substring(0, index[0]);
    string1.toCharArray(adminPass, 10);
    tempString = tempString.substring(index[0]+1,length);
    index[0] = tempString.indexOf(',');
    string1  = tempString.substring(0, index[0]);
    string1.toCharArray(userPass, 10);
    tempString = tempString.substring(index[0]+1,length);
    index[0] = tempString.indexOf(',');
    string1  = tempString.substring(0, index[0]);
    string1.toCharArray(modelNo, 10);
    string1  = tempString.substring(index[0]+1,length);
    string1.toCharArray(firmware, 10);
    EEPROM.put(ADD_KUSRNO,kuSrno);
    EEPROM.put(ADD_KUBATCHNO,kuBno);
    EEPROM.put(ADD_TCSSRNO,tcsSrno);
    EEPROM.put(ADD_TCSBATCHNO,tcsBno);
    EEPROM.put(ADD_PASS_ADMIN,adminPass);
    EEPROM.put(ADD_PASS_USER,userPass);
    EEPROM.put(ADD_MODELNO,modelNo);
    EEPROM.put(ADD_FIRMWARE,firmware);
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
    EEPROM.write(ADD_WIFI_STATUS,wifiStatus);
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Factory Reset condition
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void factoryReset(){
  EEPROM.write(ADD_UNIT,0);
  EEPROM.write(ADD_SCREEN,2);
  EEPROM.put(ADD_RANGEH,(int)100);
  EEPROM.put(ADD_RANGEL,(int)0);
  EEPROM.write(ADD_WIFI_STATUS,1);
  EEPROM.write(ADD_DATALOG_STS,1);
  EEPROM.write(ADD_DATALOG_TIME,1);
  EEPROM.put(ADD_RLY1_UPPERSET,(uint16_t)20);
  EEPROM.put(ADD_RLY1_LOWERSET,(uint16_t)10);
  EEPROM.put(ADD_RLY1_UPPERDEL,(uint16_t)1);
  EEPROM.put(ADD_RLY1_LOWERDEL,(uint16_t)1);
  EEPROM.put(ADD_RLY2_UPPERSET,(uint16_t)40);
  EEPROM.put(ADD_RLY2_LOWERSET,(uint16_t)30);
  EEPROM.put(ADD_RLY2_UPPERDEL,(uint16_t)2);
  EEPROM.put(ADD_RLY2_LOWERDEL,(uint16_t)2);
  EEPROM.put(ADD_RLY3_UPPERSET,(uint16_t)60);
  EEPROM.put(ADD_RLY3_LOWERSET,(uint16_t)50);
  EEPROM.put(ADD_RLY3_UPPERDEL,(uint16_t)1);
  EEPROM.put(ADD_RLY3_LOWERDEL,(uint16_t)1);
  EEPROM.put(ADD_RLY4_UPPERSET,(uint16_t)90);
  EEPROM.put(ADD_RLY4_LOWERSET,(uint16_t)80);
  EEPROM.put(ADD_RLY4_UPPERDEL,(uint16_t)0);
  EEPROM.put(ADD_RLY4_LOWERDEL,(uint16_t)0);
  EEPROM.put(ADD_SEC1_LOWERSET,(uint16_t)0);
  EEPROM.put(ADD_SEC1_UPPERSET,(uint16_t)20);
  EEPROM.put(ADD_SEC1_COLOR,(uint8_t)0);
  EEPROM.put(ADD_SEC2_LOWERSET,(uint16_t)21);
  EEPROM.put(ADD_SEC2_UPPERSET,(uint16_t)50);
  EEPROM.put(ADD_SEC2_COLOR,(uint8_t)1);
  EEPROM.put(ADD_SEC3_LOWERSET,(uint16_t)51);
  EEPROM.put(ADD_SEC3_UPPERSET,(uint16_t)80);
  EEPROM.put(ADD_SEC3_COLOR,(uint8_t)2);
  EEPROM.put(ADD_SEC4_LOWERSET,(uint16_t)81);
  EEPROM.put(ADD_SEC4_UPPERSET,(uint16_t)100);
  EEPROM.put(ADD_SEC4_COLOR,(uint8_t)3);
  EEPROM.write(ADD_WIFI_STATUS,1);
  System.reset();
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
    EEPROM.write(ADD_WIFI_STATUS,wifiStatus);
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
    Serial.print("KU Serial No : ");
    Serial.println((kuSrno));
    Serial.print("KU Batch No : ");
    Serial.println((kuBno));
    Serial.print("TCS Serial No : ");
    Serial.println((tcsSrno));
    Serial.print("TCS Batch No : ");
    Serial.println((tcsBno));
    Serial.print("Model Number : ");
    Serial.println((modelNo));
    Serial.print("Firmware Version : ");
    Serial.println((firmware));
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
  Serial1.print("KSNO=");
  Serial1.print(kuSrno);
  Serial1.print(",");
  Serial1.print("KBNO=");
  Serial1.print(kuBno);
  Serial1.print(",");
  Serial1.print("TSNO=");
  Serial1.print(tcsSrno);
  Serial1.print(",");
  Serial1.print("TBNO=");
  Serial1.print(tcsBno);
  Serial1.print(",");
  Serial1.print("PASSSUPER=");
  Serial1.print(superPass);
  Serial1.print(",");
  Serial1.print("PASSADMIN=");
  Serial1.print(adminPass);
  Serial1.print(",");
  Serial1.print("PASSUSER=");
  Serial1.print(userPass);
  Serial1.print(",");
  Serial1.print("MODE=");
  Serial1.print(mode);
  Serial1.print(",");
  Serial1.print("UNIT=");
  Serial1.print(pvUnit);
  Serial1.print(",");
  Serial1.print("SCREEN=");
  Serial1.print(setScreen);
  Serial1.print(",");
  Serial1.print("~");

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
  myFile.print(mode1Names[mode]);
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Arduino map Function but as FLoat
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float mapf(float x, float in_min, float in_max, float out_min, float out_max){
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Bluetooth mode Selection
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void bluetoothCon(){
  pinMode(BTAT,INPUT_PULLDOWN);
  if(digitalRead(BTAT)){
    tft.drawBitmap(60, 110, bluetooth, 16, 16,BLUE);//display bluettoth ICON
  }
  else{
    tft.drawBitmap(60, 110, bluetooth, 16, 16,BLACK);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Select Menu String from a list
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void selectMenuString(uint8_t fontName,char** displayString,int selectedString,int maxValue,int changeColor,int foreColor, int backColor)
{
  int i;
  tft.setFont(fontName);
  for(i = 0;i < selectedString;i++){
   tft.fillTriangle(118, (i +1)*16, 118, ((i +1)*16)+6, 128, ((i +1)*16)+3, backColor);
   tft.setTextColor(foreColor,bgColor);
   tft.setTextSize(1);
   tft.setCursor(12, (i +1)*16 );
   tft.print(displayString[i]);
   tft.fillTriangle(8, (i +1)*16, 8, ((i +1)*16)+6, 0, ((i +1)*16)+3, backColor);
  }

  tft.fillTriangle(118, (i +1)*16, 118, ((i +1)*16)+6, 128, ((i +1)*16)+3, changeColor);
  tft.setTextColor(changeColor,backColor);
  tft.setTextSize(1);
  tft.setCursor(12, (selectedString+1) *16);
  tft.print(displayString[selectedString]);
  tft.fillTriangle(8, (i +1)*16, 8, ((i +1)*16)+6, 0, ((i +1)*16)+3, changeColor);

  for(i = selectedString+1;i < maxValue;i++)  {
  tft.fillTriangle(118, (i +1)*16, 118, ((i +1)*16)+6, 128, ((i +1)*16)+3, backColor);
  tft.setTextColor(foreColor,backColor);
  tft.setTextSize(1);
  tft.setCursor(12, (i + 1)*16 );
  tft.print(displayString[i]);
  tft.fillTriangle(8, (i +1)*16, 8, ((i +1)*16)+6, 0, ((i +1)*16)+3, backColor);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Select Time based Units
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void selectTimeMenu(uint8_t fontName,int*  displayString,char*  charPositions,int positions,int foreColor, int backColor)
{
  tft.setFont(fontName);
  tft.drawChar(45,54,displayString[1]+48,foreColor,bgColor,1);
  tft.drawChar(60,54,displayString[2]+48,foreColor,bgColor,1);

  tft.setFont(GLCDFONT);
        for(i=0; i < positions  ; i++)
        {
           tft.drawChar(charPositions[i+3],35,30,backColor,backColor,2);
           tft.drawChar(charPositions[i+3],75,31,backColor,backColor,2);
        }

  tft.drawChar(charPositions[positions+3],35,30,foreColor,backColor,2);
  tft.drawChar(charPositions[positions+3],75,31,foreColor,backColor,2);

        for(i=positions + 1  ; i < 2; i++)
        {
          tft.drawChar(charPositions[i+3],35,30,backColor,backColor,2);
          tft.drawChar(charPositions[i+3],75,31,backColor,backColor,2);
        }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Select Set points or 3.0 digit based Units
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void selectSpMenu(uint8_t fontName,int*  displayString,char*  charPositions,int positions,int foreColor, int backColor)
{
  tft.setFont(fontName);
  tft.drawChar(charPositions[0],54,displayString[0]+48,foreColor,backColor,1);
  tft.drawChar(charPositions[1],54,displayString[1]+48,foreColor,backColor,1);
  tft.drawChar(charPositions[2],54,displayString[2]+48,foreColor,backColor,1);
  tft.drawChar(charPositions[3]-15,54,'.',foreColor,backColor,1);
  tft.drawChar(charPositions[3],54,displayString[3]+48,foreColor,backColor,1);
  tft.setFont(GLCDFONT);
        for(i=0; i < positions ; i++)
        {
           tft.drawChar(charPositions[i],35,30,backColor,backColor,2);
           tft.drawChar(charPositions[i],75,31,backColor,backColor,2);
        }

  tft.drawChar(charPositions[positions],35,30,foreColor,backColor,2);
  tft.drawChar(charPositions[positions],75,31,foreColor,backColor,2);

        for(i=positions + 1  ; i < 4 ; i++)
        {
          tft.drawChar(charPositions[i],35,30,backColor,backColor,2);
          tft.drawChar(charPositions[i],75,31,backColor,backColor,2);
        }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///  Convert time and date to unix time stamp
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long getTimestamp(int year, int month, int day, int hour, int minute, int second) {
  unsigned long secMinute = 60;
  unsigned long secHour = 60 * secMinute;
  unsigned long secDay = 24 * secHour;
  unsigned long secYear = 365 * secDay;
  int dayMonth[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

  // leap years
  int leapDays = (year >> 2) + 1;
  if (((year & 3) == 0) && month < 3)
    leapDays--;

  // calculate
  return year * secYear + (dayMonth[month-1] + (day-1) + leapDays) * secDay + hour * secHour + minute * secMinute + second + T2000UTC;
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
