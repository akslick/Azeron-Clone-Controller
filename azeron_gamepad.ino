/*
   "Azeron clone" Game Pad controller
   A custom made keyboard/joystick controller using HID on Arduino Pro Micro
   https://www.inables.com/id/Azeron-Game-Pad-DIY-Under-35/
   JerkWagon & Anon Engineering @Discord 9/2020
   Rev. 091620vs
   Dramatically Modified by ak_slick
*/

#include <Keyboard.h>
#define KEY_BEGIN(); Keyboard.begin()
#define KEY_PRESS(x); Keyboard.press(x)
#define KEY_RELEASE(x); Keyboard.release(x)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define KEY_LEFT_SHIFT 129

//Globals
const int xCenter = 250;    // Tweak for your D-pad's center values, should be 250
const int yCenter = 250;
const int xDeadband = 150;  // Sets the size of the center deadband
const int yDeadband = 150;

// D-pad: UP = w, RIGHT = d, DOWN = s, LEFT = a, DPadNone is "center value"
const char dPadUp = 'w';
const char dPadRt = 'd';
const char dPadDn = 's';
const char dPadLt = 'a';
const char dPadNone = '0';

// track for joystick input
static char lastXKeyPressed = dPadNone;
static char lastYKeyPressed = dPadNone;

// create pwm inputs
unsigned long startMillis;
unsigned long currentMillis;
unsigned long desiredPulse;
const unsigned long period = 1000;  // 1 second
bool pulseActive = false;
bool restActive = false;


typedef struct {
  int pin;
  char cmd;
  bool wasPressed;
} dKey;

// Switch pins, {PIN, KEY, False}
dKey dSwitch[] = {
  {19, 'u', false}, // Pin 19 = A1
  {2, KEY_LEFT_SHIFT, false},
  {3, '1', false},
  {4, 'f', false},
  {5, KEY_LEFT_SHIFT, false},
  {6, '2', false},
  {7, 'e', false},
  {8, 'x', false},
  {9, KEY_TAB, false},
  {10, 't', false},
  {16, 'b', false},
  {14, 'r', false},
  {15, '3', false}
};

void setup() {
  Serial.begin(9600);
  KEY_BEGIN();
  // Pin Mode setup
  for (int i = 0; i <= 12; i++) {
    pinMode(dSwitch[i].pin, INPUT_PULLUP);
  }
  // Shows key mapping if in DEBUG
  #ifdef DEBUG
  while (!Serial);  // Wait for serial to come up
  DEBUG_PRINTLN("Key mapping: ");
  DEBUG_PRINT("Pin\t");
  DEBUG_PRINTLN("Key");
  for (int i = 0; i <= 12; i++) {
    DEBUG_PRINT(dSwitch[i].pin);
    DEBUG_PRINT('\t');
    DEBUG_PRINTLN(dSwitch[i].cmd);
  }
  DEBUG_PRINTLN();
  #endif
}

void loop() {
  readJoystick();
  readKeys();
}

void readJoystick() {
  float angle = 0.0;
  int joyX = analogRead(20);  // Or A3, rotates 0 angle (0 degrees is full right by default)
  int joyY = analogRead(21);  // Or A2, rotates 0 angle
  double mapJoyX = map(joyX, 0, 1023, 0, 500);
  double mapJoyY = map(joyY, 0, 1023, 0, 500);
  // For test, use to determine xCenter, yCenter
  //DEBUG_PRINT("Mapped X value: ");
  //DEBUG_PRINTLN(mapJoyX);
  //DEBUG_PRINT("Mapped Y value: ");
  //DEBUG_PRINTLN(mapJoyY);
  /*
    // Get joystick angle
    if X AND Y = center then angle = 1000     // In deadband
      release key(last key)
    else angle = arctangent y/x * 57.2957795  // Radians to degrees
    // Which quadrant?
    if angle <> 1000                          // If NOT in deadband
      if angle >= 45 and <= 135 then UP
      if angle < 45 and angle > -45 then RIGHT
      if angle <= -45 and angle >= -135 then DOWN
      if angle < -135 and angle >= -180 OR <= 180 and angle > 135 then LEFT 
  */
  // Determine if joystick is centered...
  if ((mapJoyX <= xCenter + xDeadband && mapJoyX >= xCenter - xDeadband) && (mapJoyY <= yCenter + yDeadband && mapJoyY >= yCenter - yDeadband))  {
    angle = 1000.0;
    if (lastXKeyPressed != dPadNone)  {
      KEY_RELEASE(lastXKeyPressed);
      lastXKeyPressed = dPadNone;
    }
    if (lastYKeyPressed != dPadNone)  {
      KEY_RELEASE(lastYKeyPressed);
      lastYKeyPressed = dPadNone;
    }
  }
  else  { // Else determine its angle
    angle = atan2(mapJoyY - yCenter , mapJoyX - xCenter) * 57.2957795;
  }

  if ((angle >= 78.75 && angle <= 101.25)) {  // UP
    if (lastYKeyPressed != dPadUp)  {
      KEY_RELEASE(lastYKeyPressed);
      lastYKeyPressed = dPadUp;
      KEY_PRESS(dPadUp);
    }
    if (lastXKeyPressed != dPadNone)  {
        KEY_RELEASE(lastXKeyPressed);
        lastXKeyPressed = dPadNone;
      }
  }
  else if ((angle < 78.75 && angle >= 56.25)) {  // UP and 50% Right
    if (!pulseActive && !restActive){
      startMillis = millis();
      pulseActive = true;

      if (lastXKeyPressed != dPadRt) {
        KEY_RELEASE(lastXKeyPressed);
        lastXKeyPressed = dPadRt;
      }
    }
    else if (pulseActive && (millis()-startMillis >= 500)) {
      pulseActive = false;
      restActive = true;
      startMillis = millis()
      KEY_RELEASE(lastXKeyPressed);
      lastXKeyPressed = dPadNone;
    }
    else if (restActive && (millis()-startMillis >= 500)){
      restActive = false;
    }
        
  }
  else if ((angle < 56.25 && angle >= 33.75)) {  // UP and Right
    if (lastYKeyPressed != dPadUp)  {
      KEY_RELEASE(lastYKeyPressed);
      lastYKeyPressed = dPadUp;
      KEY_PRESS(dPadUp);
    }
    if (lastXKeyPressed != dPadRt)  {
        KEY_RELEASE(lastXKeyPressed);
        lastXKeyPressed = dPadRt;
        KEY_PRESS(dPadRt);
      }
  }
  else if ((angle < 33.75 && angle >= 11.25)) {  // Right and 50% UP 
    //
  }
  else if (angle < 11.25 && angle > -11.25) {  // RIGHT
    if (lastXKeyPressed != dPadRt)  {
      KEY_RELEASE(lastXKeyPressed);
      lastXKeyPressed = dPadRt;
      KEY_PRESS(dPadRt);
    }
      if (lastYKeyPressed != dPadNone)  {
        KEY_RELEASE(lastYKeyPressed);
        lastYKeyPressed = dPadNone;
      }
  }
  else if (angle < -11.25 && angle >= -33.75) {  // RIGHT and 50% DOWN
    
  }
  else if (angle < -33.75 && angle > -56.25) {  // RIGHT and DOWN
    if (lastYKeyPressed != dPadDn)  {
      KEY_RELEASE(lastYKeyPressed);
      lastYKeyPressed = dPadDn;
      KEY_PRESS(dPadDn);
    }
    if (lastXKeyPressed != dPadRt)  {
        KEY_RELEASE(lastXKeyPressed);
        lastXKeyPressed = dPadRt;
        KEY_PRESS(dPadRt);
      }
  }
  else if (angle < -56.25 && angle >= -78.75) {  // DOWN and 50% RIGHT
    
  }
  else if (angle < -78.75 && angle >= -101.25) {  // DOWN
    if (lastYKeyPressed != dPadDn)  {
      KEY_RELEASE(lastYKeyPressed);
      lastYKeyPressed = dPadDn;
      KEY_PRESS(dPadDn);
    }
    if (lastXKeyPressed != dPadNone)  {
        KEY_RELEASE(lastXKeyPressed);
        lastXKeyPressed = dPadNone;
      }
  }
  else if (angle < -101.25 && angle >= -123.75) {  // DOWN and 50% LEFT
    
  }
  else if (angle < -123.75 && angle >= -146.25) {  // DOWN and LEFT
    if (lastYKeyPressed != dPadDn)  {
      KEY_RELEASE(lastYKeyPressed);
      lastYKeyPressed = dPadDn;
      KEY_PRESS(dPadDn);
    }
    if (lastXKeyPressed != dPadLt)  {
        KEY_RELEASE(lastXKeyPressed);
        lastXKeyPressed = dPadLt;
        KEY_PRESS(dPadLt);
      }
  }
  else if (angle < -146.25 && angle >= -168.75) {  // LEFT and 50% DOWN
    
  }
  else if ((angle < -168.75 && angle >= -180) || (angle <= 180 && angle > 168.75))  { // LEFT
    if (lastXKeyPressed != dPadLt)  {
      KEY_RELEASE(lastXKeyPressed);
      lastXKeyPressed = dPadLt;
      KEY_PRESS(dPadLt);
    }
    if (lastYKeyPressed != dPadNone)  {
        KEY_RELEASE(lastYKeyPressed);
        lastYKeyPressed = dPadNone;
      }
  }
  else if ((angle < 168.75 && angle >= 146.25)) { // LEFT and 50% UP
      
    }
  else if (angle < 146.25 && angle >= 123.75) {  // LEFT and UP
    if (lastYKeyPressed != dPadUp)  {
      KEY_RELEASE(lastYKeyPressed);
      lastYKeyPressed = dPadUp;
      KEY_PRESS(dPadUp);
    }

    if (lastXKeyPressed != dPadLt)  {
      KEY_RELEASE(lastXKeyPressed);
      lastXKeyPressed = dPadLt;
      KEY_PRESS(dPadLt);
    }

  }
  else if (angle < 123.75 && angle >= 101.25) {  // UP and 50% LEFT
    
  }
}

void readKeys() {
  for (int i = 0; i <= 12; i++)  {
    if (digitalRead(dSwitch[i].pin) == LOW) {
      if (dSwitch[i].wasPressed == false)  {
        KEY_PRESS(dSwitch[i].cmd)
        DEBUG_PRINT("Key press:\t");
        DEBUG_PRINT(dSwitch[i].pin);
        DEBUG_PRINT('\t');
        DEBUG_PRINTLN (dSwitch[i].cmd);
        dSwitch[i].wasPressed = true;
      }
    }
    else  {
      if (dSwitch[i].wasPressed == true)  {
        KEY_RELEASE(dSwitch[i].cmd);
        DEBUG_PRINT("Key released:\t");
        DEBUG_PRINT(dSwitch[i].pin);
        DEBUG_PRINT('\t');
        DEBUG_PRINTLN (dSwitch[i].cmd);
        dSwitch[i].wasPressed = false;
      }
    }
  }
}

void checkAndSet(char key, char lastKeyPressed) {
      if (lastKeyPressed != key)  {
        KEY_RELEASE(lastKeyPressed);
        lastKeyPressed = key;
        KEY_PRESS(dPadLt);
      }
}
