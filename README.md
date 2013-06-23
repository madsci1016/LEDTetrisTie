LEDTetrisTie
============

![Alt text](/photos/title.JPG "Optional title")


A fun necktie with a classic video game twist! For details, go here: http://www.billporter.info/2013/06/21/led-tetris-tie/

Runs off a DigiSpark (ATTiny85) MCU, drives ws2811 LED pixels. 

I'm working on making the code scalable and shrinking RAM use at the same time which is a challange. If you have the
RAM for it, it should be able to scale up to a 256x256 display. I'g going to try and add back scoring, timeouts, and 
next block previews as compile time options. 

Modify draw() function with your low-level LED driver code of choice. Uses assembly driver for WS2811 library right now, 
I'm going to change-over to Adafruit's Neopixel library shortly. 

This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/ or
send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
