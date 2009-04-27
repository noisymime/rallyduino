/*

LCDI2C v0.4 3/Mar/2009 dale@wentztech.com  http://wentztech.com/Radio


What is this?

An arduino library for use with the web4robot.com i2C LCD Display in I2C more

Uses I2c Wires interface

Uses Analog pin 4 - SDA
Uses Analog pin 5 - SCL


Usage:

see the examples folder of this library distribution.


*/


  #include <Wire.h>


 #include <string.h> //needed for strlen()

  #include <inttypes.h>

  #include "WConstants.h"  //all things wiring / arduino
  
  #include "LCDI2C.h"
  

#define LCDI2C_MAX_STRLEN			40
#define LCDI2C_PRINT_STR_DELAY		20


//--------------------------------------------------------

// (don't change here - specify on calling constructor)

//how many lines has the LCD? 

int g_num_lines = 2;

int g_num_col = 16;

// Defalt address of the display

int g_i2caddress = 0x4C;

int g_display = 0;


//stuff the library user might call---------------------------------

//constructor.  num_lines must be 1, 2, 3, or 4 currently.

LCDI2C::LCDI2C (int num_lines,int num_col,int i2c_address,int display){
	
	g_num_lines = num_lines;
	g_num_col = num_col;
	g_i2caddress = i2c_address;
	g_display = display;
	
	if (g_num_lines < 1 || g_num_lines > 4){
		
		g_num_lines = 2;
		
	}
	
	if (g_num_col < 1 || g_num_col > 40){
		
		g_num_col = 16;
	}
	
}

//	Send a command to the display that is not supported

void LCDI2C::command(int value) {

  Wire.beginTransmission(g_i2caddress);
  Wire.send(0xFE);
  Wire.send(value);
  Wire.endTransmission();
  delay(CMDDELAY);
}


//Used by the print library to get information to the display

void LCDI2C::write(uint8_t value) {

  Wire.beginTransmission(g_i2caddress);
  Wire.send(value);
  Wire.endTransmission();
  delay(5);

}




//send the clear screen command to the LCD

void LCDI2C::clear(){

      Wire.beginTransmission(g_i2caddress);
      Wire.send(0xFE);
      Wire.send(0x14);
      Wire.endTransmission();
      delay(CMDDELAY);  


}

//send the Home Cursor command to the LCD      ********** Not Working ***************

void LCDI2C::home(){

	setCursor(0,0);					// The command to home the cursor does not work on the version of the dislay I have
									// So we do it this way.
}


//Turn the LCD ON

void LCDI2C::on(){

      Wire.beginTransmission(g_i2caddress);
      Wire.send(0xFE);
      Wire.send(0x0A);
      Wire.endTransmission();
      delay(CMDDELAY);  
}


// Turn the LCD OFF

void LCDI2C::off(){

      Wire.beginTransmission(g_i2caddress);
      Wire.send(0xFE);
      Wire.send(0x0B);
      Wire.endTransmission();
      delay(CMDDELAY);
        
}


//Turn the Underline Cursor ON

void LCDI2C::cursor_on(){

      Wire.beginTransmission(g_i2caddress);
      Wire.send(0xFE);
      Wire.send(0x0E);
      Wire.endTransmission();
      delay(CMDDELAY);
        
}


//Turn the Underline  Cursor OFF

void LCDI2C::cursor_off(){

      Wire.beginTransmission(g_i2caddress);
      Wire.send(0xFE);
      Wire.send(0x0F);
      Wire.endTransmission();
      delay(CMDDELAY);
      
}


//Turn the Underline Cursor ON

void LCDI2C::blink_on(){

      Wire.beginTransmission(g_i2caddress);
      Wire.send(0xFE);
      Wire.send(0x12);
      Wire.endTransmission();
      delay(CMDDELAY);
        
}


//Turn the Underline  Cursor OFF

void LCDI2C::blink_off(){

      Wire.beginTransmission(g_i2caddress);
      Wire.send(0xFE);
      Wire.send(0x13);
      Wire.endTransmission();
      delay(CMDDELAY);
      
}


//Move the cursor left 1 space

void LCDI2C::left(){

      Wire.beginTransmission(g_i2caddress);
      Wire.send(0xFE);
      Wire.send(0x10);
      Wire.endTransmission();
      delay(CMDDELAY);
        
}


//Move the cursor right 1 space

void LCDI2C::right(){

      Wire.beginTransmission(g_i2caddress);
      Wire.send(0xFE);
      Wire.send(0x11);
      Wire.endTransmission();
      delay(CMDDELAY);
      
}


// initiatize lcd after a short pause

//while there are hard-coded details here of lines, cursor and blink settings, you can override these original settings after calling .init()

void LCDI2C::init () {

      Wire.begin();
      on();
      clear();
      blink_off();
      cursor_off(); 
      home();
      
    
     
}



void LCDI2C::setCursor(int line_num, int x){

      Wire.beginTransmission(g_i2caddress);
      Wire.send(0xFE);
      Wire.send(0x0C);
      Wire.send(line_num);
      Wire.send(x);
      Wire.endTransmission();
      delay(100);

}



int LCDI2C::keypad (){

  int data = 0;

  //  Send Keypad read command
  Wire.beginTransmission(g_i2caddress);
  Wire.send(0xFE);
  Wire.send(0x1B);
  Wire.endTransmission();
  delay(CMDDELAY);
  
  //  Connect to device and request byte
  Wire.beginTransmission(g_i2caddress);
  Wire.requestFrom(g_i2caddress, 1);

  if (Wire.available()) {
    data = Wire.receive();
  }


return data;
}

unsigned char LCDI2C::init_bargraph(unsigned char graphtype)
{
	switch (graphtype)
		{
		case LCDI2C_VERTICAL_BAR_GRAPH:
				Wire.beginTransmission(g_i2caddress);
				Wire.send(0xFE);
				Wire.send(0x18);
				Wire.endTransmission();
				break;
		case LCDI2C_HORIZONTAL_BAR_GRAPH:
				Wire.beginTransmission(g_i2caddress);
				Wire.send(0xFE);
				Wire.send(0x16);
				Wire.send(0x00);
				Wire.endTransmission();
				break;
		case LCDI2C_HORIZONTAL_LINE_GRAPH:
				Wire.beginTransmission(g_i2caddress);
				Wire.send(0xFE);
				Wire.send(0x16);
				Wire.send(0x01);
				Wire.endTransmission();
				break;
		default:
				return 1;
		}
	
	return 0;
}

void LCDI2C::draw_horizontal_graph(unsigned char row, unsigned char column, unsigned char len,  unsigned char pixel_col_end)
{
	Wire.beginTransmission(g_i2caddress);
	Wire.send(0xFE);
	Wire.send(0x17);
	Wire.send(row);
	Wire.send(column);
	Wire.send(len);
	Wire.send(pixel_col_end);
	Wire.endTransmission();
}

void LCDI2C::draw_vertical_graph(unsigned char row, unsigned char column, unsigned char len,  unsigned char pixel_row_end)
{
	Wire.beginTransmission(g_i2caddress);
	Wire.send(0xFE);
	Wire.send(0x19);
	Wire.send(row);
	Wire.send(column);
	Wire.send(len);
	Wire.send(pixel_row_end);
	Wire.endTransmission();
}

void LCDI2C::load_custom_character(unsigned char char_num, unsigned char *rows)
{
	unsigned char i;

	Wire.beginTransmission(g_i2caddress);
	Wire.send(0xFE);
	Wire.send(0x1A);
	Wire.send(char_num);
	for (i = 0; i < LCDI2C_CUSTOM_CHAR_SIZE; i++)
		Wire.send(rows[i]);
	Wire.endTransmission();
}

unsigned char LCDI2C::set_backlight_brightness(unsigned char new_val)
{
	if ((new_val < LCDI2C_MIN_BRIGHTNESS)
			|| (new_val > LCDI2C_MAX_BRIGHTNESS))
		return LCDI2C_VALUE_OUT_OF_RANGE;
	
	Wire.beginTransmission(g_i2caddress);
	Wire.send(0xFE);
	Wire.send(0x03);
	Wire.send(new_val);
	Wire.endTransmission();
	return 0;
}


unsigned char LCDI2C::set_contrast(unsigned char new_val)
{
	if ((new_val < LCDI2C_MIN_CONTRAST)
			|| (new_val > LCDI2C_MAX_CONTRAST))
		return LCDI2C_VALUE_OUT_OF_RANGE;
	
	Wire.beginTransmission(g_i2caddress);
	Wire.send(0xFE);
	Wire.send(0x04);
	Wire.send(new_val);
	Wire.endTransmission();
	return 0;
}

// Overload 
void  LCDI2C::printstr(const char c[])
{
	byte len;

	while (*c)
		{
		len = min(strlen(c), LCDI2C_MAX_STRLEN);
		Wire.beginTransmission(g_i2caddress);
		Wire.send(0xFE);
		Wire.send(0x15);
		Wire.send(len);
		while (len--)
			Wire.send(*c++);
		Wire.endTransmission();
		if (*c)
			delay(LCDI2C_PRINT_STR_DELAY);	// More to send.  Wait a bit
		}
}

