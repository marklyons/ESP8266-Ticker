# ESP8266-Ticker

This project uses a simple ESP8266 WiFi microchip to poll financial data in real time -- displaying prices on a MAX7219 LED matrix reminiscent of a digital stock ticker. Additionally, there is a WS2812B LED strip inside the project enclosure that illuminates a translucent acrylic cap. 

![](demo.gif)

This always reports Bitcoin price in real time and also tracks major financial indices, representing futures data if the corresponding market is not open. 

This is a personal project of mine that I built mostly for fun, but I want to open source it in case anyone might find it useful. I will be maintaining the project here as I add more indices and customization features, as well as LED animations to represent price movements in especially volatile markets such as those backed by cryptocurrencies.
