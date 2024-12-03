#include <LiquidCrystal.h>
#include <FreqCount.h>
// #include <FlexPWM.h>

//The following are the pins for the components connecting to the microcontroller
const int timerOutput = 9; //The pin that connects to the 555 timer output
const int relayOne = 28, relayTwo = 29, relayThree = 30, relayFour = 31; //The different pins for each of the different relays
const int rs = 3, en = 4, d4 = 24, d5 = 25, d6 = 26, d7 = 27; //The pins for the LCD screen


//The following are the resistor values used for calculating the capacitance
const float RA = 1000;
const float RB_One = 90000; //Resistor assosciated with RelayOne for 10pF to 2nF range
const float RB_Two = 400; //Resistor assosciated with RelayTwo for 22nF to 2.2uF range
const float RB_Three = 100; //Resistor assosciated with RelayThree for 3.3uF to 200uF range
const float RB_Four = 50; //Resistor assosciated with RelayFour for above 200uF range




// Variables for low capacitance calculation
volatile unsigned long pulseCount = 0;
volatile bool timerComplete = false;



//Calulating Capacitance Variables
float tripleFiveTimerConstant = 1.44729;
float selectedResistor;
volatile float frequency;
float capacitance;
float capacitanceUncorrected;
String capLabel;
const int gateTime = 1000; ///Used in FreqCount determine wether to edit or not?

unsigned long pulseDuration = 0;
// State flags
volatile bool capacitorDetected = false; // Flag set by interrupt when capacitor is detected
bool relayCheckInProgress = false;       // Flag to manage auto-sensing process
unsigned long lastRelayCheckTime = 0;    // Timestamp for non-blocking relay checks
bool relayOneChecked = false;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //tells the teensy where the pins are


void setup() {
//Input and output pin assignments
  pinMode(timerOutput, INPUT);
  pinMode(relayOne, OUTPUT);
  pinMode(relayTwo, OUTPUT);
  pinMode(relayThree, OUTPUT);
  pinMode(relayFour, OUTPUT);

// Initialize relays to OFF aka LOW
  digitalWrite(relayOne, LOW);
  digitalWrite(relayTwo, LOW);
  digitalWrite(relayThree, LOW);
  digitalWrite(relayFour, LOW);

//Using FreqCount function to start counting fequency with gate time initially 1000ms or 1sec
  FreqCount.begin(1000);

  lcd.begin(16, 2);
  lcd.print("Capacitor meter:");

  Serial.begin(9600);

  // Attach interrupt for capacitor detection
  // attachInterrupt(digitalPinToInterrupt(timerOutput), detectCapacitor, CHANGE);

}

void loop() {

digitalWrite(relayThree, HIGH);

calcDuration();
frequency = calculateFrequency();
calculateCapacitance(frequency, RB_Three);

if(capacitanceUncorrected < 2*1e-9){ //Range 1
  
} else if (capacitanceUncorrected > 2*1e-9 && capacitanceUncorrected < 2.2*1e-6){ //Range 2

} else if (capacitanceUncorrected > 2*1e-9 && capacitanceUncorrected < 2.2*1e-6){ //Range 3

} else if(capacitanceUncorrected > 200*1e-6){ //Range 4

}

   // Print the results
  Serial.print("Frequency: ");
  Serial.print(frequency);
  Serial.println(" Hz");

  Serial.print("Capacitance: ");
  Serial.print(capacitance, 10);
  Serial.println(capLabel);

  
  // Display the frequency on the LCD
lcd.clear(); // Clear the previous display
lcd.setCursor(0, 0); // Set cursor to the first line
lcd.print("Capacitance: ");
lcd.setCursor(0, 1); // Set cursor to the first line
lcd.print(frequency, 2); // Print frequency with 2 decimal places
lcd.print(capLabel);
}


// // Interrupt Service Routine (ISR) for capacitor detection
// void detectCapacitor() {
//   if (digitalRead(timerOutput) == HIGH) {
//     capacitorDetected = true;
//   } else {
//     capacitorDetected = false;
//   }
// }



float calcDuration(){
  pulseDuration = pulseIn(timerOutput, HIGH); // Measure the pulse duration in microseconds
  return pulseDuration;
}

float calculateFrequency() {
  frequency = 1000000.0 / pulseDuration;  // Convert microseconds to Hz
  return frequency; // Still need to determine how to add range
}

//This function calculates the capacitance after finding the frequency
float calculateCapacitance(float frequency, float secResistor) {
  capacitance = tripleFiveTimerConstant / ((RA + 2 * secResistor) * frequency);
  capacitanceUncorrected = capacitance;
  if(capacitance >= 1e-6){
    capacitance = capacitance * 1e6;
    capLabel = "uF";
  } else if(capacitance < 1e-6 && capacitance >= 1e-9) {
    capacitance = capacitance * 1e9;
    capLabel = "nF";
  } else if(capacitance < 1e-9 && capacitance >= 1e-12){
    capacitance = capacitance * 1e12;
    capLabel = "pF";
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Capacitance scale error");
  }
  return capacitance; // Still need to determine how to add range
}
