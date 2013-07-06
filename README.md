LEDTetrisTie
============

![Alt text](/photos/title.JPG "Optional title")


A fun necktie with a classic video game twist! For details, go here: http://www.billporter.info/2013/06/21/led-tetris-tie/

Now runs off an Arduino Uno / ATMega328 equivalent. See ATTinyNoAI branch for a Digispark compatible version. 

AI player code by Mofidul Jamal. It's 10 times cooler now! 

If you have the RAM for it, it should be able to scale up to a 256x256 display. I'g going to try and add back scoring, timeouts, and next block previews as compile time options. 

Modify draw() function with your low-level LED driver code of choice. Uses Adafruit's Neopixel library to drive ws2811 LED pixels. https://github.com/adafruit/Adafruit_NeoPixel

This work is licensed under the Creative Commons Attribution-Non-Commercial-ShareAlike 3.0 Unported License.
To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/ or
send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
