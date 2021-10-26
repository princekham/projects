
/*******Interrupt-based Rotary Encoder Sketch*******
by Simon Merrett, based on insight from Oleg Mazurov, Nick Gammon, rt, Steve Spence
*/
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


int btnPin=4; //GPIO #3-Push button on encoder

BfButton btn(BfButton::STANDALONE_DIGITAL, btnPin, true, LOW);

static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte oldEncPos = 0; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent

volatile int menuCount;
volatile int menu1Count=0;
volatile int menu2Count=0;
volatile int count1 =0;
volatile int count2 =0;

boolean S_ButtonPressed = false;
boolean D_ButtonPressed = false;
boolean showCursor = false;
void staticDisplay(){

 display.setCursor(20, 20);
 display.println ("Menu"); //text
 display.display();
 

 display.setCursor(20, 40);
 display.println ("L Pump :"); //text
  display.setCursor(90, 40);
 display.println (menu1Count);
 display.display();
 
 display.setCursor(20, 60);
 display.println ("R Pump :"); //text
   display.setCursor(90, 60);
 display.println (menu2Count);
 display.display();
 
}
//void updateLCD(int value)
//{  
//
// display.clearDisplay();
//
// staticDisplay();
//
// display.setCursor(90, 30);
// display.println (value);
// display.display();
// 
//
// display.setCursor(90, 60);
// display.println (value);
// display.display();
// 
//}

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

void Set_Cursor(){
 if(D_ButtonPressed){

 if(menuCount==1){
 display.setCursor(0, 60);
 display.println (">"); //text
 display.display();
 }else if(menuCount==0){
 
 display.setCursor(0, 40);
 display.println (">"); //text
 display.display();
 }  
// else if(menuCount==2) {
// display.setCursor(0,20);
// display.println (">"); //text
// display.display();
// }

 }

 }

//Button press hanlding function
void pressHandler (BfButton *btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:
      Serial.println("Single push");
      if(D_ButtonPressed){
         S_ButtonPressed = (!S_ButtonPressed);
      }
      break;
      
    case BfButton::DOUBLE_PRESS:
      Serial.println("Double push");
      D_ButtonPressed = (!D_ButtonPressed);
      break;
      
    case BfButton::LONG_PRESS:
      Serial.println("Long push");
      break;
  }
}


void setup() {
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0,PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  Serial.begin(9600); // start the serial monitor link

    //Button settings
  btn.onPress(pressHandler)
  .onDoublePress(pressHandler) // default timeout
  .onPressFor(pressHandler, 1000); // custom timeout for 1 second

  
   //.................This part added by Sao
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

   btn.read();
  if(oldEncPos != encoderPos) {
    Serial.println(encoderPos);
   
    
    if(D_ButtonPressed ==true&& S_ButtonPressed ==false){
      showCursor = true;
    if(oldEncPos<encoderPos){
      menuCount++;
      if(menuCount>1){
        menuCount=0;
      }
    } else if (oldEncPos>encoderPos){
      menuCount--;
      if(menuCount<0){
        menuCount=1;
      }
    }
    Serial.println(menuCount);
    updateLCD();
    Set_Cursor();
  }
    if(D_ButtonPressed==true && S_ButtonPressed==true){
    //  display.clearDisplay();
      showCursor = true;
      if(menuCount==0){
            if(oldEncPos<encoderPos){
      count1++;
      menu1Count=10* count1;
      if(menu1Count>999){
        menu1Count=0;
      }
    } else if (oldEncPos>encoderPos){
      count1--;
       menu1Count=10* count1;
      if(menu1Count<0){
        menu1Count=999;
      }
    }
        Serial.print("Menu1Count:");
        Serial.println(menu1Count);
      }else if(menuCount==1){

      if(oldEncPos<encoderPos){
      count2++;
       menu2Count=10* count2;
      if(menu2Count>999){
        menu2Count=0;
      }
    } else if (oldEncPos>encoderPos){
      count2--;
      menu2Count=10* count2;
      if(menu2Count<0){
        menu2Count=999;
      }
    }
        Serial.print("Menu2Count:");
        Serial.println(menu2Count);
      }
      updateLCD();
      Set_Cursor();
    }
    
   oldEncPos = encoderPos;
  }
  if (showCursor == true && D_ButtonPressed == false){
    updateLCD();
    showCursor=false;
  }
}
