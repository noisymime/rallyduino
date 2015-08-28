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
#ifdef lcd_117_h


//#include "WConstants.h"
#include "Arduino.h"
#include <stdio.h>
//#include <WProgram.h> 

//Macros
#define LCD_PRINT_ARROW_UP() Serial.print("?1"); delay(50);
#define LCD_PRINT_ARROW_DOWN() Serial.print("?2"); delay(50);
#define LCD_PRINT_ARROW_LEFT() Serial.print("?5"); delay(50);
#define LCD_PRINT_ARROW_RIGHT() Serial.print("?6"); delay(50);
#define LCD_ARROW_UP "?1"
#define LCD_ARROW_DOWN "?2"
#define LCD_ARROW_LEFT "?5"
#define LCD_ARROW_RIGHT "?6"

class LCD
{
  public:
    LCD();
    void LCD_clear();
    void LCD_set_custom_characters();
    void LCD_print_string_with_coords(char *string, int x, int y);
    void LCD_big_number_mode(boolean is_big);
    void LCD_print_int(int to_print);
};
#endif
