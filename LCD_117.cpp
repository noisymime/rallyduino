/*
Library to handle the #117 serial->LCD controller board

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
#ifdef lcd_117_h

//#include "WProgram.h"
#include "LCD_117.h"

LCD::LCD()
{
  
}

void LCD::LCD_clear()
{
 Serial.print("?f");
 delay(100);
}

void LCD::LCD_set_custom_characters()
{
  //Define custom characters
  Serial.print("?D1040E150404040404"); //Up arrow (Custom character #1)
  delay(100);
  Serial.print("?D20404040404150E04"); //Down arrow (Custom character #2)
  delay(200);
  Serial.print("?D31B110A1B1B1B1B1B"); //Inverted Up arrow (Custom character #3)
  delay(200);
  Serial.print("?D41B1B1B1B1B0A111B"); //Inverted Down arrow (Custom character #4)
  delay(100);
  Serial.print("?D50804021F1F020408"); //Right facing arrow (Custom character #5)
  delay(100);
  Serial.print("?D60204081F1F080402"); //Left facing arrow (Custom character #6)
  delay(100);
  
  Serial.print("?c0"); //Set cursor to none mode
}

void LCD::LCD_big_number_mode(boolean is_big)
{
 if(is_big)
 {
   Serial.print("?>4"); //Enter big number (4) mode
   delay(50);
   Serial.print("?c2"); //Set cursor to blink mode
   delay(50);
   Serial.print("?f"); //Clear LCD
   delay(50);
 }
 else
 {
   Serial.print("?c0"); //Set cursor to none mode
   Serial.print("?<"); //Exit big number mode
   Serial.print("?R"); //Restore custom characters
   delay(50);
 }
}

void LCD::LCD_print_int(int to_print)
{
  Serial.print(to_print);
  delay(50);
}

void LCD::LCD_print_string_with_coords(char *string, int x, int y)
{
  //Format the coord string
  char coord_str[9];
  sprintf(coord_str, "?x%02d?y%1d", x, y);
  Serial.print(coord_str);
  delay(25);
  
  Serial.print(string);
  delay(40); 
}
#endif
