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
#define tick_delay 100 //game speed
#define max_level 9



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

//6 bit RGB colors of blocks
static PROGMEM prog_uint8_t brick_colors[brick_count]={
	0b00001111, //cyan
	0b00100011, //purple
	0b00111100, //yellow
	0b00110100, //orange?
	0b00000011, //blue
	0b00001100, //green
	0b00110000 //red
	
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


byte wall[field_width][field_height];
//The 'wall' is the 2D array that holds all bricks that have already 'fallen' into place

struct brick_type{
	byte type; //This is the current brick shape. 
	byte rotation; //active brick rotation
	byte color; //active brick color
	int position_x, position_y; //active brick position
	byte pattern[4][4]; //2D array of active brick shape, used for drawing and collosion detection

} current_brick;


//unsigned short  level        = 0;
//unsigned long  score        = 0;
//unsigned long  score_lines      = 0;

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

	//screenTest();
	play();
	
}

void screenTest(){
	for( int i = 0; i < field_width; i++ )
	{
		for( int k = 0; k < field_height; k++ )
		{
			wall[i][k] = 7;
			drawGame();
			delay(500);
		}
	}
}

void play(){

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

	drawGame();

	//pulse onbaord LED and delay game
	digitalWrite(0, HIGH);   
	delay(tick_delay);               
	digitalWrite(0, LOW);    
	delay(tick_delay);      
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

//get functions. set global variables.
byte getCommand(){
	
	//AI code to go here shortly
	unsigned short  last_key = 0;

	last_key=random(0,100);
	last_key%=4;
	
	return (byte)last_key;

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
			if(current_brick.pattern[i][k] != 0)
			{
				if( ( current_brick.position_y + k ) < 0 )
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
			if( current_brick.pattern[i][k] != 0 )
			{
				x = current_brick.position_x + i;
				y = current_brick.position_y + k;

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

//updates the position variable according to the parameters
void shift(short right, short down)
{
	current_brick.position_x += right;
	current_brick.position_y += down;
}

// updates the rotation variable, wraps around and calls updateBrickArray().
// direction: 1 for clockwise (default), 0 to revert.
void rotate( bool direction )
{
	if( direction == 1 )
	{
		if(current_brick.rotation == 0)
		{
			current_brick.rotation = 3;
		}
		else
		{
			current_brick.rotation--;
		}
	}
	else
	{
		if(current_brick.rotation == 3)
		{
			current_brick.rotation = 0;
		}
		else
		{
			current_brick.rotation++;
		}
	}
	updateBrickArray();
}


//put the brick in the wall after the eagle has landed.
void addToWall()
{
	for( byte i = 0; i < 4; i++ )
	{
		for( byte k = 0; k < 4; k++ )
		{
			if(current_brick.pattern[i][k] != 0){
			wall[current_brick.position_x + i][current_brick.position_y + k] = current_brick.color;
                        
                        }
		}
	}
}

//uses the current_brick_type and rotation variables to render a 4x4 pixel array of the current block
// from the 2-byte binary reprsentation of the block
void updateBrickArray()
{
	unsigned int data = pgm_read_word(&(bricks[ current_brick.type ][ current_brick.rotation ]));
	for( byte i = 0; i < 4; i++ )
	{
		for( byte k = 0; k < 4; k++ )
		{
			if(bitRead(data, 4*i+3-k))
			current_brick.pattern[k][i] = current_brick.color; 
			else
			current_brick.pattern[k][i] = 0;
		}
	}
}
// find the lowest completed line, do the removal animation, add to score.
// returns true if a line was removed and false if there are none.
bool clearLine()
{
	int line_check;
	for( byte i = 0; i < field_height; i++ )
	{
		line_check = 0;

		for( byte k = 0; k < field_width; k++ )
		{
			if( wall[k][i] != 0)  
			line_check++;
		}

		if( line_check == field_width )
		{
			flashLine( i );
			for( int  k = i; k >= 0; k-- )
			{
				for( byte m = 0; m < field_width; m++ )
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
void nextBrick()
{
	current_brick.rotation = 0;
	current_brick.position_x = round(field_width / 2) - 2;
	current_brick.position_y = -3;

	current_brick.type = random( 0, 6 );

	current_brick.color = pgm_read_byte(&(brick_colors[ current_brick.type ]));



	updateBrickArray();

	//displayPreview();
}

//clears the wall for a new game
void clearWall()
{
	for( byte i = 0; i < field_width; i++ )
	{
		for( byte k = 0; k < field_height; k++ )
		{
			wall[i][k] = 0;
		}
	}
}

//effect, flashes the line at the given y position (line) a few times.  
void flashLine( int line ){

	bool state = 1;
	for(byte i = 0; i < 6; i++ )
	{
		for(byte k = 0; k < field_width; k++ )
		{  
			if(state)
			wall[k][line] = 0b00111111;
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
  for(int j=0; j < field_width; j++){
		for(int k = 0; k < field_height; k++ )
		{
			draw(wall[j][k],j,k);
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
			if(current_brick.pattern[j][k] != 0)
			{
				if( current_brick.position_y + k >= 0 )
				{
					draw(current_brick.color, current_brick.position_x + j, current_brick.position_y + k);
					//field[ position_x + j ][ p osition_y + k ] = current_brick_color;
				}
			}
		}
	}
	updateDisplay();
}

//takes a byte color values an draws it to pixel array at screen x,y values.
// Assumes a UP->Down->Down->Up (Shorest wire path) LED strips display.
void draw(byte color, byte x, byte y){
	
	unsigned short address=0;
	byte r,g,b;
	
	//calculate address
	if(x%2==0) //even row
	address=field_height*x+y;
	else //odd row
	address=((field_height*(x+1))-1)-y;
	
	if(color==0){
		rgb[address].r=0;
		rgb[address].g=0;
		rgb[address].b=0;
	}
	else{
		//calculate colors, map to LED system
		b=color&0b00000011;
		g=(color&0b00001100)>>2;
		r=(color&0b00110000)>>4;
		
		rgb[address].r=map(r,0,3,0,255); 
		rgb[address].g=map(g,0,3,0,255);
		rgb[address].b=map(b,0,3,0,255);
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

	WS2811RGB(rgb, ARRAYLEN(rgb));
}
