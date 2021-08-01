/******************************************************************************
*  Omnibot 5402, Omnibot MKII, and Robie Sr. Emulator
*
* Scott McDonnell
* 07-2021
* Version 0.99
*
* Basic Arduino Uno version only emulating the original functions.
* CPU is removed from the Omnibot and replaced with a wire adapter to the
* Arduino.
* The Omnibot is controlled by audio tones of the frequencies 1400 to 4600 HZ
* Each command is 200 HZ apart. The CPU translates the incoming tone to a
* pin assertion for a function.
* The TALK ON function will tatch until the TALK OFF ommand is received.
* The TAPE START/STOP function will latch until toggled
* All other commands are momentary
******************************************************************************/

#include <FreqCount.h>
//#include <Talkie.h>
//#include <AnyRtttl.h>

// Strings used during debugging. These are output to the serial port to report which remote commands were detected. Will be removed.
const char string_0[] PROGMEM = "INALID"; // "String 0" etc are strings to store - change to suit.
const char string_1[] PROGMEM = "TALK ON";
const char string_2[] PROGMEM = "FORWARD";
const char string_3[] PROGMEM = "RIGHT";
const char string_4[] PROGMEM = "REVERSE";
const char string_5[] PROGMEM = "LEFT";
const char string_6[] PROGMEM = "TAPE START/STOP";
const char string_7[] PROGMEM = "SOUND1";
const char string_8[] PROGMEM = "SOUND2";
const char string_9[] PROGMEM = "TALK OFF";
const char *const string_table[] PROGMEM = {string_0, string_1, string_2, string_3, string_4, string_5, string_6, string_7, string_8, string_9};
char buffer[30];

// Binary patterns for base motor control
// Motor outputs connected to A0-A3
#define MOTOR_PORT PORTD
#define FORWARD B01010000
#define REVERSE B10100000
#define RIGHT   B01100000
#define LEFT    B10010000
#define STOP    B11110000

// Remote Commands
const int TALKON = 1;  // 1400Hz
const int FWD = 2;     // 1600Hz
const int RGT = 3;     // 1800Hz
const int REV = 4;     // 2000Hz
const int LFT = 5;     // 2200Hz
const int TAPESS = 6;  // 2400Hz
const int SOUND1 = 7;  // 2600Hz
const int SOUND2 = 8;  // 2800Hz
const int TALKOFF = 17;  // 4600Hz

// Robot State
bool talkState = false;
bool tapeState = false;
bool moving = false;

// Pin definitions
const int pinTalk = 4;
const int pinSound1 = 6;
const int pinSound2 = 7;
const int pinTape = 8;
const int pinRadio = 5;

// TODO: Add code to play built in sounds or play programmed melodies.
void playSound(int Sound){

}

// TODO: Add code to control the eye LEDs
void lightShow(){
  
  
}

// TODO: Add Code for speech synthesis. Phrases stored in program memory
void Say(int PHRASE){

}

// Function to move the robot. This is very basic right now
// Motors are connected to A0-A3 on the Uno (P0 to A0, etc..)
// TODO: Add code to add motors ie. void Move(int motor, int dir, int speed, int dur)
// TODO: Add code for servo control
void Move(int dir){
  switch(dir){
    case FWD:
      PORTD |= FORWARD;
      break;
    case RGT:
      PORTD |= RIGHT;
      break;
    case REV:
      PORTD |= REVERSE;
      break;
    case LFT:
      PORTD |= LEFT;
      break;
    default:
      PORTD |= STOP;
      break;
  }
  
  delay(200); //keep motor moving long enough to continue between commands. This is probably too long.
  PORTD |= STOP;
  moving = false;

}

// Function to receive the radio input and decode the tones into commands.
// Filtering by taking 2 samples and rejecting the command if both samples do not match
// Looking for most efficient way to do this
int getCommand(){

  unsigned long freq = 0;
  unsigned long freq2 = 0;
  unsigned int cmd = 0;
  
  // Take 2 samples. If all samples are not equal, reject the sample.
  // 5ms delay to allow time for the measurement
  freq = FreqCount.read();
  delay(5);
  freq2 = FreqCount.read();
  if(freq2 != freq) freq = 0; 
  
  if((freq > 6) || (freq < 24)){
    freq = freq - 6; // Subtract 6 to make commands = 1 to 17
    cmd = int(freq);
  } else {
    cmd = 0; // Command is invalid
  }
  
  return cmd;

}

// Not Used currently
// Experiment to try to run in a loop constantly unless a valid command is received from the remote.
// When a valid command is received, it will break out, act on the command and then in the next (main)
// loop, it will wait for a valid command again, and so on.
// This code would be more accurate to the exct emulation of the robot, but will block out expandability
int getCommandLoop(){

  unsigned long freq = 0;
  unsigned long freq2 = 0;
  unsigned int cmd = 0;
  // Do..while will run at least once, so we have a chance to check for a valid remote
  // command. If one is not encountered, then this code will loop until it does.
  // When a valid command is encountered, the loop will break and return the command
  do {
  
	// Take 2 samples. If all samples are not equal, reject the sample.
	// 5ms delay to allow time for the measurement
	freq = FreqCount.read();
	delay(5);
	freq2 = FreqCount.read();
	if(freq2 != freq) freq = 0; 
	delay(5);
	  
	if((freq > 6) || (freq < 24)){
		freq = freq - 6; // Subtract 6 to make commands = 1 to 17
		cmd = int(freq);
	  } else {
		cmd = 0;
	}
  }  while (cmd  == 0);
  
  return cmd;

}


void setup(){

  Serial.begin(9600);
  FreqCount.begin(5000);
  
  DDRD |= B1111;
  MOTOR_PORT |= B1111; //Stop Motors
  
  // Set up data direction for the function pins
  pinMode(pinTalk, OUTPUT);
  pinMode(pinSound1, OUTPUT);
  pinMode(pinSound2, OUTPUT);
  pinMode(pinTape, OUTPUT);
  
  // Make sure tape and talk pins are not asserted
  digitalWrite(pinTalk, HIGH);
  digitalWrite(pinTape, HIGH);
  digitalWrite(pinSound2,HIGH);
  
  //Play Melody at power on or reset
  digitalWrite(pinSound1, LOW);
  delay(20);
  digitalWrite(pinSound1, HIGH);

}

// Main loop
void loop(){
  int cmd = 0;
  int dir = 0;
  bool moving = false;
  
  // We check to see if there is a remote command
  cmd = getCommand();
  if(cmd != 0){
    strcpy_P(buffer, (char *)pgm_read_word(&(string_table[cmd])));
    Serial.println(buffer);
  }
  
  // Act on remote command
  switch (cmd) {
    
    //Drive commands
    case FWD:
    case RGT:
    case REV:
    case LFT:
    // dir = cmd - 1; // convert the drive commands to 1 thru 4
    moving = true; // Update the robot state to moving
    Move(cmd); // Do the actual moving
    moving = false;
    
    break;
      
    case TALKON: // TALK ON. Momentary. Relay stays engaged until the talk button is released.
      if(talkState == false){
        digitalWrite(pinTalk, LOW);
        delay(10);
	talkState = true;
      }
    
      break;
      
    case TALKOFF: // TALK OFF. Talk button was released. Relay is turned off.
      if(talkState == true){
        digitalWrite(pinTalk, HIGH);
        delay(10);
	talkState = false;
      } 
      break;
    
    case TAPESS: // TAPE START/STOP. This output is latched. Pressing the button toggles the state
      if (tapeState == true){
        digitalWrite(pinTape, HIGH);
	tapeState = false;
      } else {
          digitalWrite(pinTalk,LOW);
	  talkState = true;
        }
      break;
    
    case SOUND1: // SOUND 1
      digitalWrite(pinSound1, LOW);
      delay(5);
      digitalWrite(pinSound1, HIGH);
      break;
    
    case SOUND2: // SOUND 2
      digitalWrite(pinSound2, LOW);
      delay(5);
      digitalWrite(pinSound2, HIGH);
      break;
    default:
      break;  
  }
}
