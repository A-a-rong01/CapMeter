#include <LiquidCrystal.h>

//The following are the pins for the components connecting to the microcontroller
const int timerOutput = 9; //The pin that connects to the 555 timer output
const int relayOne = 28, relayTwo = 29, relayThree = 30, relayFour = 31; //The different pins for each of the different relays
const int rs = 3, en = 4, d4 = 24, d5 = 25, d6 = 26, d7 = 27; //The pins for the LCD screen

LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //tells the teensy where the pins are

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

  Serial.begin(9600);

}

void loop() {
  digitalWrite(relayThree, HIGH);
  // delay(500);
  while (digitalRead(timerOutput) == LOW);
  pulseStartTime = micros();  // Record the time at the start of the pulse
  
  // Wait for the pulse to go LOW again
  while (digitalRead(timerOutput) == HIGH);
  pulseEndTime = micros();  // Record the time at the end of the pulse
  
  // Calculate the pulse duration in microseconds
  pulseDuration = pulseEndTime - pulseStartTime;
  
  calculateFrequency();
  capacitance = (1.44 / ((RA + (2 * RB_Three)) * frequency))*1000000;
  // Print the results
  Serial.print("Frequency: ");
  Serial.print(frequency);
  Serial.println(" Hz");

  Serial.print("Capacitance: ");
  Serial.print(capacitance, 10);
  Serial.println(" uF");
  
  // Display the frequency on the LCD
  lcd.clear(); // Clear the previous display
  lcd.setCursor(0, 0); // Set cursor to the first line
  lcd.print("Capacitance: ");
  lcd.setCursor(0, 1); // Set cursor to the first line
  lcd.print(capacitance, 2); // Print frequency with 2 decimal places
  lcd.print("uF");
  
  delay(1000);  // Update once per second

}

float calcDuration(){
  // Calculate the pulse duration in microseconds
  pulseDuration = pulseEndTime - pulseStartTime;
  return pulseDuration;
}

float calculateFrequency() {
  frequency = 1000000.0 / pulseDuration;  // Convert microseconds to Hz
  return frequency; // Still need to determine how to add range
}

//This function calculates the capacitance after finding the frequency
float calculateCapacitance(float frequency,float secResistor) {
  capacitance = 1.44 / ((RA + (2 * secResistor)) * frequency);
  return capacitance; // Still need to determine how to add range
}
