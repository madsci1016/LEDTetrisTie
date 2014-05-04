/*
* LEDTetrisNeckTie.c
*
* Created: 6/21/2013 
*  Author: Bill Porter
*    www.billporter.info
*
*
*    Code to run a wicked cool LED Tetris playing neck tie. Details:              
*         http://www.billporter.info/2013/06/21/led-tetris-tie/
*
*This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
*To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/ or
*send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
*
*/ 



/* Notes
Analog 2 is pin 2
LED is on pin 0
RGB LEDS data is on pin 1
*/


#include <WS2811.h>

#include <avr/io.h>
#include <avr/pgmspace.h>



//Display Settings
#define    field_width 4
#define    field_height 20
//const short    field_start_x    = 1;
//const short    field_start_y    = 1;
//const short           preview_start_x    = 13;
//const short    preview_start_y    = 1;
//const short    delimeter_x      = 11;
//gameplay Settings
//const bool    display_preview    = 1;
#define tick_delay 50 //game speed


#define FULL 255

#define WHITE 0xFF
#define OFF 0




static uint8_t colors[]={
  0b00011111, //cyan
  0b10000010, //purple
  0b11111100, //yellow
  0b11101000, //orange?
  0b00000011, //blue
  0b00011100, //green
  0b11100000 //red
  
};


byte wall[field_width][field_height];
//The 'wall' is the 2D array that holds all bricks that have already 'fallen' into place



// Define the RGB pixel array and controller functions, using pin 0 on port b.
DEFINE_WS2811_FN(WS2811RGB, PORTB, 1)
RGB_t rgb[80]; //holds RGB brightness info

void setup(){

  pinMode(0, OUTPUT); //LED on Model B
  pinMode(1, OUTPUT); 

  for(int i=0; i<10; i++){
    rgb[i].r=0;
    rgb[i].g=0;
    rgb[i].b=255;
  }
  updateDisplay();
  randomSeed(analogRead(2));
  delay(200); 
  for(int i=0; i<80; i++){
    rgb[i].r=0;
    rgb[i].g=0;
    rgb[i].b=0;
  }
  updateDisplay();
  delay(200); 




}

void loop(){


  //screenTest();
  //play();
	fire1();
  delay(random(200, 1500));
}

void screenTest(){
  for( int i = 0; i < field_width; i++ )
  {
    for( int k = 0; k < field_height; k++ )
    {
      wall[i][k] = 7;
      //drawGame();
      delay(500);
    }
  }
}

void play(){

  

  //pulse onbaord LED and delay game
  digitalWrite(0, HIGH);   
  delay(tick_delay);               
  digitalWrite(0, LOW);    
  delay(tick_delay);      
}


//start anmation by 'shooting' a streamer out
void fire1(){
	
	byte start = random(0, field_width);
	byte target_height = random(2, field_height-6);
	byte color = random(0,6);
	byte size = random(3,5);
	
	//draw streamer, fadding tail
	for(int i=field_height; i>target_height; i--){
		draw(WHITE, FULL, start, i);
		draw(WHITE, FULL/2, start, i+1);
		draw(WHITE, FULL/4, start, i+2);
                draw(WHITE, FULL/8, start, i+3);
		draw(OFF, 0, start, i+4);
		
		updateDisplay();
		delay(tick_delay);
	}
	
        //clear display, ready explosion
	clear();
	
	//draw explosion out to final size
	for(int i=1; i<size; i++){
		
		//center pixel
		draw(colors[color], FULL, start, target_height);
		//draw out to current radius in 4 directions
		for(int y=1; y<i; y++){
			draw(colors[color], FULL/(3*y), start+y, target_height);
			draw(colors[color], FULL/(3*y), start-y, target_height);
			draw(colors[color], FULL/(3*y), start, target_height-y);
			draw(colors[color], FULL/(3*y), start, target_height+y);
                        draw(colors[color], FULL/(3*y), start+y, target_height+y);
			draw(colors[color], FULL/(3*y), start-y, target_height+y);
			draw(colors[color], FULL/(3*y), start+y, target_height-y);
			draw(colors[color], FULL/(3*y), start-y, target_height-y);
		}
		
		updateDisplay();
		delay(tick_delay*2);
	}

        //fade out color field
        
        for(int i=0; i<255; i++){
          draw(colors[color], FULL-i, start, target_height);
          //draw out to current radius in 4 directions
		for(int y=1; y<size; y++){
			draw(colors[color], FULL/(3*y)-i/(3*y), start+y, target_height);
			draw(colors[color], FULL/(3*y)-i/(3*y), start-y, target_height);
			draw(colors[color], FULL/(3*y)-i/(3*y), start, target_height-y);
			draw(colors[color], FULL/(3*y)-i/(3*y), start, target_height+y);
                        draw(colors[color], FULL/(3*y)-i/(3*y), start+y, target_height+y);
			draw(colors[color], FULL/(3*y)-i/(3*y), start-y, target_height+y);
			draw(colors[color], FULL/(3*y)-i/(3*y), start+y, target_height-y);
			draw(colors[color], FULL/(3*y)-i/(3*y), start-y, target_height-y);
		}
          updateDisplay();
          //delay(1);
        }
	
	clear();
	

        
}


















//clears the wall for a new game
void clear()
{
  for( byte i = 0; i < field_width; i++ )
  {
    for( byte k = 0; k < field_height; k++ )
    {
      draw(0,0,i,k);
    }
  }
  updateDisplay();
}








//draws wall only, does not update display
void drawWall(){
  for(int j=0; j < field_width; j++){
    for(int k = 0; k < field_height; k++ )
    {
      draw(wall[j][k],255,j,k);
    }
    
  }
	updateDisplay();
}



//takes a byte color values an draws it to pixel array at screen x,y values.
// Assumes a UP->Down->Down->Up (Shorest wire path) LED strips display.
void draw(byte color, signed int brightness, byte x, byte y){
  
  unsigned short address=0;
  byte r,g,b;
  
  if(x >= field_width || y >= field_height)
	  return;
  
  
  //calculate address
  if(x%2==0) //even row
  address=field_height*x+y;
  else //odd row
  address=((field_height*(x+1))-1)-y;
  
  if(color==0 || brightness < 0){
    rgb[address].r=0;
    rgb[address].g=0;
    rgb[address].b=0;
  }
  else{
    //calculate colors, map to LED system
    b=color&0b00000011;
    g=(color&0b00011100)>>2;
    r=(color&0b11100000)>>5;
    
    rgb[address].r=map(r,0,7,0,brightness); 
    rgb[address].g=map(g,0,7,0,brightness);
    rgb[address].b=map(b,0,3,0,brightness);
  }
  
}



//Update LED strips
void updateDisplay(){

  WS2811RGB(rgb, ARRAYLEN(rgb));
}
