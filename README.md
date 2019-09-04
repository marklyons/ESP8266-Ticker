# ESP8266-Ticker

This project uses a simple ESP8266 WiFi microchip to poll financial data in real time -- displaying prices on a MAX7219 LED matrix reminiscent of a digital stock ticker. Additionally, there is a WS2812B LED strip inside the project enclosure that illuminates a translucent acrylic cap. 

![](demo.gif)

This always reports Bitcoin price in real time and also tracks major financial indices, representing futures data if the corresponding market is not open. 

This is a personal project of mine that I built mostly for fun, but I want to open source it in case anyone might find it useful. I will be maintaining the project here as I add more indices and customization features, as well as LED animations to represent price movements in especially volatile markets such as those backed by cryptocurrencies.

## Schematic

Here is a crude drawing of how this is all wired up.

![](schematic.jpg)

One outstanding issue with how this project is currently wired up is that the ESP8266 is being powered by a 5V power supply when it actually requires just 3.3V. This is definitely not good practice, but for now the chip has not been fried! You should probably use a voltage regulator to accomplish this reduction for the ESP8266, while still taking advantage of the full 5V power in for the LED strip.

Here's how I have everything set up inside the enclosure... Doesn't tell you much, but the schematic above should explain all connections.

![](circuit_board.jpg)

##Restful API

I built a simple API in Node.js that returns a JSON array which the ESP8266 can parse source-by-source. The device handle corresponding LED color changes as it scrolls through the sources and all of this data comes from the API. The API uses a Node package called [btc-stats](https://www.npmjs.com/package/btc-stats) to automatically get average BTC trading prices across different exchanges.

##Build

The main portion of the project enclosure is constructed out of pine wood. The LED display cover is made out of white translucent acrylic sheets. Everything was sanded down with a rotary sander to ensure all edges/corners met up and to give the project a polished look. 

The LED strip panel sits on a level above the ESP8266, the MAX7219 panels, and all of the wiring. This is to eliminate any obstructions above the LEDs that would result in large shadows on the acrylic cover. I wanted to cover as much of this surface with LED strips as I could to make it as bright as possible, and I really learned a thing or two about soldering together LED strips. You can check out the video linked below to see how I did this. The video also shows the LED panel for this project at the end.

[![How to Cut, Solder, and Connect LED Strips](https://img.youtube.com/vi/PkWv06by0pU/0.jpg)](https://www.youtube.com/watch?v=PkWv06by0pU)

