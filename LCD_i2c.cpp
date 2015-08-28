/*
Library to handle 'intelligent' i2c LCD controller boards. 
uses the library available at http://www.wentztech.com/radio/arduino/files/LCDI2C.html

The following functions are mandatory:
LCD_PRINT_ARROW_UP() - Print an up arrow
LCD_PRINT_ARROW_DOWN() - Print a down arrow
LCD_PRINT_ARROW_LEFT() - Print a left arror
LCD_PRINT_ARROW_RIGHT() - Print a right arrow

LCD_clear() - Clear the LCD screen
LCD_set_custom_characters() - Define the arrow/custom characters. Can be blank if not required
LCD_print_string_with_coords(char *string, int x, int y) - Print a string at a given location
LCD_big_number_mode - If available turn on 4 digit 'big number' mode. If this is not available the library should just blank the screen, turn on a flashing cursor and home to (0,0)
LCD_print_int - Print an int
*/
#include "config.h"
#ifdef lcd_i2c_h

#include "WProgram.h"
#include "LCD_i2c.h"
#include "LCDI2C.h"

LCDI2C i2clcd = LCDI2C(4,20,0x4C,1);             // Number of lines and i2c address of the display

LCD::LCD()
{
  
}

void LCD::LCD_clear()
{
   i2clcd.clear();
}

void LCD::LCD_set_custom_characters()
{
 
  //Define custom characters
  uint8_t up_arrow[8] = { 0x00, 0x04, 0x0e, 0x15, 0x04, 0x04, 0x04, 0x04 };
  //Serial.print("?D1 04 0E 15 04 04 04 04 04"); //Up arrow (Custom character #1)
  uint8_t down_arrow[8] = { 0x00, 0x04, 0x04, 0x04, 0x04, 0x15, 0x0E, 0x04 } ;
  //Serial.print("?D2 04 04 04 04 04 15 0E 04"); //Down arrow (Custom character #2)
  uint8_t up_arrow_inverted[8] = { 0x1B, 0x11, 0x0A, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B } ;
  //Serial.print("?D3 1B 11 0A 1B 1B 1B 1B 1B"); //Inverted Up arrow (Custom character #3)
  uint8_t down_arrow_inverted[8] = { 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x0A, 0x11, 0x1B } ;
  //Serial.print("?D4 1B 1B 1B 1B 1B 0A 11 1B"); //Inverted Down arrow (Custom character #4)
  /*
  //delay(100);
  //Serial.print("?D50804021F1F020408"); //Right facing arrow (Custom character #5)
  //delay(100);
  //Serial.print("?D60204081F1F080402"); //Left facing arrow (Custom character #6)
  //delay(100);
  
  i2clcd.load_custom_character(LCD_ARROW_UP, up_arrow);
  delay(100);
  i2clcd.load_custom_character(LCD_ARROW_DOWN, up_arrow);
  delay(100);
  
  i2clcd.load_custom_character(LCD_ARROW_UP, up_arrow);
  delay(100);
  i2clcd.load_custom_character(LCD_ARROW_UP, up_arrow);
  delay(100);  
  */
  //Serial.print("?c0"); //Set cursor to none mode
}

void LCD::LCD_big_number_mode(boolean is_big)
{
  
 if(is_big)
 {
   //i2clcd_cursor_on();
   i2clcd.blink_on();
   i2clcd.clear();
 }
 else
 {
   //i2clcd_cursor_off();
   i2clcd.blink_off();
   i2clcd.clear();
 }
 
}

void LCD::LCD_print_int(int to_print)
{
  i2clcd.print(to_print);
}

void LCD::LCD_print_string_with_coords(char *string, int x, int y)
{
  i2clcd.setCursor(x, y);
  i2clcd.print(string);
}
#endif
