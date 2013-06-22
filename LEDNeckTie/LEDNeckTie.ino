/* Notes
Analog 2 is pin 2
LED is on pin 0
RGB LEDS data is on pin 1
*/

#include <fix_fft.h>
#include <WS2811.h>

// Define the output function, using pin 0 on port b.
DEFINE_WS2811_FN(WS2811RGB, PORTB, 1)

char im[128];
char data[128]; 
RGB_t rgb[20]; //holds RGB brightness info

void setup(){
	
 pinMode(0, OUTPUT); //LED on Model B
 pinMode(1, OUTPUT); 
}

void loop(){
  int static i = 0;
  static long tt;
  int val;
   
   if (millis() > tt){
      if (i < 128){
        val = analogRead(2);
        data[i] = val / 4 - 128;
        im[i] = 0;
        i++;   
        
      }
      else{
        //this could be done with the fix_fftr function without the im array.
        fix_fft(data,im,7,0);
        // I am only interessted in the absolute value of the transformation
        for (i=0; i< 64;i++){
           data[i] = sqrt(data[i] * data[i] + im[i] * im[i]); 
        }
        
        //do something with the data values 1..64 and ignore im
        //average first 30 freq bands
        for(i=0; i<30; i++){
         val = data[i]; 
          }
          val/=30;
          
         val = map(val, 0,256,0,20);
        
        for(i=0; i<val; i++)
         rgb[i].r=255;
        
        for(;i<20; i++)
         rgb[i].r=0;
         
        updateDisplay(); 
        //show_big_bars(data,0);
      }
    
    tt = millis();
   }
}

void updateDisplay(){
  
  WS2811RGB(rgb, ARRAYLEN(rgb));
  
   digitalWrite(0, HIGH);   // turn the LED on (HIGH is the voltage level)
  //digitalWrite(1, HIGH);
  delay(1000);               // wait for a second
  digitalWrite(0, LOW);    // turn the LED off by making the voltage LOW
  //digitalWrite(1, LOW); 
  delay(1000);               // wait for a second
  
}
