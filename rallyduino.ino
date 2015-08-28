#undef int
#include <stdio.h>
#include <Wire.h>
#include <EEPROM.h>
#include <string.h>
#include "config.h"
#include "floatToString.h"
#include "TimerOne.h"
#include <stdlib.h>

#include "LCD_117.h"
#include "LCD_i2c.h"

#define UP 0
#define DOWN 2
#define LEFT 0
#define RIGHT 2
#define CENTER 1
#define PROBE_PIN 0 //Actually equals digital pin 2
#define MM_IN_KM 1000000 // Number of mm in a kilometre
#define MS_IN_HOUR 3600000 //Number of ms in an hour
#define MS_IN_MIN 60000 //Number of ms in a minute
#define MILE_MULTIPLIER 0.621371192

int CALIBRATOR = (1735/4); //1734.8 for 185/60R13 tire

volatile long count1 = 0; //must be signed as technically distance can go negative
volatile long count2 = 0;

//The two time variables are used to store reset times in ms
unsigned long time1 = 0;
unsigned long time2 = 0;

//const float MILE_MULTIPLIER = 0.621371192;
boolean USE_MILES_COUNT1 = false; //Whether or not to use miles for counter 1
boolean USE_MILES_COUNT2 = false; //Whether or not to use miles for counter 2

boolean DIR_FORWARD_COUNT1 = true; //Sets default direction for counter 1 to be forward
boolean DIR_FORWARD_COUNT2 = true; //Sets default direction for counter 2 to be forward

boolean COUNT1_ON = true; //Whether or not count1 is actually turned on
boolean COUNT2_ON = true; //Whether or not count2 is actually turned on

boolean SHOW_TIME_COUNT1 = true; //Whether or not to show the time for count1. Set false to show avg speed
boolean SHOW_TIME_COUNT2 = true; //Whether or not to show the time for count2. Set false to show avg speed

//Nunchuck controller details:
boolean NUNCHUCK_Z_BUTTON = false;
boolean NUNCHUCK_C_BUTTON = false;
byte NUNCHUCK_X_AXIS = CENTER;
byte NUNCHUCK_Y_AXIS = CENTER;
byte NUNCHUCK_AXIS_THRESHOLD_MIN = 40; //Amount of movement against each axis that will trigger a move in the up and left directions (Max is 127, min 0). Higher number means more sensitive
byte NUNCHUCK_AXIS_THRESHOLD_MAX = 210; //Amount of movement against each axis that will trigger a move in the down and right directions (Max is 129, min 256). Lower number means more sensitive

//Layout properties
//byte DISTANCE1_X, DISTANCE1_Y, 

LCD lcd; //Create an LCD object

/*
Cursor Positon map:
  ----------------------
  |Count1  3   5  AvgSp|
  |   1        |  7    |
  |Count2      6  AvgSp|
  |   2    4   |  8    |
  ----------------------
*/
int CURSOR_POS = 1;

void setup()
{
  delay(3000);
  
  Wire.begin();		// join i2c bus with address 0x52
  nunchuck_init(); // send the initilization handshake  
  Serial.begin(9600);
  setup_lcd();
  delay(50);
  
  //Do a check to see if the C button is held down, if it is, start the calibration routine
  decode_nunchuck(false);
  //decode_nunchuck(false); //Not sure why but need to call this twice before it registers
  if(NUNCHUCK_C_BUTTON) { calibrate(); }
  //Attempt to set the calibrator number up, if this fails, run the calibration routine
  else if(!set_calibrator()) { calibrate(); }
  
  draw_primary_headings();
  draw_secondary_headings();
  
  //Startup combination for simulation routine
  if(NUNCHUCK_Z_BUTTON && (NUNCHUCK_Y_AXIS == UP))
  {
    //Use the Timer1 library to simulate pulses
    Timer1.initialize(); // initialize timer1
    Timer1.attachInterrupt(pulse, 8000); //Pulse every 8ms. Simulates 160km/h on 185/60R13 tire, 4 wheel studs
  }
  else {attachInterrupt(PROBE_PIN, pulse, RISING); } //Attach the interrupt that gets called every time there is a pulse 
  delay(50);
}

void nunchuck_init ()
{
  Wire.beginTransmission (0x52);	// transmit to device 0x52
  Wire.write((byte)0x40);		// sends memory address
  Wire.write((byte)0x00);		// sends sent a zero.  
  Wire.endTransmission ();	// stop transmitting
  
  delay(50);
  Wire.beginTransmission (0x52);	// transmit to device 0x52
  Wire.write((byte)0x00);		// sends one byte
  Wire.endTransmission ();	// stop transmitting
}

  

int LOOP_COUNTER = 0;
void loop()
{
  decode_nunchuck(true);
  //process_input();
  
  LOOP_COUNTER++;
  //We only do a screen redraw once every x loops
  //This means we get sufficiently up to date data displayed, but do not flood the LCD
  //Note that cursor movements / button presses still update frequently as they are part of decode_nunchuck()
  if( (LOOP_COUNTER % 30) == 0)
  {
    redraw_lcd();
    LOOP_COUNTER = 0;
    //delay(100);
  }
  
  delay(10);

}

/*
Attempts to set the CALIBRATOR variable by looking up EEPROM.
Returns true on success or false on fail
*/
boolean set_calibrator()
{
  //Attempt to get the calibration number from the EEPROM
  //the first 4 bytes of the EEPROM are used to store the 4 digits of a calibration figure in the range 0000-9999
  byte cal_1 = byte( EEPROM.read(0) );
  byte cal_2 = byte( EEPROM.read(1) );
  byte cal_3 = byte( EEPROM.read(2) );
  byte cal_4 = byte( EEPROM.read(3) );
  
  //Do a check to see if any of the retrieved values are greater than 9. If yes, this means that the EEPROM is virgin, never been used before
  //If this is the case, we reset the value to 0
  if( cal_1 > 9) { cal_1 = 0; }
  if( cal_2 > 9) { cal_2 = 0; }
  if( cal_3 > 9) { cal_3 = 0; }
  if( cal_4 > 9) { cal_4 = 0; }
  
   //If all 4 calibration figures are 0, then we've failed (Chances are this is a new arduino with no calibration number set)
  if( (cal_1 + cal_2 + cal_3 + cal_4) == 0 ) { return false; }
  
  //Set the CALIBRATOR value based on each cal value
  CALIBRATOR = (cal_1 * 1000) + (cal_2 * 100) + (cal_3 * 10) + cal_4;
  
  return true;
}

//Runs through the calibration process
void calibrate()
{
  
  lcd.LCD_big_number_mode(true);
  
  //Attempt to get the calibration number from the EEPROM
  //the first 4 bytes of the EEPROM are used to store the 4 digits of a calibration figure in the range 0000-9999
  byte cal_1 = byte( EEPROM.read(0) );
  byte cal_2 = byte( EEPROM.read(1) );
  byte cal_3 = byte( EEPROM.read(2) );
  byte cal_4 = byte( EEPROM.read(3) );
  
  //Do a check to see if any of the retrieved values are greater than 9. This means that the EEPROM is virgin, never been used before
  //If this is the case, we reset the value to 0
  if( cal_1 > 9) { cal_1 = 0; }
  if( cal_2 > 9) { cal_2 = 0; }
  if( cal_3 > 9) { cal_3 = 0; }
  if( cal_4 > 9) { cal_4 = 0; }
  
  //We create some new cal values, initial value is simply the original ones
  byte new_cal_values[] = {cal_1, cal_2, cal_3, cal_4};
  
  byte cur = 0; // Cursor position  
  //We run through a loop until the c button on the nunchuck is pressed
  boolean continue_check = true;
  boolean changed = true;
  
  while (continue_check)
  {
    decode_nunchuck(false); //Get latest values from nunchuck
    //NEED TO ACTUALLY DO STUFF HERE
    continue_check = !(NUNCHUCK_C_BUTTON && NUNCHUCK_Z_BUTTON); //Continue until c+z buttons is true (ie both buttons held down together)
    
    //Up
    if(NUNCHUCK_Y_AXIS == UP)
    {
      changed = true;
      new_cal_values[cur] += 1;
      if(new_cal_values[cur] > 9) { new_cal_values[cur] = 0; }
    }
    //Down
    else if(NUNCHUCK_Y_AXIS == DOWN)
    {
      changed = true;
      new_cal_values[cur] -= 1;
      if(new_cal_values[cur] < 0) { new_cal_values[cur] = 9; }      
    }
    
    //Left
    if(NUNCHUCK_X_AXIS == LEFT)
    {
      if(cur != 0) { cur -= 1; }
    }
    //Right
    else if(NUNCHUCK_X_AXIS == RIGHT)
    {
      if(cur != 3) { cur += 1; }
    }  
    
    if(changed)
    {
      lcd.LCD_clear();
      lcd.LCD_print_int(new_cal_values[0]);
      lcd.LCD_print_int(new_cal_values[1]);
      lcd.LCD_print_int(new_cal_values[2]);
      lcd.LCD_print_int(new_cal_values[3]);
      changed = false;
    }
    
    delay(200); //Need a delay or else the nunchuck flips out
  }
  
  //Write the values back to the EEPROM only if they are different to the original values (Want to minimise the number of EEPROM writes)
  if( cal_1 != new_cal_values[0] ) { EEPROM.write(0, new_cal_values[0]); }
  if( cal_2 != new_cal_values[1] ) { EEPROM.write(1, new_cal_values[1]); }
  if( cal_3 != new_cal_values[2] ) { EEPROM.write(2, new_cal_values[2]); }
  if( cal_4 != new_cal_values[3] ) { EEPROM.write(3, new_cal_values[3]); }
  
  set_calibrator();
  lcd.LCD_big_number_mode(false);
  
}

/*
Draws all the (relatively) static headings onto the screen
*/
void setup_lcd()
{
  /*
  ----------------------
  |Count1         AvgSp|
  |       km   UD      |
  |Count2         AvgSp|
  |       km   UD      |
  ----------------------
  
  or alternatively:
  
  ----------------------
  |Count1         Time |
  |       mi   UD      |
  |Count2         Time |
  |       mi   UD      |
  ----------------------
  
  or with 2x20:
  ----------------------
  |       mi   UD      |
  |       mi   UD      |
  ----------------------
  */
  
  lcd.LCD_set_custom_characters();
  lcd.LCD_clear();
  
}

//Draws the headings on rows 1 and 3
void draw_primary_headings()
{
  //Check LCD geometry. These headings are only printed if we have 4 rows
  if(LCD_ROWS < 4) { return; }

  
  //Start Drawing headings
  char headings1[20];
  //sprintf(headings1, "Dist1        %s", (SHOW_TIME_COUNT1?"  Time":"Avg.Sp"));
  if(SHOW_TIME_COUNT1) { strcpy(headings1, "Dist1          Time"); }
  else { strcpy(headings1, "Dist1        Avg.Sp"); }
  lcd.LCD_print_string_with_coords(headings1, 0, 0);
  
  //Headings on 3rd row
  char headings2[20];
  if(SHOW_TIME_COUNT2) { strcpy(headings2, "Dist2          Time"); }
  else { strcpy(headings2, "Dist2        Avg.Sp"); }
  lcd.LCD_print_string_with_coords(headings2, 0, 2);
  
}

//Draws the headings on rows 2 and 4
void draw_secondary_headings()
{
 
  char heading2[6];
  char heading4[6];
  
  sprintf(heading2, "%s %s", (USE_MILES_COUNT1?"Mi":"Km"), (DIR_FORWARD_COUNT1?LCD_ARROW_UP:LCD_ARROW_DOWN));
  sprintf(heading4, "%s %s", (USE_MILES_COUNT2?"Mi":"Km"), (DIR_FORWARD_COUNT2?LCD_ARROW_UP:LCD_ARROW_DOWN));
  
  int output_row1, output_row2, output_col;
  output_col = 7;
  if(LCD_ROWS < 4)
  { 
    output_row1 = 0;
    output_row2 = 1;
  }
  else
  {
    output_row1 = 1;
    output_row2 = 3;
  }
  
  lcd.LCD_print_string_with_coords(heading2, output_col, output_row1);
  lcd.LCD_print_string_with_coords(heading4, output_col, output_row2);

}

void redraw_lcd()
{
  
  //Calculate and print distance info (in km)
  float distance_1 = float(count1 * CALIBRATOR) / float(MM_IN_KM);
  if(USE_MILES_COUNT1) { distance_1 *= MILE_MULTIPLIER; }
  float distance_2 = float(count2 * CALIBRATOR) / float(MM_IN_KM);
  if(USE_MILES_COUNT2) { distance_2 *= MILE_MULTIPLIER; }
  
  //Temporary buffers for distance strings
  char distance1_string[7];
  char distance2_string[7];
  
  //Set position and print distance 1
  //dtostrf(distance_1,7,2,distance1_string);
  floatToString(distance1_string, distance_1, 2);
  lcd.LCD_print_string_with_coords(distance1_string, 0, 1);
  
  //Set position and print distance 2  
  floatToString(distance2_string, distance_2, 2);
  lcd.LCD_print_string_with_coords(distance2_string, 0, 3);
  
  //Calculate and print time or avg speed info
  //First get the current time in ms (Minus the last time the counter was reset)
  unsigned long cur_time1 = millis() - time1;
  unsigned long cur_time2 = millis() - time2;
  
  if(SHOW_TIME_COUNT1)
  {
    int hours1 = cur_time1 / MS_IN_HOUR;
    int mins1 = (cur_time1 % MS_IN_HOUR)/MS_IN_MIN;
    int secs1 = (cur_time1 % MS_IN_MIN) / 1000;
  
    //Convert above values to formatted strings (hh:mm:ss)
    char cur_time1_str[9];
    
    sprintf(cur_time1_str, "%1d:%02d:%02d", hours1, mins1, secs1);
    lcd.LCD_print_string_with_coords(cur_time1_str, 13, 1);
  }
  else
  {
    //We do average speed
    float hours1 = float(cur_time1) / float(MS_IN_HOUR);
    float avg_speed1 = float(distance_1) / float(hours1);
    
    char avg_speed1_string[7]; //String buffer for the result
    floatToString(avg_speed1_string, avg_speed1, 2);
    lcd.LCD_print_string_with_coords(avg_speed1_string, 14, 1);
  }
  
  if(SHOW_TIME_COUNT2)
  {
    //Do the same as above but for count2
    int hours2 = cur_time2 / MS_IN_HOUR;
    int mins2 = (cur_time2 % MS_IN_HOUR) / MS_IN_MIN;
    int secs2 = (cur_time2 % MS_IN_MIN) / 1000;
  
    char cur_time2_str[9];
  
    sprintf(cur_time2_str, "%1d:%02d:%02d", hours2, mins2, secs2);
    lcd.LCD_print_string_with_coords(cur_time2_str, 13, 3);
  }
  else
  {
    //We do average speed
    float hours2 = float(cur_time2) / float(MS_IN_HOUR);
    float avg_speed2 = float(distance_2) / float(hours2);
    
    char avg_speed2_string[7]; //String buffer for the result
    floatToString(avg_speed2_string, avg_speed2, 2);
    lcd.LCD_print_string_with_coords(avg_speed2_string, 14, 3);
  }
  
}

void update_nunchuck_xaxis()
{
   /*
Cursor Positon map:
  ----------------------
  |Count1  3   5 7AvgSp|
  |   1        |       |
  |Count2      6 8AvgSp|
  |   2    4   |       |
  ----------------------
*/
  byte new_pos = CURSOR_POS;
  switch (NUNCHUCK_X_AXIS) {
    case LEFT: //Left
      if(CURSOR_POS > 2) { new_pos = CURSOR_POS - 2; }
      break;
    case RIGHT: //Right
      if(CURSOR_POS < 7) { new_pos = CURSOR_POS + 2; }
      break;
  } 
  set_cursor_pos(new_pos);
}

void update_nunchuck_yaxis()
{
  int new_pos = CURSOR_POS;
  switch (NUNCHUCK_Y_AXIS) {
    case UP: //Up
      if( (CURSOR_POS % 2) == 0 ) { new_pos = CURSOR_POS - 1; }
      break;
    case DOWN: //Down
      if( (CURSOR_POS % 2) == 1 ) { new_pos = CURSOR_POS + 1; }
      break;
  }
  set_cursor_pos(new_pos);
}

void update_nunchuck_zbutton()
{
  
   //************************************************************************
  //Handle button presses
  if (NUNCHUCK_Z_BUTTON)
  {
    switch (CURSOR_POS){
      case 1:
        count1 = 0;
        time1 = millis();
        break;
      case 2:
        count2 = 0;
        time2 = millis();
        break;
      case 3:
        USE_MILES_COUNT1 = !USE_MILES_COUNT1;
        draw_secondary_headings();
        break;
      case 4:
        USE_MILES_COUNT2 = !USE_MILES_COUNT2;
        draw_secondary_headings();
        break;
      case 5:
        DIR_FORWARD_COUNT1 = !DIR_FORWARD_COUNT1;
        draw_secondary_headings();
        break;
      case 6:
        DIR_FORWARD_COUNT2 = !DIR_FORWARD_COUNT2;
        draw_secondary_headings();
        break;
      case 7:
        //Switch count1 between time and average speed
        SHOW_TIME_COUNT1 = !SHOW_TIME_COUNT1;
        draw_primary_headings(); //Redraw headings do to change
        set_cursor_pos(CURSOR_POS);
        //Need to clear the first character from avg speed/time 
        lcd.LCD_print_string_with_coords(" ", 13, 1);
        break;
      case 8:
        //Switch count2 between time and average speed
        SHOW_TIME_COUNT2 = !SHOW_TIME_COUNT2;
        draw_primary_headings(); //Redraw headings do to change
        set_cursor_pos(CURSOR_POS);
        //Need to clear the first character from avg speed/time 
        lcd.LCD_print_string_with_coords(" ", 13, 3);
        break;
    
    }
  } 
  
}

void set_cursor_pos(int new_pos)
{
  /*
  Cursor Positon map:
  ----------------------
  |Count1  3   5 7AvgSp|
  |       1    |       |
  |Count2  4   6 8AvgSp|
  |       2    |       |
  ----------------------
  */

  //Need to clear the previous cursor position
  int x, y;
  switch(CURSOR_POS)
  {
    case 1:
      x = 6;
      y = 1;
      break;
    case 2:
      x = 6;
      y = 3;
      break;
    case 3:
       x = 7;
       y = 0;
       break;
    case 4:
       x = 7;
       y = 2;
       break;
    case 5:
       x = 10;
       y = 0;
       break;
    case 6:
       x = 10;
       y = 2;
       break;
    case 7:
       x = 12;
       y = 0;
       break;
    case 8:
       x = 12;
       y = 2;
       break;
  }
  lcd.LCD_print_string_with_coords(" ", x, y);
  
  switch(new_pos)
  {
    case 1:
      lcd.LCD_print_string_with_coords(LCD_ARROW_RIGHT, 6, 1);
      break;
    case 2:
      lcd.LCD_print_string_with_coords(LCD_ARROW_RIGHT, 6, 3);
      break;
    case 3:
      lcd.LCD_print_string_with_coords(LCD_ARROW_DOWN, 7, 0);
      break;
    case 4:
      lcd.LCD_print_string_with_coords(LCD_ARROW_DOWN, 7, 2);
      break;
    case 5:
      lcd.LCD_print_string_with_coords(LCD_ARROW_DOWN, 10, 0);
      break;
    case 6:
      lcd.LCD_print_string_with_coords(LCD_ARROW_DOWN, 10, 2);
      break;
    case 7:
      lcd.LCD_print_string_with_coords(LCD_ARROW_LEFT, 12, 0);
      break;
    case 8:
      lcd.LCD_print_string_with_coords(LCD_ARROW_LEFT, 12, 2);
      break;
  }
  
  CURSOR_POS = new_pos;
}

/*
This function reads the status of the nunchuck controller
If there has been changes since the last read, and do_updates is true, the relevant function will be called to update the cursor and lcd
*/
void decode_nunchuck(boolean do_updates)
{
  
  int cnt = 0;
  uint8_t outbuf[6];
  
  Wire.requestFrom (0x52, 6);	// request data from nunchuck
  while (Wire.available ())
  {
    outbuf[cnt] = (Wire.read()^0x17) + 0x17; //Receive and decode 1 byte //nunchuk_decode_byte (Wire.receive ());
    cnt++;
  }
  
  if (cnt < 5) { return; } //Return on fail (There must be at least 6 bytes returned from the nunchuck)
  
  boolean changed = false; //Represents whether or not anything has changed
  int joy_x_axis = outbuf[0];
  int joy_y_axis = outbuf[1];
  
  
  // X Axis changes
  if (int(joy_x_axis) < NUNCHUCK_AXIS_THRESHOLD_MIN) 
  {
    changed = (NUNCHUCK_X_AXIS != -1);
    NUNCHUCK_X_AXIS = LEFT; //Left
  } 
  else {
  if (int(joy_x_axis) > NUNCHUCK_AXIS_THRESHOLD_MAX) 
  {
    changed = (NUNCHUCK_X_AXIS != 1);
    NUNCHUCK_X_AXIS = RIGHT;  //Right
  }
  else {NUNCHUCK_X_AXIS = CENTER; }//Center
  }
  if(changed && do_updates) { update_nunchuck_xaxis(); }
  
  changed = false;
  //Y Axis Changes
  if (int(joy_y_axis) < NUNCHUCK_AXIS_THRESHOLD_MIN) 
  {
    //Down
    changed = (NUNCHUCK_Y_AXIS != UP);
    NUNCHUCK_Y_AXIS = DOWN; 
  } 
  else {
  if (int(joy_y_axis) > NUNCHUCK_AXIS_THRESHOLD_MAX) 
  {
     //Up
    changed = (NUNCHUCK_Y_AXIS != DOWN);
    NUNCHUCK_Y_AXIS = UP;
  }
  else {NUNCHUCK_Y_AXIS = CENTER; }//Center
  }
  if(changed && do_updates) { update_nunchuck_yaxis(); }
  
  //Update for buttons
  // byte outbuf[5] contains bits for z and c buttons
  changed = false;
  if ((outbuf[5] >> 0) & 1)
  { 
    changed = (NUNCHUCK_Z_BUTTON != false);
    NUNCHUCK_Z_BUTTON = false; 
  }
  else
  { 
    changed = (NUNCHUCK_Z_BUTTON != true);
    NUNCHUCK_Z_BUTTON = true; 
  }
  if(changed && do_updates) { update_nunchuck_zbutton(); }
  
  //C button
  changed = false;
  if ((outbuf[5] >> 1) & 1)
  { 
    changed = (NUNCHUCK_C_BUTTON != false);
    NUNCHUCK_C_BUTTON = false; 
  }
  else
  { 
    changed = (NUNCHUCK_C_BUTTON != true);
    NUNCHUCK_C_BUTTON = true; 
  }


  Wire.beginTransmission (0x52);	// transmit to device 0x52
  Wire.write((byte)0x00);		// sends one byte
  Wire.endTransmission ();	// stop transmitting
  
 
}

/*
This function is called every time there is a pulse from the probe (eg: a lot)
It is called as an interrupt
*/
void pulse()
{
  //If we're going forward, increment the counter, else decrement it
  if(COUNT1_ON)
  {
    if(DIR_FORWARD_COUNT1)
    { count1 ++; }
    else
    { count1 --; }
  }

  if(COUNT2_ON)
  {
    if(DIR_FORWARD_COUNT2)
    { count2 ++; }
    else
    { count2 --; }
  }
}

