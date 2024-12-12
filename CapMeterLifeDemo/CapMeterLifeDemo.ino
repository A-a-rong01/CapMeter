#include <LiquidCrystal.h>

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


// Variables to store time measurements
unsigned long pulseDuration;
//Variables for helping me testing the number of pulses
const int autoRangingPulseTimer = 5;

//Calulating Variables
float frequency;
float capacitance = 0.00;
float tripleFiveTimerConstant = 1.44729;
float resistorRange = 90000;

//label for capacitance
volatile bool capacitorDetected = false; // Flag set by interrupt when capacitor is detected
String capLabel;

volatile bool begIntroRan = false; // Flag set by interrupt when capacitor is detected



LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //tells the teensy where the pins are

void setup() {
  // put your setup code here, to run once:
  pinMode(timerOutput, INPUT);
  pinMode(relayOne, OUTPUT);
  pinMode(relayTwo, OUTPUT);
  pinMode(relayThree, OUTPUT);
  pinMode(relayFour, OUTPUT);

  digitalWrite(relayOne, HIGH);
  digitalWrite(relayTwo, LOW);
  digitalWrite(relayThree, LOW);
  digitalWrite(relayFour, LOW);

  lcd.begin(16, 2);
  lcd.print("B EE 425");
  lcd.setCursor(0, 1);
  lcd.print("Design Project");
  delay(2000); 

  Serial.begin(9600);

  // Attach interrupt for capacitor detection
  attachInterrupt(digitalPinToInterrupt(timerOutput), detectCapacitor, FALLING);
}

void loop() {
  if(!begIntroRan){
    introMessage();
    begIntroRan = true;
  }

  if(capacitance == 0.00){ // mabybe add "!capacitorDetected ||" later
    noCapMessageLite();
    Serial.println("No Capacitor Detected");
  }
  
  calcDuration();
  calculateFrequency();
  findRange();
  calculateCapacitance(frequency, resistorRange);
  // Print the results
  Serial.print("Frequency: ");
  Serial.print(frequency);
  Serial.println(" Hz");

  Serial.print("Capacitance: ");
  Serial.print(capacitance, 10);
  Serial.println(capLabel);
  
  // Update LCD only on significant change
  static float lastCapacitance = 0;
  if (abs(capacitance - lastCapacitance) > 0.01) { // Update only on significant changes
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Capacitance: ");
    lcd.setCursor(0, 1);
    lcd.print(capacitance, 2);
    lcd.print(capLabel);
    lastCapacitance = capacitance;
}
  delay(1000);  // Update once per second

}
void introMessage() {
  lcd.setCursor(0,0);
  lcd.print("Long Bui");
  lcd.setCursor(0, 1);
  lcd.print("Aaron Guerrero");
  delay(2000); 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Capacitance:");
}


// Interrupt Service Routine (ISR) for capacitor detection
void detectCapacitor() {
  if (digitalRead(timerOutput) == HIGH) {
    capacitorDetected = true;
  } else {
    capacitorDetected = false;
  }
}

void noCapMessageLite() {
    lcd.setCursor(0,1);
    lcd.print("No Cap Detected");
}


float calcDuration(){
  pulseDuration = pulseIn(timerOutput, HIGH); // Measure the pulse duration in microseconds
  return pulseDuration;
}

float calculateFrequency() {
  if (pulseDuration == 0) {
    frequency = 0; // Or some default value
    return frequency;
  }
  frequency = 1000000.0 / pulseDuration;  // Convert microseconds to Hz
  return frequency; // Still need to determine how to add range
}

//This function calculates the capacitance after finding the frequency
float calculateCapacitance(float frequency, float secResistor) {
  if (frequency == 0) {
    capacitance = 0; // Or some default value
    return capacitance;
  }
  capacitance = tripleFiveTimerConstant / ((RA + 2 * secResistor) * frequency);
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
    Serial.print("Capacitance scale error");
  }
  return capacitance; // Still need to determine how to add range
}

void findRange() {
    if(frequency > 2e3){
      resistorRange = RB_One;
      Serial.print("first range!");
    } else {
      digitalWrite(relayOne, LOW);
      digitalWrite(relayTwo, HIGH);
      Serial.print("Second range");
      calcDuration();
      calculateFrequency();
      if(frequency > 150 && frequency <50e3){
        Serial.println("second range");
        resistorRange = RB_Two;
      } else{
        digitalWrite(relayTwo, LOW);
        digitalWrite(relayThree, HIGH);
        Serial.print("Third range");
        calcDuration();
        calculateFrequency();
        if(frequency > 6 && frequency < 200){
          Serial.print("Third range");
          resistorRange = RB_Three;
        } else {
          Serial.print("Fourth range");
          resistorRange = RB_Four;
        }
      }
    }
}




