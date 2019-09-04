#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <algorithm>
#include <string>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "FastLED.h"
#include <LedControl.h>
#include <ArduinoJson.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define NUM_PANELS 4
#define CLK_PIN   D5
#define DATA_PIN  D7
#define CS_PIN    D4
#define CHAR_SPACING  1 
#define BUF_SIZE  75
#define NUM_LEDS 30  // # of LEDS in the strip
#define LED_DATA 3 // Output Pin to Data Line on Strip

int SCROLL_SPEED = 10; // Will want to change this for different messages.
int brightness = 0;
int fadeAmount = 5;
unsigned long timestamp=0;
unsigned long interval=80;
const char* ssid = "yourssid";
const char* password = "yourpwd";
double curr_price = -1.0;
unsigned long bufferLong[14] = {0};
unsigned char connectText[] PROGMEM ={"Connecting..."};
unsigned char emptySpace[] PROGMEM = {"      "};
unsigned char connectedText[] PROGMEM ={"Connected! "};
unsigned char errorParsing[] PROGMEM ={"Error parsing JSON object."};
unsigned char tickerText[100] ={""};

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, NUM_PANELS);
CRGB leds[NUM_LEDS];
LedControl lc = LedControl(D7, D5, D4, NUM_PANELS);
DynamicJsonBuffer jsonBuffer;

void loop() {
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin("http://btcprice-lyons.herokuapp.com/ticker");  //Specify request destination
    int httpCode = http.GET(); //Send the request

    if (httpCode > 0) { //Check the returning code
      String responseString = http.getString();   //Get the request response payload
      JsonArray& root = jsonBuffer.parseArray(responseString);

      // Test if parsing succeeds.
      if (!root.success()) {
        scrollMessage(errorParsing);
        return;
      }

      //Just hardcode instead of looping for now.
      String btcPrice = root[0]["level"];
      String sp500level = root[1]["level"];
      String sp500valueChange = root[1]["value_change"];
      String sp500percentChange = root[1]["percent_change"];
      String sp500color = root[1]["color"];
      String nasdaqLevel = root[2]["level"];
      String nasdaqValueChange = root[2]["value_change"];
      String nasdaqPercentChange = root[2]["percent_change"];
      String nasdaqColor = root[2]["color"];
      String dowLevel = root[3]["level"];
      String dowValueChange = root[3]["value_change"];
      String dowPercentChange = root[3]["percent_change"];
      String dowColor = root[3]["color"];

      //S&P500 data parsing
      String sp500readout = "S&P 500 FUTURES  " + sp500level + "  (" + sp500valueChange + " / " + sp500percentChange + ")";
      clearBuffer();
      for(int i = 0; i < sp500readout.length(); i++) {
        tickerText[i] = sp500readout[i];
      }
      scrollMessage(emptySpace);
      scrollMessage(tickerText);

      if(sp500color == "red") {
        setRed();
      } else {
        setGreen();
      }

      //Nasdaq data parsing
      String nasdaqReadout = "NASDAQ FUTURES  " + nasdaqLevel + "  (" + nasdaqValueChange + " / " + nasdaqPercentChange + ")";
      clearBuffer();
      for(int i = 0; i < nasdaqReadout.length(); i++) {
        tickerText[i] = nasdaqReadout[i];
      }
      
      scrollMessage(emptySpace);
      scrollMessage(tickerText);

      if(nasdaqColor == "red") {
        setRed();
      } else {
        setGreen();
      }

      //Dow data parsing
      String dowReadout = "DJI FUTURES  " + dowLevel + "  (" + dowValueChange + " / " + dowPercentChange + ")";
      clearBuffer();
      for(int i = 0; i < dowReadout.length(); i++) {
        tickerText[i] = dowReadout[i];
      }
      
      scrollMessage(emptySpace);
      scrollMessage(tickerText);

      if(dowColor == "red") {
        setRed();
      } else {
        setGreen();
      }

      //BTC/USD data parsing
      String btcPriceIsolated = btcPrice;
      btcPriceIsolated.replace(",", "");
      double btcPriceDouble = btcPriceIsolated.toDouble();
      String btcPriceReadout = "BTC/USD  $" + btcPrice;

      clearBuffer();
      for(int i = 0; i < btcPriceReadout.length(); i++) {
        tickerText[i] = btcPriceReadout[i];
      }
      
      scrollMessage(emptySpace);
      scrollMessage(tickerText);

      //BTC price color change will just refer to changes from last read
      if(btcPriceDouble > curr_price) {   
        setGreen();
      } else if (btcPriceDouble < curr_price) {
        setRed();
      }

      curr_price = btcPriceDouble;
    }
 
    http.end();   //Close connection
  }
 
}

void clearBuffer() {
  for(int i = 0; i < 100; i++) {
    tickerText[i] = (char) 0;
  }
}

void setup () {
  for (int x = 0; x < NUM_PANELS; x++) {
    lc.shutdown(x,false);       //The MAX72XX is in power-saving mode on startup
    lc.setIntensity(x,8);       // Set the brightness to default value
    lc.clearDisplay(x);         // and clear the display
  }

  FastLED.addLeds<NEOPIXEL, LED_DATA>(leds, NUM_LEDS);
  scrollMessage(connectText);
  WiFi.begin(ssid, password);
  setWhite();
  
  while (WiFi.status() != WL_CONNECTED)  {
    for(int i = 0; i < NUM_LEDS; i++ ) {
     leds[i].setRGB(255, 255, 255);  // Set Color HERE!!!
     leds[i].fadeLightBy(brightness);
    }

    FastLED.show();
    brightness = brightness + fadeAmount;
    if(brightness == 0 || brightness == 255) {
      fadeAmount = -fadeAmount ; 
    }

    delay(9);
  }

  scrollMessage(emptySpace);
  scrollMessage(connectedText);
  delay(3000);
  
  SCROLL_SPEED = 25;

}

void clearMXPanels() {
  for (int x = 0; x < NUM_PANELS; x++) {
    lc.clearDisplay(x);         // and clear the display
  }
}

void printText(uint8_t modStart, uint8_t modEnd, char *pMsg) {
  uint8_t   state = 0;
  uint8_t   curLen;
  uint16_t  showLen;
  uint8_t   cBuf[8];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     // finite state machine to print the characters in the space available
  {
    switch(state)
    {
      case 0: // Load the next character from the font table
        // if we reached end of message, reset the message pointer
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
          state = 2;
          break;
        }

        // retrieve the next character form the font file
        showLen = mx.getChar(*pMsg++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;
        // !! deliberately fall through to next state to start displaying

      case 1: // display the next part of the character
        mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;

      case 2: // initialize state for displaying empty columns
        curLen = 0;
        state++;
        // fall through

      case 3:  // display inter-character spacing or end of message padding (blank columns)
        mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen)
          state = 0;
        break;

      default:
        col = -1;   // this definitely ends the do loop
    }
  } while (col >= (modStart * COL_SIZE));

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void setGreen() {
  int brightness = 255;
  while (brightness > 0)  {
    for(int i = 0; i < NUM_LEDS; i++ ) {
     leds[i].setRGB(0, 255, 0);  // Set Color HERE!!!
     leds[i].fadeLightBy(brightness);
    }

    FastLED.show();
    brightness = brightness - 25;
    delay(20);
  }
}

void setWhite() {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    leds[i].setRGB(255, 255, 255);
  }
  FastLED.show();
}

void setRed() {
  int brightness = 255;
  while (brightness > 0)  {
    for(int i = 0; i < NUM_LEDS; i++ ) {
     leds[i].setRGB(255, 0, 0);  // Set Color HERE!!!
     leds[i].fadeLightBy(brightness);
    }

    FastLED.show();
    brightness = brightness - 25;
    delay(20);
  }
}

// Scrolling text stuff. Not sure what any of this is but it works.

const unsigned char font5x7 [] PROGMEM = {      //Numeric Font Matrix (Arranged as 7x font data + 1x kerning data)
    B00000000,  //Space (Char 0x20)
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    3,//cambias el tamaño del espacio entre letras
 
    B01000000,  //!
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B00000000,
    B01000000,
    2,
 
    B10100000,  //"
    B10100000,
    B10100000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    4,
 
    B01010000,  //#
    B01010000,
    B11111000,
    B01010000,
    B11111000,
    B01010000,
    B01010000,
    6,
 
    B00100000,  //$
    B01111000,
    B10100000,
    B01110000,
    B00101000,
    B11110000,
    B00100000,
    6,
 
    B11000000,  //%
    B11001000,
    B00010000,
    B00100000,
    B01000000,
    B10011000,
    B00011000,
    6,
 
    B01100000,  //&
    B10010000,
    B10100000,
    B01000000,
    B10101000,
    B10010000,
    B01101000,
    6,
 
    B11000000,  //'
    B01000000,
    B10000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    3,
 
    B00100000,  //(
    B01000000,
    B10000000,
    B10000000,
    B10000000,
    B01000000,
    B00100000,
    4,
 
    B10000000,  //)
    B01000000,
    B00100000,
    B00100000,
    B00100000,
    B01000000,
    B10000000,
    4,
 
    B00000000,  //*
    B00100000,
    B10101000,
    B01110000,
    B10101000,
    B00100000,
    B00000000,
    6,
 
    B00000000,  //+
    B00100000,
    B00100000,
    B11111000,
    B00100000,
    B00100000,
    B00000000,
    6,
 
    B00000000,  //,
    B00000000,
    B00000000,
    B00000000,
    B11000000,
    B01000000,
    B10000000,
    3,
 
    B00000000,  //-
    B00000000,
    B11111000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    6,
 
    B00000000,  //.
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B11000000,
    B11000000,
    3,
 
    B00000000,  ///
    B00001000,
    B00010000,
    B00100000,
    B01000000,
    B10000000,
    B00000000,
    6,
 
    B01110000,  //0
    B10001000,
    B10011000,
    B10101000,
    B11001000,
    B10001000,
    B01110000,
    6,
 
    B01000000,  //1
    B11000000,
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B11100000,
    4,
 
    B01110000,  //2
    B10001000,
    B00001000,
    B00010000,
    B00100000,
    B01000000,
    B11111000,
    6,
 
    B11111000,  //3
    B00010000,
    B00100000,
    B00010000,
    B00001000,
    B10001000,
    B01110000,
    6,
 
    B00010000,  //4
    B00110000,
    B01010000,
    B10010000,
    B11111000,
    B00010000,
    B00010000,
    6,
 
    B11111000,  //5
    B10000000,
    B11110000,
    B00001000,

    B00001000,
    B10001000,
    B01110000,
    6,
 
    B00110000,  //6
    B01000000,
    B10000000,
    B11110000,
    B10001000,
    B10001000,
    B01110000,
    6,
 
    B11111000,  //7
    B10001000,
    B00001000,
    B00010000,
    B00100000,
    B00100000,
    B00100000,
    6,
 
    B01110000,  //8
    B10001000,
    B10001000,
    B01110000,
    B10001000,
    B10001000,
    B01110000,
    6,
 
    B01110000,  //9
    B10001000,
    B10001000,
    B01111000,
    B00001000,
    B00010000,
    B01100000,
    6,
 
    B00000000,  //:
    B11000000,
    B11000000,
    B00000000,
    B11000000,
    B11000000,
    B00000000,
    3,
 
    B00000000,  //;
    B11000000,
    B11000000,
    B00000000,
    B11000000,
    B01000000,
    B10000000,
    3,
 
    B00010000,  //<
    B00100000,
    B01000000,
    B10000000,
    B01000000,
    B00100000,
    B00010000,
    5,
 
    B00000000,  //=
    B00000000,
    B11111000,
    B00000000,
    B11111000,
    B00000000,
    B00000000,
    6,
 
    B10000000,  //>
    B01000000,
    B00100000,
    B00010000,
    B00100000,
    B01000000,
    B10000000,
    5,
 
    B01110000,  //?
    B10001000,
    B00001000,
    B00010000,
    B00100000,
    B00000000,
    B00100000,
    6,
 
    B01110000,  //@
    B10001000,
    B00001000,
    B01101000,
    B10101000,
    B10101000,
    B01110000,
    6,
 
    B01110000,  //A
    B10001000,
    B10001000,
    B10001000,
    B11111000,
    B10001000,
    B10001000,
    6,
 
    B11110000,  //B
    B10001000,
    B10001000,
    B11110000,
    B10001000,
    B10001000,
    B11110000,
    6,
 
    B01110000,  //C
    B10001000,
    B10000000,
    B10000000,
    B10000000,
    B10001000,
    B01110000,
    6,
 
    B11100000,  //D
    B10010000,
    B10001000,
    B10001000,
    B10001000,
    B10010000,
    B11100000,
    6,
 
    B11111000,  //E
    B10000000,
    B10000000,
    B11110000,
    B10000000,
    B10000000,
    B11111000,
    6,
 
    B11111000,  //F
    B10000000,
    B10000000,
    B11110000,
    B10000000,
    B10000000,
    B10000000,
    6,
 
    B01110000,  //G
    B10001000,
    B10000000,
    B10111000,
    B10001000,
    B10001000,
    B01111000,
    6,
 
    B10001000,  //H
    B10001000,
    B10001000,
    B11111000,
    B10001000,
    B10001000,
    B10001000,
    6,
 
    B11100000,  //I
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B11100000,
    4,
 
    B00111000,  //J
    B00010000,
    B00010000,
    B00010000,
    B00010000,
    B10010000,
    B01100000,
    6,
 
    B10001000,  //K
    B10010000,
    B10100000,
    B11000000,
    B10100000,
    B10010000,
    B10001000,
    6,
 
    B10000000,  //L
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B11111000,
    6,
 
    B10001000,  //M
    B11011000,
    B10101000,
    B10101000,
    B10001000,
    B10001000,
    B10001000,
    6,
 
    B10001000,  //N
    B10001000,
    B11001000,
    B10101000,
    B10011000,
    B10001000,
    B10001000,
    6,
 
    B01110000,  //O
    B10001000,
    B10001000,
    B10001000,
    B10001000,
    B10001000,
    B01110000,
    6,
 
    B11110000,  //P
    B10001000,
    B10001000,
    B11110000,
    B10000000,
    B10000000,
    B10000000,
    6,
 
    B01110000,  //Q
    B10001000,
    B10001000,
    B10001000,
    B10101000,
    B10010000,
    B01101000,
    6,
 
    B11110000,  //R
    B10001000,
    B10001000,
    B11110000,
    B10100000,
    B10010000,
    B10001000,
    6,
 
    B01111000,  //S
    B10000000,
    B10000000,
    B01110000,
    B00001000,
    B00001000,
    B11110000,
    6,
 
    B11111000,  //T
    B00100000,
    B00100000,
    B00100000,
    B00100000,
    B00100000,
    B00100000,
    6,
 
    B10001000,  //U
    B10001000,
    B10001000,
    B10001000,
    B10001000,
    B10001000,

    B01110000,
    6,
 
    B10001000,  //V
    B10001000,
    B10001000,
    B10001000,
    B10001000,
    B01010000,
    B00100000,
    6,
 
    B10001000,  //W
    B10001000,
    B10001000,
    B10101000,
    B10101000,
    B10101000,
    B01010000,
    6,
 
    B10001000,  //X
    B10001000,
    B01010000,
    B00100000,
    B01010000,
    B10001000,
    B10001000,
    6,
 
    B10001000,  //Y
    B10001000,
    B10001000,
    B01010000,
    B00100000,
    B00100000,
    B00100000,
    6,
 
    B11111000,  //Z
    B00001000,
    B00010000,
    B00100000,
    B01000000,
    B10000000,
    B11111000,
    6,
 
    B11100000,  //[
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B11100000,
    4,
 
    B00000000,  //(Backward Slash)
    B10000000,
    B01000000,
    B00100000,
    B00010000,
    B00001000,
    B00000000,
    6,
 
    B11100000,  //]
    B00100000,
    B00100000,
    B00100000,
    B00100000,
    B00100000,
    B11100000,
    4,
 
    B00100000,  //^
    B01010000,
    B10001000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    6,
 
    B00000000,  //_
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B11111000,
    6,
 
    B10000000,  //`
    B01000000,
    B00100000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    4,
 
    B00000000,  //a
    B00000000,
    B01110000,
    B00001000,
    B01111000,
    B10001000,
    B01111000,
    6,
 
    B10000000,  //b
    B10000000,
    B10110000,
    B11001000,
    B10001000,
    B10001000,
    B11110000,
    6,
 
    B00000000,  //c
    B00000000,
    B01110000,
    B10001000,
    B10000000,
    B10001000,
    B01110000,
    6,
 
    B00001000,  //d
    B00001000,
    B01101000,
    B10011000,
    B10001000,
    B10001000,
    B01111000,
    6,
 
    B00000000,  //e
    B00000000,
    B01110000,
    B10001000,
    B11111000,
    B10000000,
    B01110000,
    6,
 
    B00110000,  //f
    B01001000,
    B01000000,
    B11100000,
    B01000000,
    B01000000,
    B01000000,
    6,
 
    B00000000,  //g
    B01111000,
    B10001000,
    B10001000,
    B01111000,
    B00001000,
    B01110000,
    6,
 
    B10000000,  //h
    B10000000,
    B10110000,
    B11001000,
    B10001000,
    B10001000,
    B10001000,
    6,
 
    B01000000,  //i
    B00000000,
    B11000000,
    B01000000,
    B01000000,
    B01000000,
    B11100000,
    4,
 
    B00010000,  //j
    B00000000,
    B00110000,
    B00010000,
    B00010000,
    B10010000,
    B01100000,
    5,
 
    B10000000,  //k
    B10000000,
    B10010000,
    B10100000,
    B11000000,
    B10100000,
    B10010000,
    5,
 
    B11000000,  //l
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B11100000,
    4,
 
    B00000000,  //m
    B00000000,
    B11010000,
    B10101000,
    B10101000,
    B10001000,
    B10001000,
    6,
 
    B00000000,  //n
    B00000000,
    B10110000,
    B11001000,
    B10001000,
    B10001000,
    B10001000,
    6,
 
    B00000000,  //o
    B00000000,
    B01110000,
    B10001000,
    B10001000,
    B10001000,
    B01110000,
    6,
 
    B00000000,  //p
    B00000000,
    B11110000,
    B10001000,
    B11110000,
    B10000000,
    B10000000,
    6,
 
    B00000000,  //q
    B00000000,
    B01101000,
    B10011000,
    B01111000,
    B00001000,
    B00001000,
    6,
 
    B00000000,  //r
    B00000000,
    B10110000,
    B11001000,
    B10000000,
    B10000000,
    B10000000,
    6,
 
    B00000000,  //s
    B00000000,
    B01110000,
    B10000000,
    B01110000,
    B00001000,
    B11110000,
    6,
 
    B01000000,  //t
    B01000000,
    B11100000,
    B01000000,
    B01000000,
    B01001000,
    B00110000,
    6,
 
    B00000000,  //u
    B00000000,
    B10001000,
    B10001000,
    B10001000,
    B10011000,
    B01101000,
    6,
 
    B00000000,  //v
    B00000000,
    B10001000,
    B10001000,
    B10001000,
    B01010000,
    B00100000,
    6,
 
    B00000000,  //w
    B00000000,
    B10001000,
    B10101000,
    B10101000,
    B10101000,
    B01010000,
    6,
 
    B00000000,  //x
    B00000000,
    B10001000,
    B01010000,
    B00100000,
    B01010000,
    B10001000,
    6,
 
    B00000000,  //y
    B00000000,
    B10001000,
    B10001000,
    B01111000,
    B00001000,
    B01110000,
    6,
 
    B00000000,  //z
    B00000000,
    B11111000,
    B00010000,
    B00100000,
    B01000000,
    B11111000,
    6,
 
    B00100000,  //{
    B01000000,
    B01000000,
    B10000000,
    B01000000,
    B01000000,
    B00100000,
    4,
 
    B10000000,  //|
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    2,
 
    B10000000,  //}
    B01000000,
    B01000000,
    B00100000,
    B01000000,
    B01000000,
    B10000000,
    4,
 
    B00000000,  //~
    B00000000,
    B00000000,
    B01101000,
    B10010000,
    B00000000,
    B00000000,
    6,
 
    B01100000,  // (Char 0x7F)
    B10010000,
    B10010000,
    B01100000,
    B00000000,
    B00000000,
    B00000000,
    5,
    
    B00000000,  // smiley
    B01100000,
    B01100110,
    B00000000,
    B10000001,
    B01100110,
    B00011000,
    5
};
 
void scrollFont() {
    for (int counter=0x20;counter<0x80;counter++){
        loadBufferLong(counter);
        delay(500);
    }
}
 
// Scroll Message
void scrollMessage(const unsigned char * messageString) {
    int counter = 0;
    int myChar=0;
    do {
        // read back a char 
        myChar =  pgm_read_byte_near(messageString + counter); 
        if (myChar != 0){
            loadBufferLong(myChar);
        }
        counter++;
    } 
    while (myChar != 0);
}
// Load character into scroll buffer
void loadBufferLong(int ascii){
    if (ascii >= 0x20 && ascii <=0x7f){
        for (int a=0;a<7;a++){                      // Loop 7 times for a 5x7 font
            unsigned long c = pgm_read_byte_near(font5x7 + ((ascii - 0x20) * 8) + a);     // Index into character table to get row data
            unsigned long x = bufferLong [a*2];     // Load current scroll buffer
            x = x | c;                              // OR the new character onto end of current
            bufferLong [a*2] = x;                   // Store in buffer
        }
        byte count = pgm_read_byte_near(font5x7 +((ascii - 0x20) * 8) + 7);     // Index into character table for kerning data
        for (byte x=0; x<count;x++){
            rotateBufferLong();
            printBufferLong();
            delay(SCROLL_SPEED);
        }
    }
}
// Rotate the buffer
void rotateBufferLong(){
    for (int a=0;a<7;a++){                      // Loop 7 times for a 5x7 font
        unsigned long x = bufferLong [a*2];     // Get low buffer entry
        byte b = bitRead(x,31);                 // Copy high order bit that gets lost in rotation
        x = x<<1;                               // Rotate left one bit
        bufferLong [a*2] = x;                   // Store new low buffer
        x = bufferLong [a*2+1];                 // Get high buffer entry
        x = x<<1;                               // Rotate left one bit
        bitWrite(x,0,b);                        // Store saved bit
        bufferLong [a*2+1] = x;                 // Store new high buffer
    }
}  
// Display Buffer on LED matrix
void printBufferLong(){
  for (int a=0;a<7;a++){                    // Loop 7 times for a 5x7 font
    unsigned long x = bufferLong [a*2+1];   // Get high buffer entry
    byte y = x;                             // Mask off first character
    lc.setRow(3,a,y);                       // Send row to relevent MAX7219 chip
    x = bufferLong [a*2];                   // Get low buffer entry
    y = (x>>24);                            // Mask off second character
    lc.setRow(2,a,y);                       // Send row to relevent MAX7219 chip
    y = (x>>16);                            // Mask off third character
    lc.setRow(1,a,y);                       // Send row to relevent MAX7219 chip
    y = (x>>8);                             // Mask off forth character
    lc.setRow(0,a,y);                       // Send row to relevent MAX7219 chip
  }
}
