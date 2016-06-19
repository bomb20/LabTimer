//LabTimer - A Arduino based process-timer device to easily manage development ttimes in the analog photographic positive development process.
//Author: Vincent Truchseß
//E-Mail: 42@arkme.de
//
//license: This Project is published under the Apache License 2.0
//
//The goal is to build a timer device that runs two timers parallel. One timer is ment for the development process and counts down the last ten seconds.
//The other one is for the fixing process and only gives you an small beep when fixing time is over.
//
//This Code is ment to be user with a I2C 16x2 lcd and a 3x3 button matrix-keypad.
//

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
//configuring the pinout of the I2C-extender to the pins of the lcd
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
#include <Keypad.h>

//configuring the keypad
//just had an 4x4 keypad for prototypng at hand, gonna change this later
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
//the buttons 1,2,3,4,5,6,7 are without function
// U - Setting the Development timer + 15 Sec.
// D - Setting the Development timer - 15 Sec.
// S - Start/stop the Development timer
// R - Reset the Development timer
// same chars in lowercase - do the same for the fixer timer
// l - turn the Backlight on an off
char keys[ROWS][COLS] = {
  {'U','S','u','s'},
  {'D','R','d','r'},
  {'l','1','2','3'},
  {'4','5','6','7'}
};
byte rowPins[ROWS] = {28, 26, 24, 22}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {36, 34, 32, 30}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
//times are stored in seconds
//The dev timer
int timerA = 0;
//the fix timer
int timerB = 0;
const String zero = "00:00";
int timerStartA = 0;
int timerStartB = 0;
bool timerRunsA = false;
bool timerRunsB = false;
long stime;
bool backlight = true;
//pin used for the piezzo-speaker
byte speakerpin = 12;

void setup() {
  //Initializing stuff on the display
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Developer:");
  lcd.setCursor(0,1);
  lcd.print("Fixer:");
  for (int i = 0; i <= 2; i++){
    lcd.setCursor(11,i);
    lcd.print(zero);
  }
  pinMode(speakerpin, OUTPUT);
}

//this method udates the times on the Display accoding to the actual values
void updateTimerDisplay(){
  int timer;
  for (byte row = 0; row <= 1; row++){
    if (row ==  0) timer = timerA;
    else timer = timerB;
    int minutes = timer / 60;
    int seconds = timer % 60;
    lcd.setCursor(11,row);
    if (minutes < 10){
      lcd.print('0');
      lcd.setCursor(12,row);
    }
    lcd.print(minutes);
    lcd.setCursor(14,row);
    if (seconds < 10){
      lcd.print('0');
      lcd.setCursor(15,row);
    }
    lcd.print(seconds);
   }
}

void toggleBacklight(){
  if (backlight == true){ 
    lcd.noBacklight();
    backlight = false;
  }
  else{ 
    lcd.backlight();
    backlight = true;
  }
}

void timerReset(char timer){
  if (timer == 'a'){
    timerRunsA = false;
    timerA = timerStartA;
  }
  else if (timer == 'b'){
    timerRunsB = false;
    timerB = timerStartB;
  }
}



void loop() {
  //on every iteration of the main loop, check if a button was pressed and perform certain actions
  char key = keypad.getKey();
  if (key != NO_KEY){
    if (timerRunsA == false){
      //setting up the dev timer
      switch (key){
        case 'U':
        timerStartA = (timerStartA + 15) % 6000; //%6000, so there is a maximum of 99:59 to count down.
        timerA = timerStartA;
        break;
        case 'D':
        if (timerStartA > 15) timerStartA -= 15;
        else timerStartA = 0;
        timerA = timerStartA;
        break;
        default:
        break;
      }
    }
    if (timerRunsB == false){
      //setting up the fix timer
      switch (key){
        case 'u':
        timerStartB = (timerStartB + 15) % 6000;
        timerB = timerStartB;
        break;
        case 'd':
        if (timerStartB > 15) timerStartB -= 15;
        else timerStartB = 0;
        timerB = timerStartB;
        break;
        default:
        break;
      }
    }
    switch (key){
      //buttons, that work at any time:
      //Start/stop/reset a timer; toggeling backlight
      case 'S':
      if (timerRunsA == true) timerRunsA = false;
      else timerRunsA = true;
      break;
      case 'R':
      timerReset('a');
      break;
      case 's':
      if (timerRunsB == true) timerRunsB = false;
      else timerRunsB = true;
      break;
      case 'r':
      timerReset('b');
      break;
      case 'l':
      toggleBacklight();
      break;
      default:
      break;
    }
  }
//Do the actual countdown timer

//Still need to implement the sounds here
if ((millis() - stime) / 1000 >= 1){
  stime = millis();
  if (timerRunsA == true){ 
    timerA--;
    if (timerA <= 10){
      if (timerA == 0){ 
        tone(13,440,500);
      }
      else if (timerA <=3){ 
        tone(13,440, 200);
      }
      else{
        tone(13,330, 200);
      }
    }
  }
  if (timerRunsB == true) timerB--;
}

if (timerA <= 0) timerReset('a');
if (timerB <= 0) timerReset('b');

updateTimerDisplay();
  
 
  delay(10);
}

