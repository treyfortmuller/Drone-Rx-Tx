/* YourDuinoStarter Example: nRF24L01 Radio Remote control: Joystick to Servos
  - WHAT IT DOES
   Joystick on this Arduino communicates by nRF25L01 Radio to
   a second Arduino with an nRF24L01 radio and 2 pan-tilt servos
   SEE: The variable 'hasHardware'. You can test without Joystick and later set hasHardware = true;

  - SEE the comments after "//" on each line below
  - CONNECTIONS:
   - nRF24L01 Radio Module: See http://arduino-info.wikispaces.com/Nrf24L01-2.4GHz-HowTo
   1 - GND
   2 - VCC 3.3V !!! NOT 5V
   3 - CE to Arduino pin 7
   4 - CSN to Arduino pin 8
   5 - SCK to Arduino pin 13
   6 - MOSI to Arduino pin 11
   7 - MISO to Arduino pin 12
   8 - UNUSED
  - V2.12 02/08/2016
   - Uses the RF24 Library by TMRH20 and Maniacbug: https://github.com/TMRh20/RF24 (Download ZIP)
   Questions: terry@yourduino.com */

/*-----( Import needed libraries )-----*/
#include <SPI.h>   // Comes with Arduino IDE
#include "RF24.h"  // Download and Install (See above)
#include "printf.h" // Needed for "printDetails" Takes up some memory
/*-----( Declare Constants and Pin Numbers )-----*/
#define  CE_PIN  7   // The pins to be used for CE and SN
#define  CSN_PIN 8

#define JOYSTICK_XL   A1  // The Joystick potentiometers connected to Arduino Analog inputs
#define JOYSTICK_YL   A0
#define JOYSTICK_SW  A2  // The Joystick push-down switch, will be used as a Digital input

#define JOYSTICK_XR  A4
#define JOYSTICK_YR  A5

/*-----( Declare objects )-----*/
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus (usually) pins 7 & 8 (Can be changed) */
RF24 radio(CE_PIN, CSN_PIN);

/*-----( Declare Variables )-----*/
byte addresses[][6] = {"1Node", "2Node"}; // These will be the names of the "Pipes"

unsigned long timeNow;  // Used to grab the current time, calculate delays
unsigned long started_waiting_at;
boolean timeout;       // Timeout? True or False

// Allows testing of radios and code without Joystick hardware. Set 'true' when joystick connected
//boolean hasHardware = false;
boolean hasHardware = true;

/**
  Create a data structure for transmitting and receiving data
  This allows many variables to be easily sent and received in a single transmission
  See http://www.cplusplus.com/doc/tutorial/structures/
*/
struct dataStruct {
  unsigned long _micros;  // to save response times
  int XLposition;          // The Joystick position values
  int YLposition;

  int XRposition;
  int YRposition;
  
  bool switchOn;          // The Joystick push-down switch
} myData;                 // This can be accessed in the form:  myData.Xposition  etc.


void setup()   /****** SETUP: RUNS ONCE ******/
{
  Serial.begin(115200);  // MUST reset the Serial Monitor to 115200 (lower right of window )
  // NOTE: The "F" in the print statements means "unchangable data; save in Flash Memory to conserve SRAM"
  Serial.println(F("YourDuino.com Example: Send joystick data by nRF24L01 radio to another Arduino"));
  printf_begin(); // Needed for "printDetails" Takes up some memory
  pinMode(JOYSTICK_SW, INPUT_PULLUP);  // Pin A2 will be used as a digital input

  radio.begin();          // Initialize the nRF24L01 Radio
  radio.setChannel(108);  // Above most WiFi frequencies
  radio.setDataRate(RF24_2MBPS); // Fast enough.. Better range

  // Set the Power Amplifier Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  // PALevelcan be one of four levels: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  radio.setPALevel(RF24_PA_MAX);
    //radio.setPALevel(RF24_PA_MAX);

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);

  // Start the radio listening for data
  radio.startListening();

//  radio.printDetails(); //Uncomment to show LOTS of debugging information
}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  radio.stopListening();                                    // First, stop listening so we can talk.

  if (hasHardware)  // Set in variables at top
  {
    /*********************( Read the Joystick positions )*************************/
    myData.XLposition = analogRead(JOYSTICK_XL);
    myData.YLposition = analogRead(JOYSTICK_YL);
    myData.XRposition = analogRead(JOYSTICK_XR);
    myData.YRposition = analogRead(JOYSTICK_YR);
    myData.switchOn  = !digitalRead(JOYSTICK_SW);  // Invert the pulldown switch
  }
  else
  {
    myData.XLposition = 514;  // Send some known fake data
    myData.YLposition = 515;
    myData.XRposition = 514;  // Send some known fake data
    myData.YRposition = 515;
  }

  myData._micros = micros();  // Send back for timing


  Serial.print(F("Now sending  -  "));

  if (!radio.write( &myData, sizeof(myData) )) {            // Send data, checking for error ("!" means NOT)
    Serial.println(F("Transmit failed "));
  }

  radio.startListening();                                    // Now, continue listening

  started_waiting_at = micros();               // timeout period, get the current microseconds
  timeout = false;                            //  variable to indicate if a response was received or not



  while ( ! radio.available() ) {                            // While nothing is received
    if (micros() - started_waiting_at > 200000 ) {           // If waited longer than 200ms, indicate timeout and exit while loop
      timeout = true;
      break;
    }
  }

  if ( timeout )
  { // Describe the results
    Serial.println(F("Response timed out -  no Acknowledge."));
  }
  else
  {
    // Grab the response, compare, and send to Serial Monitor
    radio.read( &myData, sizeof(myData) );
    timeNow = micros();

    // Show it
    Serial.print(F("Sent "));
    Serial.print(timeNow);
    Serial.print(F(", Got response "));
    Serial.print(myData._micros);
    Serial.print(F(", Round-trip delay "));
    Serial.print(timeNow - myData._micros);
    Serial.println(F(" microseconds "));

  }

  // Send again after delay. When working OK, change to something like 100
  delay(100);

}//--(end main loop )---

/*-----( Declare User-written Functions )-----*/

// NONE YET
//*********( THE END )***********
