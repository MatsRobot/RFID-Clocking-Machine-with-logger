// ------------------------------------------------------------------------------------------- //
// **************************     RFID Clocking Machine   Version 4.4  *********************** //
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

#define rxPin 2  // Transmission Received Used by RFID module
#define txPin 2 // This should be Pin 3, but Pin 3 is not used by RFID and is used by the LCD Enable
#define BuzzerPin 9      // Buzzer Connected to Digital Pin 9

RTC_DS3231 RTC; // creates an RTC object in the code variables for reading the RTC time & handling the INT(0) interrupt it generates
#define DS3231_I2C_ADDRESS 0x68
int temp3231; //variables for reading the DS3231 RTC temperature register
byte tMSB = 0;
byte tLSB = 0;

const int chipSelect = 10;  // for the data logging shield, we use digital pin 10 for the SD cs line

File logfile;      // the SD logging file
SSD1306AsciiAvrI2c oled;  // Initialise OLED display

SoftwareSerial RFID= SoftwareSerial(rxPin, txPin);   // create a Serial object RFID used by RFID Module

  char Line1[20];  //Display line 1
  char Line2[17];  //Display line 2
  char InfoLine[20];  //Display line 3
  char ErrorMessage[25];  // String to hold error messages  

int val = 0; // Used by RFID module
int bytesread = 0; // Used by RFID module

// char arrays are used because the undefined variables were causing the program to crach
// as it was too long to fit in the memory, so instead all the variables are defined as
// finite size char arrays.  The complexity is to read and write into these arrays.
// and the last Character should be Null, so we need to define one character more than we need.

char b[5];   // 4 digits + \0 to hold the temporary date/time byte
char TimeDate_Tag[20];   // array to hold the DateTime

char Time_Tag[10];   // array to hold the Time 
char Date_Tag[10];   // array to hold the Date

// Create Char array to hold RFID tag
char RFID_Tag[20]; // variable to hold RFID tag number
char DEC_Tag[20]; // variable to hold RFID 6 digit Hex value from RFID_Tag to convert to Decimal variable
long RFID_Dec_Tag; // variable to hold Decimal RFID Value

long Valid_Stamp; // variable to hold Valid Check

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
  oled.clear();
  
  oled.setFont(Verdana12_bold);  // Set font System5x7
  oled.setCursor(0, 0);  
  oled.println(F("RFID Datalogger V4.4"));

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
  
// Setting the SPI pins high helps some sd cards go into sleep mode 
// the following pullup resistors only need to be enabled for the stand alone logger builds - not the UNO loggers
   pinMode(chipSelect, OUTPUT); 
   digitalWrite(chipSelect, HIGH); //Always pullup the CS pin with the SD library

//and you may need to pullup MOSI/MISO
//pinMode(MOSIpin, OUTPUT); digitalWrite(MOSIpin, HIGH); //pullup the MOSI pin
//pinMode(MISOpin, INPUT); digitalWrite(MISOpin, HIGH);  //pullup the MISO pin
    delay(1);

// Set up Serial Monitor Bit rate and Print Serial Ready
  Serial.begin(9600);
  delay(100);                        
  Serial.flush();
  Serial.println("Serial Ready");  // Write with the carrage return and line feed

// Set the RFID serial Bit rate and set RX pin to read data
  RFID.begin(9600);
  pinMode(rxPin, INPUT);     // Set pin as Input to read from RFID
//  pinMode(txPin, OUTPUT);  // This pin is used as we don't transmit to the RFID card 
  pinMode(13, OUTPUT);     //the LED on board to Output even if not used.

// *** Initialize the SD card 
// **************************
  Serial.print("Initializing SD card...");   // Write to Serial Monitor

// make sure that the default chip select pin is set to output even if NOT Used
  pinMode(10, OUTPUT); // SD Chip Select pin
  
// see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
  {
   strcpy(ErrorMessage,"Card Failed");
   error(ErrorMessage);
  }
  
  Serial.println("card initialized.");

// Write to the same file (8 Char file name)
   char filename[] = "eSign.CSV";
       logfile = SD.open(filename, FILE_WRITE); 
   if (! logfile) 
   {
    strcpy(ErrorMessage,"Could not create file");
    error(ErrorMessage);
   }
   
 // Display the file name on Serial Monitor
  Serial.print("Logging to: ");
  Serial.println(filename);

// *** Initialize and connect to RTC DS1307
// ****************************************
// Begin the I2C interface connection
  Wire.begin(); 
  if (!RTC.begin()) 
  {
    logfile.println("RTC fail");
    strcpy(ErrorMessage,"Clock (RTC) Failed");
    error(ErrorMessage);
      Serial.println("RTC fail");
  }
   
// ****** Set Clock time
// *********************
//RTC.adjust(DateTime(__DATE__, __TIME__)); //******* uncomment this line to set time to system time 

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
    delay(10);

String(now.day()).toCharArray(b,3); // Convert the day number to a string and then add it to the temp char array b (2 digits + \0 char)
  if (now.day()<10)                 // if the number is less than 10 i.e. one digit, add a '0' in front of it
                                    // TimeDate_Tag[DD/MM/YYYY HH:MM:SS\0] 
    {
      TimeDate_Tag[0] = '0';        // [0]=D 1st digit as 0 if the number is only one digit i.e. 0 to 9
      TimeDate_Tag[1] = b[0];       // [1]=D 2nd digit as the number
    }
  else
    {
      TimeDate_Tag[0] = b[0];       // [0]=D 1st digit number
      TimeDate_Tag[1] = b[1];       // [1]=D 2nd digit number
    }
  TimeDate_Tag[2] = '/';            // [2]=/ Put a '/' date separator


// Repeat the day procedure for month
String(now.month()).toCharArray(b,3);  // Convert the Month number to a string and then add it to the temp char array b (2 digits + \0 char)
  if (now.month()<10) 
    {
      TimeDate_Tag[3] = '0';        // [3]=M 0 if one digit 
      TimeDate_Tag[4] = b[0];       // [4]=M 
    }
  else
    {
      TimeDate_Tag[3] = b[0];       // [3]=M 
      TimeDate_Tag[4] = b[1];       // [4]=M 
    }
  TimeDate_Tag[5] = '/';           // [5]=/ Put a '/' date separator


// Repeat the day procedure for year
String(now.year()).toCharArray(b,5); // Convert the Year number to a string and then add it to the temp char array b (4 digits + \0 char)
    TimeDate_Tag[6] = b[0];        // [6]=Y
    TimeDate_Tag[7] = b[1];        // [7]=Y
    TimeDate_Tag[8] = b[2];        // [8]=Y
    TimeDate_Tag[9] = b[3];        // [9]=Y
    TimeDate_Tag[10] = ' ';        // [10]=' ' Put a ' ' to separator date and time


// Repeat the day procedure for hours
String(now.hour()).toCharArray(b,3);  // Convert the Hour number to a string and then add it to the temp char array b (2 digits + \0 char)
  if (now.hour()<10) 
    {
      TimeDate_Tag[11] = '0';        // [11]=H 0 if one digit
      TimeDate_Tag[12] = b[0];       // [12]=H
    }
  else
    {
      TimeDate_Tag[11] = b[0];       // [11]=H
      TimeDate_Tag[12] = b[1];       // [12]=H
    }
  TimeDate_Tag[13] = ':';            // [13]=/ Put a ':' Time separator


// Repeat the day procedure for minutes
String(now.minute()).toCharArray(b,3);   // Convert the Minute number to a string and then add it to the temp char array b (2 digits + \0 char)
  if (now.minute()<10) 
    {
      TimeDate_Tag[14] = '0';       // [14]=M 0 if one digit
      TimeDate_Tag[15] = b[0];      // [15]=M
    }
  else
    {
      TimeDate_Tag[14] = b[0];      // [14]=M
      TimeDate_Tag[15] = b[1];      // [15]=M
    }
  TimeDate_Tag[16] = ':';            // [16]=/ Put a ':' Time separator


// Repeat the day procedure for seconds
String(now.second()).toCharArray(b,3);   // Convert the Seconds number to a string and then add it to the temp char array b (2 digits + \0 char)
  if (now.second()<10) 
    {
      TimeDate_Tag[17] = '0';       // [17]=S 0 if one digit
      TimeDate_Tag[18] = b[0];      // [18]=S
    }
  else
    {
      TimeDate_Tag[17] = b[0];     // [17]=S
      TimeDate_Tag[18] = b[1];     // [18]=S
    }
  TimeDate_Tag[19] = '\0';         // [19]=Null add the NULL

// Populate the Date_Tag value
  for (int i = 0; i<8; i++) //  Date is the first 10 digits of the TimeDate_Tag string, but we read 8 and replace the last two digit with the year
       {
        Date_Tag[i] = TimeDate_Tag[i];   // re-create variable with the new Charactor Date [0 to 7 plus Null] (0 to 7 of TimeDate_Tag plus null)
       }
        Date_Tag[6] = TimeDate_Tag[8];  // Replace the year with just two digits year (this is currently 20)
        Date_Tag[7] = TimeDate_Tag[9];  // Replace the year with just two digits year (this is currently 20)

       Date_Tag[8] = '\0';  // add Null


// Populate the Date_Tag value
  for (int i = 11; i<19; i++)           // ignore the seconds by reading to digit 16 of the TimeDate_Tag
       {
        Time_Tag[i-11] = TimeDate_Tag[i];   // re-create variable with the new Charactor Time [0 to 7 plus Null] (11 to 18 of TimeDate_Tag plus null)
       }
       Time_Tag[8] = '\0';  // add Null


// Create a Check sum to ensure that data is not tampered with
// The valid string is: int(RFID Decimal value /(Seconds +20))+ int((Day+Month+Year+Hour+min)/2)
   Valid_Stamp = RFID_Dec_Tag/(now.second()+20)+ int(now.day()+ now.month()+ now.year()+ now.hour()+ now.minute())/2;


   if (now.year()<2017) 
   {
    strcpy(ErrorMessage,"Clock Battery");
    error(ErrorMessage);
    delay(3000);
   }

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


   sprintf(BattChar, "%d",  BattVal);
   sprintf(Line1, "Hold till Complete"); 
   strcpy(Line2, Date_Tag);
   
DisplayText(Line1, Line2);


// ***********************            RDM630 RFID reader         ********************************
// **********************************************************************************************

  val = 0;                 // Digit Value read from RFID
  bytesread = 0;           // Count the Digits read

   while(RFID.read() !=2); // Wait for start byte from RFID reader when tag is swiped
      { delay(10); }

   while(bytesread < 12)   // Read 12 digit RFID code
      {
       val = RFID.read(); // read 12 digit code
       delay (10);
       if(val == 3)
          { // if header or stop bytes before the 10 digit reading
            break; // stop reading
          }
       if(val != 2)
          {
           RFID_Tag[bytesread] = val;    // add the digit
           bytesread++;                  // ready to read next digit
           RFID_Tag[bytesread] = '\0';   // add the NULL
          }
      }
 

// Chop the 6 digit Hex number 0000xxxxxx00 out of the 12 digit RFID
// The Decimal version of the 6 digit is Written on the card

     for(int i=4; i<10 ; i++)
       {
        if (RFID_Tag[i] <16);
        DEC_Tag[i-4]=RFID_Tag[i];            // Put 6 digit to Char DEC_Tag
       }
 
 // Hold the Decimal number in a separate Strings
  RFID_Dec_Tag = strtoul(DEC_Tag, 0, 16);  // convert 6 digit Hex to Decimal and put in String Populate 
 
  
 // Flash LED to confirm It has read the card
    for (int i = 0; i<3; i++)          // Number of flashes
    {
     digitalWrite(BlueLED, HIGH);       // turn the Blue LED ON 
     delay(50);                  
     digitalWrite(BlueLED, LOW);        // turn the Blue LED OFF 
     delay(50);                  
    }

// Print a message to the LCD.

   sprintf(Line1, "Card Detected");   
   sprintf(Line2, "Reading...");
   DisplayText(Line1, Line2);


// Create a Check sum to ensure that data is not tampered with
// The valid string is: int(RFID Decimal value /(Seconds +20))+ int((Day+Month+Year+Hour+min)/2)
   Valid_Stamp = RFID_Dec_Tag/(now.second()+20)+ int(now.day()+ now.month()+ now.year()+ now.hour()+ now.minute())/2;


// Write Date, Time, RFID Hex Tag, RFID Decimal value, BAttery Level, Temperature, Valid Stamp and to the SD card
// CVS file Separated by comma


  logfile.print(Date_Tag);   
  logfile.print(',');      
  logfile.print(Time_Tag);   
  logfile.print(',');   
  logfile.print(RFID_Tag);
  logfile.print(',');
  logfile.print(RFID_Dec_Tag);
  logfile.print(',');
  logfile.print(BattVal);  
  logfile.print(',');
  logfile.print(temp3231);
  logfile.print(',');      
  logfile.print(Valid_Stamp);

       
  
  logfile.println(); // CVS file Next line
    

// empty the SD Write buffer by reading data      
  logfile.flush();

  delay(500); // Delay so it doesn't log twice if card is held for a long time
 

   strcpy(Line1,Time_Tag); 
   strcat(Line1," "); 
   strcat(Line1,Date_Tag); 
   sprintf(Line2, "Complete.");

   DisplayText(Line1, Line2);

   delay(500); // wait 1 second

// Buzzer to confirm it has written to the SD card and process is complete
// Buzzer pizzo is on pin 9 also works with this procedure

 ///*

    digitalWrite(BuzzerPin, HIGH);
    delay(50); 
    digitalWrite(BuzzerPin, LOW);
    delay(50); 

//*/  
  
  digitalWrite(BlueLED, HIGH);       // turn the Blue LED on 
  delay(500); // Delay so it doesn't log twice if card is held for a long time
  digitalWrite(BlueLED, LOW);       // turn the Blue LED off 

 // Empty the buffer Rem to see if this is needed as it hold the procedure !!!!!!!!!!!!!!!!!!!!!!!!!!
///*
// wait for a start byte from the RFID halts the procedure
//   while(RFID.read() !=2);                 // wait for start byte from rfid reader when tag is swiped
//      { delay(100); }

 delay(3000); // Delay before empying the RFID buffer to give chanve for the card to be removed

   for (int i = 0; i<100; i++)             //Read the RFID buffer 100 times to empty any repeat
     {
     RFID.read();
     delay(2); 
 }
// */

  delay(3000); // Delay before returning back to the beginning of the procedure

} // end of void loop(void)
// ------------------------------------------------------------------------------ //


