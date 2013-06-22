/*
 * LEDTetrisNeckTie.c
 *
 * Created: 6/21/2013 
 *  Author: Bill Porter
 *		www.billporter.info
 *
 *
 *		Code to run a wicked cool LED Tetris playing neck tie. Details: 						 
 *				 http://www.billporter.info/2013/06/21/led-tetris-tie/
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

//constants and initialization
#define UP	0
#define DOWN	1
#define RIGHT	2
#define LEFT	3

#define brick_count 7
static PROGMEM prog_uint16_t bricks[ brick_count ][4] = {
  {
    0b0100010001000100,      //1x4
    0b0000000011110000,
    0b0100010001000100,
    0b0000000011110000
  },
  {
    0b0000010011100000,      //T
    0b0000010001100100,
    0b0000000011100100,
    0b0000010011000100
  },
  {
    0b0000011001100000,      //2x2
    0b0000011001100000,
    0b0000011001100000,
    0b0000011001100000
  },
  {
    0b0000000011100010,      //L
    0b0000010001001100,
    0b0000100011100000,
    0b0000011001000100
  },
  {
    0b0000000011101000,      //inverse L
    0b0000110001000100,
    0b0000001011100000,
    0b0000010001000110
  },
  {
    0b0000100011000100,      //S
    0b0000011011000000,
    0b0000100011000100,
    0b0000011011000000
  },
  {
    0b0000010011001000,      //Z
    0b0000110001100000,
    0b0000010011001000,
    0b0000110001100000
  }
};



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
#define tick_microseconds 0 //adjustable after speed-optimization



#define max_level 9

/*const unsigned short level_ticks_timeout[ max_level ]  = {
  32,
  28,
  24,
  20,
  17,
  13,
  10,
  8,
  5
};*/
//const unsigned int score_per_level          = 10; //per brick in lv 1+
//const unsigned int score_per_line          = 300;

//runtime variables
byte field[field_width][field_height];
byte wall[field_width][field_height];

unsigned int  ticks        = 0;
unsigned short  last_key      = 0;

unsigned short  current_brick_type;
//unsigned short  next_brick_type;
unsigned short  current_brick[4][4];
unsigned short  current_brick_color;
int position_x, position_y;
unsigned short  rotation;

unsigned short  level        = 0;
//unsigned long  score        = 0;
//unsigned long  score_lines      = 0;

// Define the output function, using pin 0 on port b.
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
  randomSeed(analogRead(3));
  delay(200); 
  for(int i=0; i<80; i++){
    rgb[i].r=0;
    rgb[i].g=0;
    rgb[i].b=0;
  }
  updateDisplay();
  delay(200); 

  newGame();


}

void loop(){

  getLastKey();

  if( last_key == UP )
  {
    if( checkRotate( 1 ) == true )
    {
      rotate( 1 );
      moveDown();
    }
    else
    moveDown();
  }
  else if( last_key == LEFT )
  {
    if( checkShift( -1, 0 ) == true )
    {
      shift( -1, 0 );
      moveDown();
    }
    else
    moveDown();
  }
  else if( last_key == RIGHT )
  {
    if( checkShift( 1, 0 ) == true )
    {
      shift( 1, 0 );
      moveDown();
    }
    else
    moveDown();
  }

  if(last_key == DOWN )
  {
    moveDown();
    
  }

  render();
  display();
  //delayMicroseconds( tick_microseconds );
  digitalWrite(0, HIGH);   // turn the LED on (HIGH is the voltage level)
  //digitalWrite(1, HIGH);
  delay(100);               // wait for a second
  digitalWrite(0, LOW);    // turn the LED off by making the voltage LOW
  //digitalWrite(1, LOW); 
  delay(100);             
}


/* for(int i=0; i<80; i++){
    rgb[i].r=255;
    rgb[i].g=0;
    rgb[i].b=0;
  }
updateDisplay();
delay(1000); 
  for(int i=0; i<80; i++){
    rgb[i].r=0;
    rgb[i].g=255;
    rgb[i].b=0;
  }
updateDisplay();
delay(1000); 
  for(int i=0; i<80; i++){
    rgb[i].r=0;
    rgb[i].g=0;
    rgb[i].b=255;
  }
updateDisplay();
delay(1000);*/

void moveDown(){
  if( checkGround() )
  {
    addToWall();
    if( checkCeiling() )
    {
      gameOver();
    }
    else
    {
      while( clearLine() )
      {
        //scoreOneUpLine();
      }
      nextBrick();
      //scoreOneUpBrick();
    }
  }
  else
  {
    //grounding not imminent
    shift( 0, 1 );
  }
  //scoreAdjustLevel();
  ticks = 0;
}

//get functions. set global variables.
void getLastKey(){
  /*
  * saves the most recently received key code to last_key.

  last_key = 0;
  while( Serial.available() > 0 )
  {
    last_key = Serial.read();
  }
  */

  last_key=random(0,100);
  last_key%=4;

}

//check functions. do nothing and return true or false.
bool checkRotate( bool direction )
{
  /*
  * checks if the next rotation is possible or not.
  */
  rotate( direction );
  bool result = !checkCollision();
  rotate( !direction );

  return result;
}
bool checkShift(short right, short down)
{
  /*
  * checks if the current block can be moved to the left by comparing it with the wall
  */
  shift( right, down );
  bool result = !checkCollision();
  shift( -right, -down );

  return result;
}
bool checkGround()
{
  /*
  * checks if the block would crash if it were to move down another step
  * i.e. returns true if the eagle has landed.
  */
  shift( 0, 1 );
  bool result = checkCollision();
  shift( 0, -1 );
  return result;
}
bool checkCeiling()
{
  /*
  * checks if the block's highest point has hit the ceiling (true)
  * this is only useful if we have determined that the block has been
  * dropped onto the wall before!
  */
  for( int i = 0; i < 4; i++ )
  {
    for( int k = 0; k < 4; k++ )
    {
      if(current_brick[i][k] != 0)
      {
        if( ( position_y + k ) < 0 )
        {
          return true;
        }
      }
    }
  }
  return false;
}
bool checkCollision()
{
  /*
  * checks if the proposed movement puts the current block into the wall.
  */
  int x = 0;
  int y =0;

  for( int i = 0; i < 4; i++ )
  {
    for( int k = 0; k < 4; k++ )
    {
      if( current_brick[i][k] != 0 )
      {
        x = position_x + i;
        y = position_y + k;

        if(x >= 0 && y >= 0 && wall[x][y] != 0)
        {
          //this is another brick IN the wall!
          return true;
        }
        else if( x < 0 || x >= field_width )
        {
          //out to the left or right
          return true;
        }
        else if( y >= field_height )
        {
          //below sea level
          return true;
        }
      }
    }
  }
  return false; //since we didn't return true yet, no collision was found
}

//do functions - actually execute the command being the function's name
void shift(short right, short down)
{
  /*
  * updates the position variable according to the parameters
  */
  position_x += right;
  position_y += down;
}

void rotate( bool direction )
{
  /*
  * updates the rotation variable and calls updateBrickArray().
  * direction: 1 for clockwise (default), 0 to revert.
  */
  if( direction == 1 )
  {
    if(rotation == 0)
    {
      rotation = 3;
    }
    else
    {
      rotation--;
    }
  }
  else
  {
    if(rotation == 3)
    {
      rotation = 0;
    }
    else
    {
      rotation++;
    }
  }
  updateBrickArray();
}

void addToWall()
{
  /*
  * put the brick in the wall after the eagle has landed.
  */
  for( int i = 0; i < 4; i++ )
  {
    for( int k = 0; k < 4; k++ )
    {
      if(current_brick[i][k] != 0)
      wall[position_x + i][position_y + k] = current_brick_color;
    }
  }
}

void updateBrickArray()
{
  /*
  * uses the current_brick_type and rotation variables to render a 4x4 pixel array of the current block.
  */
  unsigned int data = pgm_read_word(&(bricks[ current_brick_type ][ rotation ]));
  for( int i = 0; i < 4; i++ )
  {
    for( int k = 0; k < 4; k++ )
    {
      if(bitRead(data, 4*i+3-k))
      current_brick[k][i] = current_brick_color; //probability of this being thought through <= 0
      else
      current_brick[k][i] = 0;
    }
  }
}

bool clearLine()
{
  /*
  * find the lowest completed line, do the removal animation, add to score.
  * returns true if a line was removed and false if there are none.
  */
  int line_check;
  for( int i = 0; i < field_height; i++ )
  {
    line_check = 0;

    for( int k = 0; k < field_width; k++ )
    {
      if( wall[k][i] != 0)  
      line_check++;
    }

    if( line_check == field_width )
    {
      flashLine( i );
      for( int k = i; k >= 0; k-- )
      {
        for( int m = 0; m < field_width; m++ )
        {
          if( k > 0)
          {
            wall[m][k] = wall[m][k-1];
          }
          else
          {
            wall[m][k] = 0;
          }
        }
      }

      return true; //line removed.
    }
  }
  return false; //no complete line found
}

void nextBrick()
{


  /*
  * randomly selects a new brick and resets rotation / position.
  */
  rotation = 0;
  position_x = round(field_width / 2) - 2;
  position_y = -3;

  current_brick_type = random( 0, 6 );
  
  current_brick_color = random(1, 7);
  


  updateBrickArray();

  //displayPreview();
}

void clearWall()
{
  /*
  * clears the wall for a new game
  */
  for( int i = 0; i < field_width; i++ )
  {
    for( int k = 0; k < field_height; k++ )
    {
      wall[i][k] = 0;
    }
  }
}

void render()
{


  /*
  * joins wall and floating brick into one array which can be displayed
  */

  //copy the wall to the output array
  for( int i = 0; i < field_width; i++ )
  {
    for( int k = 0; k < field_height; k++ )
    {
      field[i][k] = wall[i][k];
    }
  }

  //superimpose the brick array on top of that at the correct position
  for( int i = 0; i < 4; i++ )
  {
    for( int k = 0; k < 4; k++ )
    {
      if(current_brick[i][k] != 0)
      {
        if( position_y + k >= 0 )
        {
          field[ position_x + i ][ position_y + k ] = current_brick_color;
        }
      }
    }
  }
}

void flashLine( int line ){
  /*
  * effect, flashes the line at the given y position (line) a few times.
  */
  bool state = 0;
  for( int i = 0; i < 6; i++ )
  {
    for( int k = 0; k < field_width; k++ )
    {  
      if(state)
      field[k][line] = 7;
      else
      field[k][line] = 0;
      //field[k][line] = state;
    }
    state = !state;
    display();
    delay(100);
  }
}

void display()
{

  unsigned short row=0;
  unsigned short address=0;
  /*
  * Display the current rendered game field
    //map to my funny display
  */

  //row 0
  for( int k = 0; k < field_height; k++ )
  {
    address=k;
    
    if(field[row][k]){
      if(bitRead(field[row][k],0))
      rgb[address].r=255; //change to my rows
      if(bitRead(field[row][k],1))
      rgb[address].g=255;
      if(bitRead(field[row][k],2))
      rgb[address].b=255;
    }
    else {
      rgb[address].r=0;
      rgb[address].g=0;
      rgb[address].b=0;
    }
  }

  //row 1
  row++;
  for( int k = 0; k < field_height; k++ )
  {
    address=39-k;
    
    if(field[row][k]){
      if(bitRead(field[row][k],0))
      rgb[address].r=255; //change to my rows
      if(bitRead(field[row][k],1))
      rgb[address].g=255;
      if(bitRead(field[row][k],2))
      rgb[address].b=255;
    }
    else {
      rgb[address].r=0;
      rgb[address].g=0;
      rgb[address].b=0;
    }
  }
  //row 2
  row++;
  for( int k = 0; k < field_height; k++ )
  {
    address=40+k;
    
    if(field[row][k]){
      if(bitRead(field[row][k],0))
      rgb[address].r=255; //change to my rows
      if(bitRead(field[row][k],1))
      rgb[address].g=255;
      if(bitRead(field[row][k],2))
      rgb[address].b=255;
    }
    else {
      rgb[address].r=0;
      rgb[address].g=0;
      rgb[address].b=0;
    }
  }
  //row 3
  row++;
  for( int k = 0; k < field_height; k++ )
  {
    address=79-k;
    
    if(field[row][k]){
      if(bitRead(field[row][k],0))
      rgb[address].r=255; //change to my rows
      if(bitRead(field[row][k],1))
      rgb[address].g=255;
      if(bitRead(field[row][k],2))
      rgb[address].b=255;
    }
    else {
      rgb[address].r=0;
      rgb[address].g=0;
      rgb[address].b=0;
    }
  }
  /*
    for( int k = 0; k < field_height; k++ )
    {
      
            if(field[row][k])
            rgb[39-k].r=255; //change to my rows
            else
            rgb[39-k].r=0;
        }
    //row 2
    for( int k = 0; k < field_height; k++ )
    {
      
            if(field[2][k])
            rgb[k+40].r=255; //change to my rows
            else
            rgb[k+40].r=0;
        }
    //row 3
    for( int k = 0; k < field_height; k++ )
    {
      
            if(field[3][k])
            rgb[79-k].r=255; //change to my rows
            else
            rgb[79-k].r=0;
        }*/
  updateDisplay();
  //Display.pushData();
}

void gameOver()
{
  /*
  * pretty self-explanatory. Also displays final score (maybe)
  
  Serial.println( "Game Over." );

  Serial.print( "Level:\t");
  Serial.println( level );

  Serial.print( "Lines:\t" );
  Serial.println( score_lines );

  Serial.print( "Score:\t");
  Serial.println( score );
  Serial.println();

  Serial.println("Insert coin to continue");
  waitForInput();
*/
  newGame();
}

void newGame()
{
  /*
  * clean up, reset timers, scores, etc. and start a new round.
  */
  level = 0;
  ticks = 0;
  //score = 0;
  //score_lines = 0;
  last_key = 0;
  clearWall();

  nextBrick();
}


void updateDisplay(){

  WS2811RGB(rgb, ARRAYLEN(rgb));
  /*
digitalWrite(0, HIGH);   // turn the LED on (HIGH is the voltage level)
//digitalWrite(1, HIGH);
delay(1000);               // wait for a second
digitalWrite(0, LOW);    // turn the LED off by making the voltage LOW
//digitalWrite(1, LOW); 
delay(1000);               // wait for a second
*/
}
