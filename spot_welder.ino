// Ref: https://www.makerguides.com/tm1637-arduino-tutorial/ 

//SCL - A5 and SDA - A4
// promini layout at : https://2.bp.blogspot.com/-IhJMnyz0MB4/Vwsy92E4BLI/AAAAAAAAL9o/hI5jSk5_Z1sNOIOMQigtI7Ccv5G4ClHZw/s1600/Arduino%2Bmiini%2BATmega328-pinout%2B-%2BCopy%2B-%2BCopy.png
// external interrupt pins : 2, 3


#include "HX711.h"
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 6;
const int LOADCELL_SCK_PIN = 5;

// for press button
//const int buttonPin = 3; // for tare pin
//const int fillbutton = 2;

//ezButton button(fillbutton);  // create ezButton object that attach to pin 7;
//for on-off switch
const int switch1 = 8; // for getting into setup mode



//
//TM1637Display display7(CLK, DIO);
HX711 scale;

//-----------------copied from scale-------------------------------------------------------------------------------------------

/*******Interrupt-based Rotary Encoder Sketch*******
by Simon Merrett, based on insight from Oleg Mazurov, Nick Gammon, rt, Steve Spence
*/
#include <EEPROM.h>
#include <BfButton.h>
//For OLED added by KZY

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Fonts/FreeSerif9pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// 


#define EXE_INTERVAL_3 100
#define EXE_INTERVAL_2 800

unsigned long lastExecutedMillis_3 = 0; // vairable to save the last executed time for code block 1
unsigned long lastExecutedMillis_2 = 0; // vairable to save the last executed time for code block 2


int address1 = 0;      //EEPROM address counter
//int address2 = 5;      //EEPROM address counter

int btnPin=17; //GPIO #17-Push button on encoder

BfButton btn(BfButton::STANDALONE_DIGITAL, btnPin, true, LOW);

static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
// the middle pin to GND
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte oldEncPos = 0; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent

volatile int menuCount;
volatile int menu1Count=0;
volatile int menu2Count=0;
volatile byte setVal =0;
//volatile byte count =0;

boolean Fill_ButtonPressed = false;
boolean D_ButtonPressed = false;
boolean showCursor = false;
boolean onoff = false;
boolean filling = false;
boolean filled = false;
void staticDisplay(){

 display.setCursor(20, 20);
 display.println ("Menu"); //text
 display.display();
 

 display.setCursor(20, 40);
 display.println ("Set Val :"); //text
  display.setCursor(90, 40);
 display.println (menu1Count);
 display.display();
 
 display.setCursor(20, 60);
 display.println ("P Val :"); //text
   display.setCursor(90, 60);
 display.println (menu2Count);
 display.display();
 
}


void updateLCD()
{  

 display.clearDisplay();

 staticDisplay();

 display.setCursor(90, 40);
 display.println (menu1Count);
 display.display();
  display.setCursor(90, 60);
 display.println (menu2Count);
 display.display();
 
}

void Set_Cursor(){ //if double-pressed, the cursor will show up.
 if(D_ButtonPressed){

 
 display.setCursor(0, 40);
 display.println (">"); //text
 display.display();
 }  

 }



//Button press hanlding function
void pressHandler (BfButton *btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:
      Serial.println("Single push");
      D_ButtonPressed = (!D_ButtonPressed);
      break;
      
    case BfButton::DOUBLE_PRESS:
      Serial.println("Double push");
      
      break;
      
    case BfButton::LONG_PRESS:
      Serial.println("Long push");
      break;
  }
}


void setup() {

  
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(fillbutton, INPUT_PULLUP);
  pinMode(switch1, INPUT_PULLUP);
  
  Serial.println("HX711 Demo");

  Serial.println("Initializing the scale");

  // Initialize library with data output pin, clock input pin and gain factor.
  // Channel selection is made by passing the appropriate gain:
  // - With a gain factor of 64 or 128, channel A is selected
  // - With a gain factor of 32, channel B is selected
  // By omitting the gain factor parameter, the library
  // default "128" (Channel A) is used here.
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  scale.set_scale(384700.f);                      // this is inversely proportional to the output
  scale.tare();               // reset the scale to 0
  // Interrupt for button press for tare function
  attachInterrupt(digitalPinToInterrupt(buttonPin),buttonPressed,RISING);  //  function for creating external interrupts at pin8 on Rising (LOW to HIGH)   
  //attachInterrupt(digitalPinToInterrupt(fillbutton),fillbuttonPressed,RISING); 
  //-----------------------------------------------------
   pinMode(10, OUTPUT);    // sets the digital pin 11 as output
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(digitalPinToInterrupt(PinA),PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(digitalPinToInterrupt(PinB),PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  Serial.begin(9600); // start the serial monitor link

    //Button settings
  btn.onPress(pressHandler)
  .onDoublePress(pressHandler) // default timeout
  .onPressFor(pressHandler, 1000); // custom timeout for 1 second
  
  
   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
   Serial.println("SSD1306 allocation failed");
   for(;;);
 }
 delay(2000);
 display.setFont(&FreeSerif9pt7b);
 
 display.clearDisplay();
 display.setTextSize(1);             
 display.setTextColor(WHITE);        
 display.setCursor(25,40);  
 
 display.println("Welcome!");
 display.display();  
                                                               
 delay(2000); 
 display.clearDisplay();
 // Retrieve data from EEPROM
 setVal = EEPROM.read(0);
menu1Count = setVal*10;
//menu2Count = count*10;

 
  staticDisplay();
}

void PinA(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void loop(){

if(digitalRead(switch1)==HIGH){
  //interrupts();

   btn.read();
  
  if(oldEncPos != encoderPos) {
    Serial.println(encoderPos);
   
    if(D_ButtonPressed==true ){
    //  display.clearDisplay();
      showCursor = true;
   
      if(oldEncPos<encoderPos){
      setVal++;
      menu1Count=10* setVal;
      if(menu1Count>2500){
        menu1Count=0;
      }
    } else if (oldEncPos>encoderPos){
      setVal--;
       menu1Count=10* setVal;
      if(menu1Count<0){
        menu1Count=2500;
      }
    }
        Serial.print("SetVal:");
        Serial.println(menu1Count);
      
      updateLCD();
      Set_Cursor();
    }
    
   oldEncPos = encoderPos;
  }
  if (showCursor == true && D_ButtonPressed == false){
    updateLCD();
    //Update EEPROM
    EEPROM.update(address1, (menu1Count/10)); // EEPROM can hold from 0 to 255
   // EEPROM.update(address2, (menu1Count/10));
    showCursor=false;
  }
} else if (digitalRead(switch1)==LOW){ // HIGH means off; LOW means on
 
 
    unsigned long currentMillis = millis();


  if (currentMillis - lastExecutedMillis_3 >= EXE_INTERVAL_3) {
    lastExecutedMillis_3 = currentMillis; // save the last executed time
    menu2Count = scale.get_units(3)*1000;
//    Serial.print("weight:");
//    Serial.println(menu2Count);
}

  if (currentMillis - lastExecutedMillis_2 >= EXE_INTERVAL_2) {
    lastExecutedMillis_2 = currentMillis; // save the last executed time
    updateLCD();
}  
if ((menu2Count<menu1Count)&&(digitalRead(fillbutton)==LOW)&&(filled==false)){ //Low means there is input
  digitalWrite(10, HIGH); // sets the digital pin 11 on
  Serial.println("There is output");
}
else {
  digitalWrite(10,LOW);
  Serial.println("no output");
}

if(menu2Count>=menu1Count)
  filled=true;


if(digitalRead(fillbutton)==HIGH)
  filled=false;

//else if((menu2Count<menu1Count)&&(digitalRead(fillbutton)==HIGH))
//{
//  filled=false;
//}



}
}

void buttonPressed(){
  delay(200);
 scale.tare(); 
 Serial.println("Tare button pressed");
}
