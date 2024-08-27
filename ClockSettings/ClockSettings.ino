// ------------------------------------------------------------------------------------------- //
// **************************     Clock Setting   Version 4.4  *********************** //
// ------------------------------------------------------------------------------------------- //
// RFID Sign
//
// uses MatsRobotFont8x8.h font and replace 1 to 9 with user defined 8x8 characters
// Numbers are decimal converted binary top down columns
// To amend .h file rename it as .ino and open it with Arduino and rename it back once changed
/*
RFID Sign
128, 192, 192, 192,   0,   0,  16,  72, //'1'
 36,  36, 146, 146, 146,  146, 146, 36, //'2'
100, 200, 144,  32, 192,    0,   0,  0, //'3'
511, 511,   0,   0,   0,    0,   0,  0, //'4'
  0,   1,   0,  24,  60,   24,   1, 67, //'5'
 62, 128, 127,  12, 255,   30,   0,  0, //'6'
 63, 127, 224, 192, 223, 201, 214, 192, //'7'
223, 197, 193, 192, 223, 192, 223, 209, //'8'
206, 192, 248, 121,   0,   0,   0,   0, //'9'

Battery
Full  "AAC"
OK    "ABC"
Low   "DBC"
Empty "EBC"

255, 255, 255, 255, 255, 255, 255, 255, //'A'
129, 129, 129, 129, 129, 129, 129, 129, //'B'
129, 129, 231,  36,  60,   0,   0,   0, //'C'
255, 255, 255, 129, 129, 129, 129, 129, //'D'
255, 129, 129, 129, 129, 129, 129, 129, //'E'

Degree sign
  0,   0,   0,   0,   0,   7,   5,   7, //'$'
  
 */
//
// ------------------------------------------------------------------------------------------- //
//                                         Include files                                       //
// ------------------------------------------------------------------------------------------- //
#include <SD.h>                // include the SD library code for the SD card Module
#include <Wire.h>              // include the I2C library for RTC device and any other
#include <RTClib.h>            // include the RTC library code this also includes RTClib.cpp
#include <SoftwareSerial.h>    // include the RFID Serial library code

#define I2C_ADDRESS 0x3C    // 0X3C+SA0 - 0x3C or 0x3D
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#include <MatsRobotFont8x8.h>

// ------------------------------------------------------------------------------------------- //
//                             Variables and Pin Definitions                                   //
// ------------------------------------------------------------------------------------------- //

#define BuzzerPin 9      // Buzzer Connected to Digital Pin 9

RTC_DS3231 RTC; // creates an RTC object in the code variables for reading the RTC time & handling the INT(0) interrupt it generates
#define DS3231_I2C_ADDRESS 0x68
int temp3231; //variables for reading the DS3231 RTC temperature register
byte tMSB = 0;
byte tLSB = 0;

SSD1306AsciiAvrI2c oled;  // Initialise OLED display

  char Line1[20];  //Display line 1
  char Line2[17];  //Display line 2
  char InfoLine[20];  //Display line 3
  char ErrorMessage[25];  // String to hold error messages  


// char arrays are used because the undefined variables were causing the program to crach
// as it was too long to fit in the memory, so instead all the variables are defined as
// finite size char arrays.  The complexity is to read and write into these arrays.
// and the last Character should be Null, so we need to define one character more than we need.

char b[5];   // 4 digits + \0 to hold the temporary date/time byte
char TimeDate_Tag[22]= "00/00/0000 - 00:00:00";   // array to hold the DateTime
char TimeDate_Enc[22];   // array to hold the Encrypted DateTime

char Time_Tag[6]= "00:00";   // array to hold the DateTime 
char Date_Tag[11]= "00/00/0000";   // array to hold the Encrypted DateTime


// Create Char array to hold RFID tag

int BlueLED=14; // Place an LED to use A0 on Pin 14
int BattVal=0; // Variable for Battery percent
char BattChar[5]="0000"; // Variable for Battery percent
char BattString[5];   // 4 digits + \0 to hold the temporary date/time byte


// ------------------------------------------------------------------------------------------- //
//                                       Error Trapping                                        //
// ------------------------------------------------------------------------------------------- //

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);

// Print Error message to the LCD.
   sprintf(Line1, "error: ");   
   sprintf(Line2, str);
   DisplayText(Line1, Line2);

 // On Error flash the LED and set Buzzer for 10 Beeps 
  for (int i = 0; i<5; i++) // Number of Flash and 1/2 number of Beeps
 {
 //    digitalWrite(BlueLED, HIGH);    // turn the Blue LED on

///*
// Using Buzzer
  digitalWrite(BuzzerPin, HIGH);
  delay(200); 
  digitalWrite(BuzzerPin, LOW);
  delay(200);
//*/
}
  while(1);                       // wait while there is an error
} // end of void error(char *str)



// ------------------------------------------------------------------------------------------- //
//                                       Display Procedure                                        //
// ------------------------------------------------------------------------------------------- //

void DisplayText(char *Line1, char *Line2)
{
//  oled.clear();
  
  oled.setFont(Verdana12_bold);  // Set font System5x7
  oled.setCursor(0, 0);  
  oled.println(F("Clock Setting   V4.4"));
  // Battery
  oled.setCursor(70, 5);  
  oled.setFont(MatsRobotFont8x8);  // Set font
  BattVal=int(analogRead(A1)); // value of A1 Full is about 680, empty less than 450
  if (BattVal>=700) {oled.print(F("AAC")); sprintf(BattString, "Full");}  // Full  "AAC"
  else if (BattVal>=580 && BattVal<700) {oled.print(F("ABC")); sprintf(BattString, "OK  ");}  // OK    "ABC"
  else if (BattVal<580) {oled.print(F("DBC")); sprintf(BattString, "Low ");}  // Low   "DBC"
  else {oled.print(F("EBC"));}  //  Empty "EBC"
  
  oled.setFont(System5x7);  // Set font
  sprintf(InfoLine,"V4 %2d'C Bat ",temp3231);
  strcat(InfoLine, BattString);
  oled.setCursor(0, 7);  
  oled.print(InfoLine);

// Logo
  oled.setFont(MatsRobotFont8x8);  // Set font
  oled.setCursor(100, 5);  
  oled.print(F("123"));
  oled.setCursor(100, 6);  
  oled.print(F("456")); 
  oled.setCursor(100, 7);  
  oled.print(F("789")); 

// 3 line text
  oled.setFont(Arial_bold_14);  // Set font
  oled.setCursor(0, 2);  
  oled.println(Line1);
  oled.setCursor(0, 4);  
  oled.print(Line2);

} // end of void DisplayText



// ------------------------------------------------------------------------------------------- //
//                                         Set up Procedures                                   //
// ------------------------------------------------------------------------------------------- //

void setup(void)
{

// Set the LED and Buzzer pins to output
  pinMode(BlueLED, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);


// *** Initialize and connect to RTC DS1307
// ****************************************
// Begin the I2C interface connection
  Wire.begin(); 
  if (!RTC.begin()) 
  {
    strcpy(ErrorMessage,"Clock (RTC) Failed");
    error(ErrorMessage);
      Serial.println("RTC fail");
  }
   
// ****** Set Clock time
// *********************
RTC.adjust(DateTime(__DATE__, __TIME__)); //******* uncomment this line to set time to system time 

/* to avoid time getting reset all the time remove this code from the final code
   RTC.adjust(DateTime("DEC 31 2011","12:59:45")); // Setting the time to a fixed value. 
   RTC.adjust(DateTime(12,8,25,13,00,00));     // DateTime (year, month, day, hour, min, sec);
*/


} // end of void setup(void)

// ------------------------------------------------------------------------------------------- //
//                                  Repeated (LOOP) Procedures                                 //
// ------------------------------------------------------------------------------------------- //

void loop(void)
{
  
  digitalWrite(BlueLED, HIGH);    // turn the Blue LED on
  delay(10);                     // wait a bit

  // ***************************   RTC and populate the time date variable ************************
// **********************************************************************************************

  //—–This part reads the time and disables the RTC alarm
    DateTime now = RTC.now(); //this reads the time from the RTC
    sprintf(TimeDate_Tag, "%02d/%02d/%04d% - 02d:%02d:%02d",  now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());

  delay(10);


 // read the RTC temperature register and print that out
    // Note: the DS3231 temp registers (11h-12h) are only updated every 64seconds
    Wire.beginTransmission(DS3231_I2C_ADDRESS);
    Wire.write(0x11);                     //the register where the temp data is stored
    Wire.endTransmission();
    Wire.requestFrom(DS3231_I2C_ADDRESS, 2);   //ask for two bytes of data
    if (Wire.available()) {
    tMSB = Wire.read();            //2’s complement int portion
    tLSB = Wire.read();             //fraction portion
    temp3231 = ((((short)tMSB << 8) | (short)tLSB) >> 6) /4.0;  // Allows for readings below freezing: thanks to Coding Badly
    temp3231 =temp3231 -2; // caliboration 2 degrees out 
   }
  else {
    temp3231 = 0;
    //if temp3231 contains zero, then you know you had a problem reading the data from the RTC!
  }

// Populate the Date_Tag value
  for (int i = 0; i<10; i++) //  Date is the first 10 digits of the TimeDate_Tag string, but we read 8 and replace the last two digit with the year
       {
        Date_Tag[i] = TimeDate_Tag[i];   // re-create variable with the new Charactor Date [0 to 7 plus Null] (0 to 7 of TimeDate_Tag plus null)
       }
//        Date_Tag[6] = TimeDate_Tag[8];  // Replace the year with just two digits year (this is currently 20)
//        Date_Tag[7] = TimeDate_Tag[9];  // Replace the year with just two digits year (this is currently 20)

       Date_Tag[10] = '\0';  // add Null


// Populate the Time_Tag value
  for (int i = 11; i<16; i++)           // ignore the seconds by reading to digit 16 of the TimeDate_Tag
       {
        Time_Tag[i-11] = TimeDate_Tag[i];   // re-create variable with the new Charactor Time [0 to 7 plus Null] (11 to 18 of TimeDate_Tag plus null)
       }
       Time_Tag[5] = '\0';  // add Null

   sprintf(BattChar, "%d",  BattVal);
   strcpy(Line2, "Date Set");
   strcpy(Line1, TimeDate_Tag);
   
DisplayText(Line1, Line2);



// Buzzer to confirm it has written to the SD card and process is complete
// Buzzer pizzo is on pin 9 also works with this procedure

  digitalWrite(BuzzerPin, HIGH);
  delay(1); 
  digitalWrite(BuzzerPin, LOW);
  delay(1); 
  
  digitalWrite(BlueLED, HIGH);       // turn the Blue LED on 
  delay(500); // Delay so it doesn't log twice if card is held for a long time
  digitalWrite(BlueLED, LOW);       // turn the Blue LED off 

 delay(1000); // Delay before empying the RFID buffer to give chanve for the card to be removed

 
} // end of void loop(void)
// ------------------------------------------------------------------------------ //


