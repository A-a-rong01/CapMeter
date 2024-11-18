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
unsigned long pulseStartTime;
unsigned long pulseEndTime;
unsigned long pulseDuration;


//Calulating Variables
float selectedResistor;
float frequency;
float capacitance;


LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //tells the teensy where the pins are

void setup() {
  // put your setup code here, to run once:
  pinMode(timerOutput, INPUT);
  pinMode(relayOne, OUTPUT);
  pinMode(relayTwo, OUTPUT);
  pinMode(relayThree, OUTPUT);
  pinMode(relayFour, OUTPUT);

  digitalWrite(relayOne, LOW);
  digitalWrite(relayTwo, LOW);
  digitalWrite(relayThree, LOW);
  digitalWrite(relayFour, LOW);

  lcd.begin(16, 2);
  lcd.print("Capacitor meter:");
}

void loop() {
  // put your main code here, to run repeatedly:
  while(!isCapacitorAttached()){
    lcd.setCursor(0,1);
    lcd.print("No capacitor ");
    delay(2000);
    lcd.setCursor(0,1);
    lcd.print("Please insert");
    delay(2000);
  }
  
  selectedResistor = selectResistor(); //Essentially is the autorange

  //The if statement makes sure that there is a selected resistor value before calculating the capacitance
  if(selectedResistor != -1){
    turnRelayOn(selectedResistor); // Turns on the Relay after Range is found
    pulseDuration = calcDuration(); //When Relay is turned on, determines duration of the pulse
    frequency = calculateFrequency(); //Calculates freq
    capacitance = calculateCapacitance(frequency, selectedResistor); //Calculated Capacitance after determining RB value and Freq
    displayCapacitance(capacitance); //Displays capacitance
  }
}

//This function checks to see if there is a capacitor attached
bool isCapacitorAttached(){
  digitalWrite(relayOne, HIGH);
  if (digitalRead(timerOutput) == HIGH){
    return true;
  } else {
    return false;
  }
}


//This function determined which reistor to use for the calculations aka AutoRange
float selectResistor() {
  digitalWrite(relayOne, HIGH);
  delay(1); // Wait for 1 millisecond
  pulseDuration = pulseIn(timerOutput, HIGH, 1000);
  digitalWrite(relayOne, LOW);
  if (pulseDuration > 0) {
    return RB_One;
  } else {
    digitalWrite(relayTwo, HIGH);
    delay(1);
    pulseDuration = pulseIn(timerOutput, HIGH, 1000);
    digitalWrite(relayTwo, LOW);
      if (pulseDuration > 0) {
        return RB_Two;
      } else {
        digitalWrite(relayThree, HIGH);
        delay(1);
        pulseDuration = pulseIn(timerOutput, HIGH, 1000);
        digitalWrite(relayThree, LOW);
          if (pulseDuration > 0) {
            return RB_Three;
          } else {
            digitalWrite(relayFour, HIGH);
            delay(1);
            pulseDuration = pulseIn(timerOutput, HIGH, 1000);
            digitalWrite(relayFour, LOW);
            return RB_Four;
      }
    }
  }
}

//Once the range/resistor value is found, we turn on the relay and measure the pulses.
float turnRelayOn(float selectedResistor) {
  // Turn off all relays first
  digitalWrite(relayOne, LOW);
  digitalWrite(relayTwo, LOW);
  digitalWrite(relayThree, LOW);
  digitalWrite(relayFour, LOW);

  // Turn on the appropriate relay
  if (selectedResistor == RB_One){
    digitalWrite(relayOne, HIGH);
  } else if (selectedResistor == RB_Two){
    digitalWrite(relayTwo, HIGH);
  } else if (selectedResistor == RB_Three){
    digitalWrite(relayThree, HIGH);
  } else if (selectedResistor == RB_Four){
    digitalWrite(relayFour, HIGH);
  } 
}

float calcDuration(){
  pulseDuration = pulseIn(timerOutput, HIGH); // Measure the pulse duration in microseconds
  return pulseDuration;
}

//This function calculates the frequency after selecting Rb and correct Relay value
float calculateFrequency() {
  frequency = 1000000.0 / pulseDuration;  // Convert microseconds to Hz
  return frequency; // Still need to determine how to add range
}

//This function calculates the capacitance after finding the frequency
float calculateCapacitance(float frequency, float secResistor) {
  float capacitance = 1.44 / (0.693 * (RA + 2 * secResistor) * frequency);
  return capacitance; // Still need to determine how to add range
}



//This function displays the capacitance on the LCD display
void displayCapacitance(float capacitance) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Capacitance:");
  lcd.setCursor(0, 1);
  lcd.print(capacitance, 2);
  lcd.print(" uF");
}



