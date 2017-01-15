/* YourDuinoStarter Example: nRF24L01 Radio remote control of servos by joystick
  - WHAT IT DOES
   Joystick on other Arduino communicates by nRF25L01 Radio to
   this Arduino with 2 pan-tilt servos
   SEE:

   The variable 'hasHardware'. You can test without servos and later set hasHardware = true;
        You NEED separate Servo power, not USB. YourDuino RoboRED has built in 2A power for servos
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
// NEED the SoftwareServo library installed
// http://playground.arduino.cc/uploads/ComponentLib/SoftwareServo.zip
// #include <SoftwareServo.h>  // Regular Servo library creates timer conflict!
/*-----( Declare Constants and Pin Numbers )-----*/
#define  CE_PIN  7   // The pins to be used for CE and SN
#define  CSN_PIN 8

#define PWM_MIN 850
#define PWM_MAX 2150
//
//#define ServoHorizontalPIN 3   //Pin Numbers for servos and laser/LED
//#define ServoVerticalPIN   5
//#define LaserPIN           6
//
//#define ServoMIN_H  30  // Don't go to very end of servo travel
//#define ServoMAX_H  150 // which may not be all the way from 0 to 180.
//#define ServoMIN_V  30  // Don't go to very end of servo travel
//#define ServoMAX_V  150 // which may not be all the way from 0 to 180


//////////////////////CONFIGURATION///////////////////////////////
#define CHANNEL_NUMBER 12  //set the number of chanels
#define CHANNEL_DEFAULT_VALUE 1500  //set the default servo value
#define FRAME_LENGTH 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PULSE_LENGTH 300  //set the pulse length
#define onState 1  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin 9  //set PPM signal output pin on the arduino

/*this array holds the servo values for the ppm signal
 change theese values in your code (usually servo values move between 1000 and 2000)*/
int ppm[CHANNEL_NUMBER];

/*-----( Declare objects )-----*/
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus (usually) pins 7 & 8 (Can be changed) */
RF24 radio(CE_PIN, CSN_PIN);

//SoftwareServo HorizontalServo;
//SoftwareServo VerticalServo;  // create servo objects to control servos

/*-----( Declare Variables )-----*/
byte addresses[][6] = {"1Node", "2Node"}; // These will be the names of the "Pipes"

// Allows testing of radios and code without servo hardware. Set 'true' when servos connected
//boolean hasHardware = false;  // Allows testing of radios and code without Joystick hardware.
boolean hasHardware = true;
int armed = 1000;

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
  Serial.begin(115200);   // MUST reset the Serial Monitor to 115200 (lower right of window )
  // NOTE: The "F" in the print statements means "unchangable data; save in Flash Memory to conserve SRAM"
  Serial.println(F("YourDuino.com Example: Receive joystick data by nRF24L01 radio from another Arduino"));
  Serial.println(F("and control servos if attached (Check 'hasHardware' variable"));
  printf_begin(); // Needed for "printDetails" Takes up some memory
  //  /*-----( Set up servos )-----*/
  //  if (hasHardware)
  //  {
  //    HorizontalServo.attach(ServoHorizontalPIN);  // attaches the servo to the servo object
  //    VerticalServo.attach(ServoVerticalPIN);
  //  }

  radio.begin();          // Initialize the nRF24L01 Radio
  radio.setChannel(108);  // 2.508 Ghz - Above most Wifi Channels
  radio.setDataRate(RF24_250KBPS); // Fast enough.. Better range

  // Set the Power Amplifier Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  // PALevelcan be one of four levels: RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
  radio.setPALevel(RF24_PA_MAX);

  //   radio.setPALevel(RF24_PA_MAX);

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);

  // Start the radio listening for data
  radio.startListening();

  //  radio.printDetails(); //Uncomment to show LOTS of debugging information

  //initiallize default ppm values
  for(int i=0; i<CHANNEL_NUMBER; i++){
      ppm[i]= CHANNEL_DEFAULT_VALUE;
    }//--(end setup )---

  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, !onState);  //set the PPM signal pin to the default state (off)
  
  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;
  
  OCR1A = 100;  // compare match register, change this
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();
}

void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{

  if ( radio.available())
  {

    while (radio.available())   // While there is data ready to be retrieved from the receive pipe
    {
      radio.read( &myData, sizeof(myData) );             // Get the data
    }

    radio.stopListening();                               // First, stop listening so we can transmit
    radio.write( &myData, sizeof(myData) );              // Send the received data back.
    radio.startListening();                              // Now, resume listening so we catch the next packets.

    Serial.print(F("Packet Received - Sent response "));  // Print the received packet data
    Serial.print(myData._micros);
    Serial.print(F("uS XL= "));
    Serial.print(myData.XLposition);
    Serial.print(F(" YL= "));
    Serial.print(myData.YLposition);
    Serial.print(F("uS XR= "));
    Serial.print(myData.XRposition);
    Serial.print(F(" YR= "));
    Serial.print(myData.YRposition);
    if ( myData.switchOn == 1)
    {
      Serial.println(F(" Switch ON"));
    }
    else
    {
      Serial.println(F(" Switch OFF"));
    }

  } // END radio available

  if (hasHardware)
  {
    int throttle = map(myData.XLposition, 1023, 0, PWM_MIN, PWM_MAX);
    int yaw = map(myData.YLposition, 0, 1023, PWM_MIN, PWM_MAX);
    int roll = map(myData.XRposition, 1023, 0, PWM_MIN, PWM_MAX);
    int pitch = map(myData.YRposition, 0, 1023, PWM_MIN, PWM_MAX);

    // TRIMS
    throttle += 0;
    yaw += 12;
    roll -= 29;
    pitch += 48;
    
    int arm = map(myData.switchOn, 0, 1, 1000, 2000);
    if (arm != armed) {
      armed = arm;
    }
    
    Serial.print("Yaw");
    Serial.print(yaw);
    Serial.print(" Throttle");
    Serial.print(throttle);
    Serial.print(" Roll");
    Serial.print(roll);
    Serial.print(" Pitch");
    Serial.print(pitch);
    Serial.print(" Arm");
    Serial.println(arm);

    ppm[0] = throttle;
    ppm[1] = roll;
    ppm[2] = pitch;
    ppm[3] = yaw;
    ppm[4] = armed;
    
  }
}

ISR(TIMER1_COMPA_vect){  //leave this alone
  static boolean state = true;
  
  TCNT1 = 0;
  
  if (state) {  //start pulse
    digitalWrite(sigPin, onState);
    OCR1A = PULSE_LENGTH * 2;
    state = false;
  } else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;
  
    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= CHANNEL_NUMBER){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PULSE_LENGTH;// 
      OCR1A = (FRAME_LENGTH - calc_rest) * 2;
      calc_rest = 0;
    }
    else{
      OCR1A = (ppm[cur_chan_numb] - PULSE_LENGTH) * 2;
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
}
