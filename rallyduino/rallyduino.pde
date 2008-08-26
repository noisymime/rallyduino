#undef int
#include <stdio.h>
#include <Wire.h>
#include <EEPROM.h>
#include "floatToString.h"
#include "TimerOne.h"
#include "LCD_117.h"

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
int NUNCHUCK_X_AXIS = 0; // <0 = Left, 0 = Center, >0 = Right
int NUNCHUCK_Y_AXIS = 0; // <0 = Up, 0 = Center, >0 = Down

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
  Wire.begin ();		// join i2c bus with address 0x52
  nunchuck_init (); // send the initilization handshake
  
  Serial.begin(9600);
  
  //calibrate(); //Run through the calibration routine
  setup_lcd();
  draw_primary_headings();
  draw_secondary_headings();
  //attachInterrupt(PROBE_PIN, pulse, RISING); //Attach the interrupt that gets called every time there is a pulse
  //Use the Timer1 library to simulate pulses
  Timer1.initialize(); // initialize timer1
  Timer1.attachInterrupt(pulse, 8000); //Pulse every 8ms. Simulates 160km/h on 185/60R13 tire, 4 wheel studs
  
  delay(50);
}

void nunchuck_init ()
{
  Wire.beginTransmission (0x52);	// transmit to device 0x52
  Wire.send (0x40);		// sends memory address
  Wire.send (0x00);		// sends sent a zero.  
  Wire.endTransmission ();	// stop transmitting
}

  

int LOOP_COUNTER = 0;
void loop()
{
  decode_nunchuck();
  //process_input();
  
  LOOP_COUNTER++;
  //We only do a screen redraw once every 20 loops
  //This means we get sufficiently up to date data displayed, but do not flood the LCD
  //Note that cursor movements / button presses still update frequently as they are part of decode_nunchuck()
  if( (LOOP_COUNTER % 20) == 0)
  {
    redraw_lcd();
    LOOP_COUNTER = 0;
    //delay(100);
  }
  
  delay(10);

}

//Runs through the calibration process
void calibrate()
{
  Serial.print("?>4"); //Enter big number (4) mode
  delay(50);
  Serial.print("?c2"); //Set cursor to blink mode
  delay(50);
  Serial.print("?f"); //Clear LCD
  delay(50);
  
  //Attempt to get the calibration number from the EEPROM
  //the first 4 bytes of the EEPROM are used to store the 4 digits of a calibration figure in the range 0000-9999
  int cal_1 = int( EEPROM.read(0) );
  int cal_2 = int( EEPROM.read(1) );
  int cal_3 = int( EEPROM.read(2) );
  int cal_4 = int( EEPROM.read(3) );
  
  //Do a check to see if any of the retrieved values are greater than 9. This means that the EEPROM is virgin, never been used before
  //If this is the case, we reset the value to 0
  if( cal_1 > 9) { cal_1 = 0; }
  if( cal_2 > 9) { cal_2 = 0; }
  if( cal_3 > 9) { cal_3 = 0; }
  if( cal_4 > 9) { cal_4 = 0; }
  
  //We create some new cal values, initial value is simply the original ones
  int new_cal_1 = cal_1;
  int new_cal_2 = cal_2;
  int new_cal_3 = cal_3;
  int new_cal_4 = cal_4;
  
  int cur = 0; // Cursor position  
  //We run through a loop until the c button on the nunchuck is pressed
  boolean continue_check = true;
  while (continue_check)
  {
    decode_nunchuck(); //Get latest values from nunchuck
    //NEED TO ACTUALLY DO STUFF HERE
    continue_check = !NUNCHUCK_C_BUTTON; //Continue until c_button is true (ie pressed)
    
    Serial.print("?f"); //Clear LCD
    Serial.print("new_cal_1");
    Serial.print("new_cal_2");
    Serial.print("new_cal_3");
    Serial.print("new_cal_4");
  }
  
  //Write the values back to the EEPROM only if they are different to the original values (Want to minimise the number of EEPROM writes)
  if( cal_1 != new_cal_1 ) { EEPROM.write(0, new_cal_1); }
  if( cal_2 != new_cal_2 ) { EEPROM.write(0, new_cal_2); }
  if( cal_3 != new_cal_3 ) { EEPROM.write(0, new_cal_3); }
  if( cal_4 != new_cal_4 ) { EEPROM.write(0, new_cal_4); }
  
  //Set the CALIBRATOR value based on each new_cal value
  CALIBRATOR = (new_cal_1 * 1000) + (new_cal_2 * 100) + (new_cal_3 * 10) + new_cal_4;

  //EEPROM.write(address, value)
  Serial.print("?c3"); //Set cursor to underline mode
  Serial.print("?<"); //Exit big number mode
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
  */
  
  LCD_set_custom_characters();
  LCD_clear();
  
}

//Draws the headings on rows 1 and 3
void draw_primary_headings()
{
  
  //Start Drawing headings
  char headings1[20];
  //sprintf(headings1, "Dist1        %s", (SHOW_TIME_COUNT1?"  Time":"Avg.Sp"));
  if(SHOW_TIME_COUNT1) { strcpy(headings1, "Dist1          Time"); }
  else { strcpy(headings1, "Dist1        Avg.Sp"); }
  LCD_print_string_with_coords(headings1, 0, 0);
  
  //Headings on 3rd row
  char headings2[20];
  if(SHOW_TIME_COUNT2) { strcpy(headings2, "Dist2          Time"); }
  else { strcpy(headings2, "Dist2        Avg.Sp"); }
  LCD_print_string_with_coords(headings2, 0, 2);

}

//Draws the headings on rows 2 and 4
void draw_secondary_headings()
{
 
  char heading2[6];
  char heading4[6];
  
  sprintf(heading2, "%s %s", (USE_MILES_COUNT1?"Mi":"Km"), (DIR_FORWARD_COUNT1?LCD_ARROW_UP:LCD_ARROW_DOWN));
  sprintf(heading4, "%s %s", (USE_MILES_COUNT2?"Mi":"Km"), (DIR_FORWARD_COUNT2?LCD_ARROW_UP:LCD_ARROW_DOWN));
  
  LCD_print_string_with_coords(heading2, 7, 1);
  LCD_print_string_with_coords(heading4, 7, 3);

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
  floatToString(distance1_string, distance_1, 2);
  LCD_print_string_with_coords(distance1_string, 0, 1);
  
  //Set position and print distance 2  
  floatToString(distance2_string, distance_2, 2);
  LCD_print_string_with_coords(distance2_string, 0, 3);
  
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
    LCD_print_string_with_coords(cur_time1_str, 13, 1);
  }
  else
  {
    //We do average speed
    float hours1 = float(cur_time1) / float(MS_IN_HOUR);
    float avg_speed1 = float(distance_1) / float(hours1);
    
    char avg_speed1_string[7]; //String buffer for the result
    floatToString(avg_speed1_string, avg_speed1, 2);
    LCD_print_string_with_coords(avg_speed1_string, 14, 1);
  }
  
  if(SHOW_TIME_COUNT2)
  {
    //Do the same as above but for count2
    int hours2 = cur_time2 / MS_IN_HOUR;
    int mins2 = (cur_time2 % MS_IN_HOUR) / MS_IN_MIN;
    int secs2 = (cur_time2 % MS_IN_MIN) / 1000;
  
    char cur_time2_str[9];
  
    sprintf(cur_time2_str, "%1d:%02d:%02d", hours2, mins2, secs2);
    LCD_print_string_with_coords(cur_time2_str, 13, 3);
  }
  else
  {
    //We do average speed
    float hours2 = float(cur_time2) / float(MS_IN_HOUR);
    float avg_speed2 = float(distance_2) / float(hours2);
    
    char avg_speed2_string[7]; //String buffer for the result
    floatToString(avg_speed2_string, avg_speed2, 2);
    LCD_print_string_with_coords(avg_speed2_string, 14, 3);
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
  int new_pos = CURSOR_POS;
  switch (NUNCHUCK_X_AXIS) {
    case -1: //Left
      if(CURSOR_POS > 2) { new_pos = CURSOR_POS - 2; }
      break;
    case 1: //Right
      if(CURSOR_POS < 7) { new_pos = CURSOR_POS + 2; }
      break;
  } 
  set_cursor_pos(new_pos);
}

void update_nunchuck_yaxis()
{
  int new_pos = CURSOR_POS;
  switch (NUNCHUCK_Y_AXIS) {
    case -1: //Up
      if( (CURSOR_POS % 2) == 0 ) { new_pos = CURSOR_POS - 1; }
      break;
    case 1: //Down
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
        LCD_print_string_with_coords(" ", 13, 1);
        break;
      case 8:
        //Switch count2 between time and average speed
        SHOW_TIME_COUNT2 = !SHOW_TIME_COUNT2;
        draw_primary_headings(); //Redraw headings do to change
        set_cursor_pos(CURSOR_POS);
        //Need to clear the first character from avg speed/time 
        LCD_print_string_with_coords(" ", 13, 3);
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
  LCD_print_string_with_coords(" ", x, y);
  
  switch(new_pos)
  {
    case 1:
      LCD_print_string_with_coords(LCD_ARROW_RIGHT, 6, 1);
      break;
    case 2:
      LCD_print_string_with_coords(LCD_ARROW_RIGHT, 6, 3);
      break;
    case 3:
      LCD_print_string_with_coords(LCD_ARROW_DOWN, 7, 0);
      break;
    case 4:
      LCD_print_string_with_coords(LCD_ARROW_DOWN, 7, 2);
      break;
    case 5:
      LCD_print_string_with_coords(LCD_ARROW_DOWN, 10, 0);
      break;
    case 6:
      LCD_print_string_with_coords(LCD_ARROW_DOWN, 10, 2);
      break;
    case 7:
      LCD_print_string_with_coords(LCD_ARROW_LEFT, 12, 0);
      break;
    case 8:
      LCD_print_string_with_coords(LCD_ARROW_LEFT, 12, 2);
      break;
  }
  
  CURSOR_POS = new_pos;
}

void decode_nunchuck()
{
  int cnt = 0;
  uint8_t outbuf[6];

  Wire.requestFrom (0x52, 6);	// request data from nunchuck
  while (Wire.available ())
  {
    outbuf[cnt] = (Wire.receive()^0x17) + 0x17; //Receive and decode 1 byte //nunchuk_decode_byte (Wire.receive ());
    cnt++;
  }
  
  if (cnt < 5) { return; } //Return on fail (There must be at least 6 bytes returned from the nunchuck)
  
  boolean changed = false; //Represents whether or not anything has changed
  int joy_x_axis = outbuf[0];
  int joy_y_axis = outbuf[1];
  
  // X Axis changes
  if (int(joy_x_axis) < 40) 
  {
    changed = (NUNCHUCK_X_AXIS != -1);
    NUNCHUCK_X_AXIS = -1; //Left
  } 
  else {
  if (int(joy_x_axis) > 215) 
  {
    changed = (NUNCHUCK_X_AXIS != 1);
    NUNCHUCK_X_AXIS = 1;  //Right
  }
  else {NUNCHUCK_X_AXIS = 0; }//Center
  }
  if(changed) { update_nunchuck_xaxis(); }

  changed = false;
  //Y Axis Changes
  if (int(joy_y_axis) < 40) 
  {
    //Down
    changed = (NUNCHUCK_Y_AXIS != 1);
    NUNCHUCK_Y_AXIS = 1; 
  } 
  else {
  if (int(joy_y_axis) > 210) 
  {
     //Up
    changed = (NUNCHUCK_Y_AXIS != -1);
    NUNCHUCK_Y_AXIS = -1;
  }
  else {NUNCHUCK_Y_AXIS = 0; }//Center
  }
  if(changed) { update_nunchuck_yaxis(); }
  
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
  if(changed) { update_nunchuck_zbutton(); }
  
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
  Wire.send (0x00);		// sends one byte
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

