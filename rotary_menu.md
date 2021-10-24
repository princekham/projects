```
/* Encoder Library - Basic Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 * This example code is in the public domain.
 */

#include <Encoder.h>


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

int menuCount = 0; //counts the clicks of the rotary encoder between menu items (0-1 in this case)


bool menu1_selected = false; //enable/disable to change the value of menu item
bool menu2_selected = false;
bool buttonPressed = true;

int menu1_Value = 0; //value within menu 1
int menu2_Value = 0; //value within menu 2
  int data;


// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder myEnc(2, 3);
//   avoid using pins with LEDs attached





#define INTERVAL_MESSAGE1 500
unsigned long time_1 = 0;

void staticDisplay(){

 
 display.setCursor(20, 30);
 display.println ("L Pump :"); //text
 display.display();
 
 display.setCursor(20, 60);
 display.println ("R Pump :"); //text
 display.display();
 
}

void updateLCD(int value)
{  

 display.clearDisplay();

 staticDisplay();

 display.setCursor(90, 30);
 display.println (value);
 display.display();
 

 display.setCursor(90, 60);
 display.println (value);
 display.display();
 
}
void setup() {
  pinMode(4, INPUT_PULLUP); //Button added by KZY
  Serial.begin(9600);
  Serial.println("Basic Encoder Test:");

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
 display.setCursor(25,30);  
 
 display.println("Demo");
 display.display();  
                                                               
 delay(2000); 
 display.clearDisplay();
 staticDisplay();

 attachInterrupt(digitalPinToInterrupt(4), pushButton, FALLING); //PushButton pin is an interrupt pin
}

long oldPosition  = -999;

void loop() {
  long newPosition = myEnc.read();
//  Serial.print("new position: "); // increase and decrese 4 fold
//  Serial.println(newPosition/4);

  if (newPosition != oldPosition) {

  if(buttonPressed){
    if(oldPosition<newPosition){
      menuCount--;
      if(menuCount<0){
        menuCount=2;
      }
    } else if (oldPosition>newPosition){
      menuCount++;
      if(menuCount>2){
        menuCount=0;
      }
    }
    Serial.print(menuCount);
  }

    
    oldPosition = newPosition;
    data=newPosition/4;
    Serial.println(data);

  



    
    updateLCD(data); 
  }

}
void pushButton(){
  
}
```
The following is rotary encoder with interrupt
```

/*******Interrupt-based Rotary Encoder Sketch*******
by Simon Merrett, based on insight from Oleg Mazurov, Nick Gammon, rt, Steve Spence
*/

static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte oldEncPos = 0; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent

void setup() {
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0,PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  Serial.begin(115200); // start the serial monitor link
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
  if(oldEncPos != encoderPos) {
    Serial.println(encoderPos);
    oldEncPos = encoderPos;
  }
}

```
