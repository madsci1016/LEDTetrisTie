
/*
* LEDTetrisNeckTie.c
*
* Created: 6/21/2013 
*  Author: Bill Porter
*    www.billporter.info
*
*   AI player code by: Mofidul Jamal
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



#include <Adafruit_NeoPixel.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

//Constants on how it's physically wired
#define LEDDATAPIN 2
#define POWEROFFPIN 4
#define BATTERYVOLTAGEPIN 0

//constants and initialization
#define UP  0
#define DOWN  1
#define RIGHT  2
#define LEFT  3
#define brick_count 7

#define FULL 255
#define WHITE 0xFF
#define OFF 0

//Display Settings
#define    FIELD_WIDTH 4
#define    FIELD_HEIGHT 20
//const short    field_start_x    = 1;
//const short    field_start_y    = 1;
//const short           preview_start_x    = 13;
//const short    preview_start_y    = 1;
//const short    delimeter_x      = 11;
//gameplay Settings
//const bool    display_preview    = 1;
#define tick_delay 100 //game speed
#define max_level 9

//weight given to the highest column for ai
#define HIGH_COLUMN_WEIGHT 5
//weight given to the number of holes for ai
#define HOLE_WEIGHT 3

static PROGMEM prog_uint16_t bricks[ brick_count ][4] = {
  {
    0b0100010001000100,      //1x4 cyan
    0b0000000011110000,
    0b0100010001000100,
    0b0000000011110000
  },
  {
    0b0000010011100000,      //T  purple
    0b0000010001100100,
    0b0000000011100100,
    0b0000010011000100
  },
  {
    0b0000011001100000,      //2x2 yellow
    0b0000011001100000,
    0b0000011001100000,
    0b0000011001100000
  },
  {
    0b0000000011100010,      //L orange
    0b0000010001001100,
    0b0000100011100000,
    0b0000011001000100
  },
  {
    0b0000000011101000,      //inverse L blue
    0b0000110001000100,
    0b0000001011100000,
    0b0000010001000110
  },
  {
    0b0000100011000100,      //S green
    0b0000011011000000,
    0b0000100011000100,
    0b0000011011000000
  },
  {
    0b0000010011001000,      //Z red
    0b0000110001100000,
    0b0000010011001000,
    0b0000110001100000
  }
};

//8 bit RGB colors of blocks
//RRRGGGBB
static PROGMEM prog_uint8_t brick_colors[brick_count]={
  0b00011111, //cyan
  0b10000010, //purple
  0b11111100, //yellow
  0b11101000, //orange?
  0b00000011, //blue
  0b00011100, //green
  0b11100000 //red
  
};


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


byte wall[FIELD_WIDTH][FIELD_HEIGHT];
//The 'wall' is the 2D array that holds all bricks that have already 'fallen' into place

bool aiCalculatedAlready = false;

struct TAiMoveInfo{
  byte rotation;
  int positionX, positionY;
  int weight;
} aiCurrentMove;

struct TBrick{
  byte type; //This is the current brick shape. 
  byte rotation; //active brick rotation
  byte color; //active brick color
  int positionX, positionY; //active brick position
  byte pattern[4][4]; //2D array of active brick shape, used for drawing and collosion detection

} currentBrick;


//unsigned short  level        = 0;
//unsigned long  score        = 0;
//unsigned long  score_lines      = 0;

// Define the RGB pixel array and controller functions, 
Adafruit_NeoPixel strip = Adafruit_NeoPixel(80, LEDDATAPIN, NEO_GRB + NEO_KHZ800);



void setup(){

  pinMode(LEDDATAPIN, OUTPUT); //LED on Model B
  pinMode(POWEROFFPIN, OUTPUT); 
  pinMode(13, OUTPUT); 
  newGame();


}

void loop(){

  checkBattery();

  //screenTest();
  play();
  
}

//checks battery voltage, toggles external power latch if battery is low
void checkBattery(){

  int voltage=0;
  
  //read ADC pin, convert to voltage measurement
  voltage = map(analogRead(BATTERYVOLTAGEPIN),0,1024,0,5000);
  
  //low battery voltage, turn off power to tie
  if(voltage<2450)
    digitalWrite(4, HIGH);
	
}

//tests pixels
void screenTest(){
  for( int i = 0; i < FIELD_WIDTH; i++ )
  {
    for( int k = 0; k < FIELD_HEIGHT; k++ )
    {
      wall[i][k] = 7;
      drawGame();
      delay(500);
    }
  }
}

//plays the game!
void play(){

  //see how high the wall goes, end game early to save battery power
  if(getHighestColumn() > 9)
    newGame();
  
  
if(currentBrick.positionY < 2){
    moveDown();
  //pulse onbaord LED and delay game
  digitalWrite(13, HIGH);   
  delay(tick_delay);               
  digitalWrite(13, LOW);    
  delay(tick_delay);  
}
else{
  //this flag gets set after calculating the AI move
  //and reset after we get the nextbrick in the nextBrick call
  if(aiCalculatedAlready == false)
  {
    performAI();
  }
  else
  {
    byte command = getCommand();
      if( command == UP )
  {
    if( checkRotate( 1 ) == true )
    {
      rotate( 1 );
      moveDown();
    }
    else
    moveDown();
  }
  else if( command == LEFT )
  {
    if( checkShift( -1, 0 ) == true )
    {
      shift( -1, 0 );
      moveDown();
    }
    else
    moveDown();
  }
  else if( command == RIGHT )
  {
    if( checkShift( 1, 0 ) == true )
    {
      shift( 1, 0 );
      moveDown();
    }
    else
    moveDown();
  }

  if(command == DOWN )
  {
    moveDown();
    
  }
    //pulse onbaord LED and delay game
  digitalWrite(13, HIGH);   
  delay(tick_delay);               
  digitalWrite(13, LOW);    
  delay(tick_delay);  
  
  }
  }
  drawGame();

    
}

//performs AI player calculations. 
void performAI(){
  struct TBrick initialBrick;
  //save position of the brick in its raw state
  memcpy((void*)&initialBrick, (void*)&currentBrick, sizeof(TBrick));
  //stores our 20 possible AI moves
  struct TAiMoveInfo aiMoves[4 * FIELD_WIDTH];
  //counter keeps track of the current index into our aimoves array
  byte aiMoveCounter = 0;
  //save position of the the brick at the left most rotated position
  struct TBrick aiLeftRotatedBrick;
  //save position of the brick at the rotated position
  struct TBrick aiRotatedBrick;

  //first check the rotations(initial, rotated once, twice, thrice)
  for(int aiRotation = 0; aiRotation < 4; aiRotation++ )
  {
    //rotate if possible
    if(checkRotate(1) == true)
      rotate(1);
    //save the rotated brick
    memcpy((void*)&aiRotatedBrick, (void*)&currentBrick, sizeof(TBrick));
    //shift as far left as possible
    while(checkShift(-1,0) == true)
      shift(-1, 0);
    //save this leftmost rotated position
    memcpy((void*)&aiLeftRotatedBrick, (void*)&currentBrick, sizeof(TBrick));

    //now check each possible position of X
    for(int aiPositionX = 0; aiPositionX < FIELD_WIDTH; aiPositionX++)
    {
      //next move down until we can't
      while(checkGround() == false )
      {
        shift(0,1);
      }
      //calculate ai weight of this particular final position
      int aiMoveWeight = aiCalculateWeight();
      //save the weight, positions and rotations for this ai move
      aiMoves[aiMoveCounter].weight = aiMoveWeight;
      aiMoves[aiMoveCounter].rotation = currentBrick.rotation;
      aiMoves[aiMoveCounter].positionX = currentBrick.positionX;
      aiMoves[aiMoveCounter].positionY = currentBrick.positionY;
      //move our index up for the next position to save to
      aiMoveCounter++;
      //drawGame();
      //Serial.println(aiMoveWeight);
      //delay(500);

      //now restore the previous position and shift it right by the column # we are checking
      memcpy((void*)&currentBrick, (void*)&aiLeftRotatedBrick, sizeof(TBrick));
      if(checkShift(aiPositionX+1,0) == true)
        shift(aiPositionX+1,0);
    }

    //reload rotated start position
    memcpy((void*)&currentBrick, (void*)&aiRotatedBrick, sizeof(TBrick));
  }
  
  //at this point we have calculated all the weights of every possible position and rotation of the brick

  //find move with lowest weight 
  int lowestWeight = aiMoves[0].weight;
  int lowestWeightIndex = 0;
  for(int i = 1; i < aiMoveCounter; i++)
  {
    if(aiMoves[i].weight <= lowestWeight)
    {
      lowestWeight = aiMoves[i].weight;
      lowestWeightIndex = i;
    }
  }
  //save this AI move as the current move
  memcpy((void*)&aiCurrentMove, (void*)&aiMoves[lowestWeightIndex], sizeof(TAiMoveInfo));
  //restore original brick that we started with
  memcpy((void*)&currentBrick, (void*)&initialBrick, sizeof(TBrick));
  //update the brick, set the ai flag so we know that we dont need to recalculate
  updateBrickArray();
  aiCalculatedAlready = true;
}

//calculates the ai weight
//when this function is called, the currentBrick is moved into a final legal position at the bottom of the wall
//which is why we add it to the wall first and then remove it at the end
int aiCalculateWeight(){
  int weights = 0;
  //add to wall first before calculating ai stuffs
  addToWall(); 
  //get the two weights
  int highestColumn = getHighestColumn();
  int holeCount = getHoleCount();

  //if this position will yield a full completed row then its weight is 0, which is the lowest possible
  //remember the the lowest weight will be the best move to make
  if(getFullLinePossible() == true)
  {
    weights = 0;
  }
  else
  {
      weights = (HIGH_COLUMN_WEIGHT * highestColumn) + (HOLE_WEIGHT * holeCount);
  }
  removeFromWall(); //undo the wall addition when done
  return weights;
}


//returns how high the wall goes 
int getHighestColumn(){
  int columnHeight = 0;
  //count
  int maxColumnHeight = 0;
  for(int j = 0; j < FIELD_WIDTH; j++)
  {
    columnHeight = 0;
    for(int k = FIELD_HEIGHT-1; k!=0; k--)
    {
      if(wall[j][k] != 0)
      {
        columnHeight = FIELD_HEIGHT - k;
        //Serial.print(k);
        //Serial.println(" is k");
        //delay(100);
      }
    }
    if(columnHeight > maxColumnHeight)
      maxColumnHeight = columnHeight;
  }
  return maxColumnHeight;
}

//counts the number of given holes for the ai calculation
int getHoleCount(){
  int holeCount = 0;
  for(int j = 0; j < FIELD_WIDTH; j++)
  {
    for(int k = currentBrick.positionY + 2; k < FIELD_HEIGHT; k++)
    {
      if(wall[j][k] == 0)
        holeCount++;
    }
  }
  return holeCount;
}

//determines if a full line is possible given the current wall (for ai)
bool getFullLinePossible()
{
  int lineCheck;
  for(byte i = 0; i < FIELD_HEIGHT; i++)
  {
    lineCheck = 0;
    for(byte k = 0; k < FIELD_WIDTH; k++)
    {
      if( wall[k][i] != 0)  
        lineCheck++;
    }
    
    if(lineCheck == FIELD_WIDTH)
    {
      return true;
    }
  }
  return false;
}
//gets commands according to ai state
byte getCommand(){
  if(currentBrick.rotation != aiCurrentMove.rotation)
    return UP;
  if(currentBrick.positionX > aiCurrentMove.positionX)
    return LEFT;
  if(currentBrick.positionX < aiCurrentMove.positionX)
    return RIGHT;
  if(currentBrick.positionX == aiCurrentMove.positionX)
    return DOWN;
}

//checks if the next rotation is possible or not.
bool checkRotate( bool direction )
{
  rotate( direction );
  bool result = !checkCollision();
  rotate( !direction );

  return result;
}

//checks if the current block can be moved by comparing it with the wall
bool checkShift(short right, short down)
{
  shift( right, down );
  bool result = !checkCollision();
  shift( -right, -down );

  return result;
}

// checks if the block would crash if it were to move down another step
// i.e. returns true if the eagle has landed.
bool checkGround()
{
  shift( 0, 1 );
  bool result = checkCollision();
  shift( 0, -1 );
  return result;
}

// checks if the block's highest point has hit the ceiling (true)
// this is only useful if we have determined that the block has been
// dropped onto the wall before!
bool checkCeiling()
{
  for( int i = 0; i < 4; i++ )
  {
    for( int k = 0; k < 4; k++ )
    {
      if(currentBrick.pattern[i][k] != 0)
      {
        if( ( currentBrick.positionY + k ) < 0 )
        {
          return true;
        }
      }
    }
  }
  return false;
}

//checks if the proposed movement puts the current block into the wall.
bool checkCollision()
{
  int x = 0;
  int y =0;

  for( byte i = 0; i < 4; i++ )
  {
    for( byte k = 0; k < 4; k++ )
    {
      if( currentBrick.pattern[i][k] != 0 )
      {
        x = currentBrick.positionX + i;
        y = currentBrick.positionY + k;

        if(x >= 0 && y >= 0 && wall[x][y] != 0)
        {
          //this is another brick IN the wall!
          return true;
        }
        else if( x < 0 || x >= FIELD_WIDTH )
        {
          //out to the left or right
          return true;
        }
        else if( y >= FIELD_HEIGHT )
        {
          //below sea level
          return true;
        }
      }
    }
  }
  return false; //since we didn't return true yet, no collision was found
}

//updates the position variable according to the parameters
void shift(short right, short down)
{
  currentBrick.positionX += right;
  currentBrick.positionY += down;
}

// updates the rotation variable, wraps around and calls updateBrickArray().
// direction: 1 for clockwise (default), 0 to revert.
void rotate( bool direction )
{
  if( direction == 1 )
  {
    if(currentBrick.rotation == 0)
    {
      currentBrick.rotation = 3;
    }
    else
    {
      currentBrick.rotation--;
    }
  }
  else
  {
    if(currentBrick.rotation == 3)
    {
      currentBrick.rotation = 0;
    }
    else
    {
      currentBrick.rotation++;
    }
  }
  updateBrickArray();
}

void moveDown(){
  if( checkGround() )
  {
    addToWall();
    drawGame();
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
  //ticks = 0;
}

//put the brick in the wall after the eagle has landed.
void addToWall()
{
  for( byte i = 0; i < 4; i++ )
  {
    for( byte k = 0; k < 4; k++ )
    {
      if(currentBrick.pattern[i][k] != 0){
        wall[currentBrick.positionX + i][currentBrick.positionY + k] = currentBrick.color;
        
      }
    }
  }
}

//removes brick from wall, used by ai algo
void removeFromWall(){
  for( byte i = 0; i < 4; i++ )
  {
    for( byte k = 0; k < 4; k++ )
    {
      if(currentBrick.pattern[i][k] != 0){
        wall[currentBrick.positionX + i][currentBrick.positionY + k] = 0;
        
      }
    }
  }
}

//uses the currentBrick_type and rotation variables to render a 4x4 pixel array of the current block
// from the 2-byte binary reprsentation of the block
void updateBrickArray()
{
  unsigned int data = pgm_read_word(&(bricks[ currentBrick.type ][ currentBrick.rotation ]));
  for( byte i = 0; i < 4; i++ )
  {
    for( byte k = 0; k < 4; k++ )
    {
      if(bitRead(data, 4*i+3-k))
      currentBrick.pattern[k][i] = currentBrick.color; 
      else
      currentBrick.pattern[k][i] = 0;
    }
  }
}
//clears the wall for a new game
void clearWall()
{
  for( byte i = 0; i < FIELD_WIDTH; i++ )
  {
    for( byte k = 0; k < FIELD_HEIGHT; k++ )
    {
      wall[i][k] = 0;
    }
  }
}

// find the lowest completed line, do the removal animation, add to score.
// returns true if a line was removed and false if there are none.
bool clearLine()
{
  int line_check;
  for( byte i = 0; i < FIELD_HEIGHT; i++ )
  {
    line_check = 0;

    for( byte k = 0; k < FIELD_WIDTH; k++ )
    {
      if( wall[k][i] != 0)  
      line_check++;
    }

    if( line_check == FIELD_WIDTH )
    {
      flashLine( i );
      for( int  k = i; k >= 0; k-- )
      {
        for( byte m = 0; m < FIELD_WIDTH; m++ )
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

//randomly selects a new brick and resets rotation / position.
void nextBrick(){
  currentBrick.rotation = 0;
  currentBrick.positionX = round(FIELD_WIDTH / 2) - 2;
  currentBrick.positionY = -3;

  currentBrick.type = random( 0, brick_count );

  currentBrick.color = pgm_read_byte(&(brick_colors[ currentBrick.type ]));

  aiCalculatedAlready = false;

  updateBrickArray();

  //displayPreview();
}

//effect, flashes the line at the given y position (line) a few times.  
void flashLine( int line ){

  bool state = 1;
  for(byte i = 0; i < 6; i++ )
  {
    for(byte k = 0; k < FIELD_WIDTH; k++ )
    {  
      if(state)
      wall[k][line] = 0b11111111;
      else
      wall[k][line] = 0;
      
    }
    state = !state;
    drawWall();
    updateDisplay();
    delay(200);
  }

}


//draws wall only, does not update display
void drawWall(){
  for(int j=0; j < FIELD_WIDTH; j++){
    for(int k = 0; k < FIELD_HEIGHT; k++ )
    {
      draw(wall[j][k],FULL,j,k);
    }
    
  }

}

//'Draws' wall and game piece to screen array 
void drawGame()
{

  //draw the wall first
  drawWall();

  //now draw current piece in play
  for( int j = 0; j < 4; j++ )
  {
    for( int k = 0; k < 4; k++ )
    {
      if(currentBrick.pattern[j][k] != 0)
      {
        if( currentBrick.positionY + k >= 0 )
        {
          draw(currentBrick.color, FULL, currentBrick.positionX + j, currentBrick.positionY + k);
          //field[ positionX + j ][ p osition_y + k ] = currentBrick_color;
        }
      }
    }
  }
  updateDisplay();
}

//takes a byte color values an draws it to pixel array at screen x,y values.
// Assumes a Down->UP->RIGHT->Up->Down->etc (Shorest wire path) LED strips display.
//new brightness value lets you dim LEDs w/o changing color. 
void draw(byte color, signed int brightness, byte x, byte y){
  
  unsigned short address=0;
  byte r,g,b;
  
  //flip y for new tie layout. remove if your strips go up to down
  y = (FIELD_HEIGHT-1) - y;
  
  //calculate address
  if(x%2==0) //even row
  address=FIELD_HEIGHT*x+y;
  else //odd row
  address=((FIELD_HEIGHT*(x+1))-1)-y;
  
  if(color==0 || brightness < 0){
    strip.setPixelColor(address, 0);
  }
  else{
    //calculate colors, map to LED system
    b=color&0b00000011;
    g=(color&0b00011100)>>2;
    r=(color&0b11100000)>>5;
    
    //make sure brightness value is correct
    brightness=constrain(brightness,0,FULL);
    
    strip.setPixelColor(address, map(r,0,7,0,brightness), map(g,0,7,0,brightness), map(b,0,3,0,brightness));

  }
  
}

//obvious function
void gameOver()
{
  /*  
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

//clean up, reset timers, scores, etc. and start a new round.
void newGame()
{

  //  level = 0;
  // ticks = 0;
  //score = 0;
  //score_lines = 0;
  //last_key = 0;
  clearWall();

  nextBrick();
}

//Update LED strips
void updateDisplay(){

  strip.show();
  
  
}
