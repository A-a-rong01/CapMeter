#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

// Pin connected to the 555 timer output (Pin 3 of the 555 timer)
const int freqPin = 2; // You can change this depending on your connection

// Constants for your known resistor values (in ohms)
const float R1 = 10000.0; // 10k resistor
const float Rpot = 10000.0; // Potentiometer value in ohms (adjust as needed)

// Variables to store time measurements
unsigned long pulseStartTime;
unsigned long pulseEndTime;
unsigned long pulseDuration;
float frequency;
float capacitance;

void setup() {
  Serial.begin(9600);
  pinMode(freqPin, INPUT);
  lcd.begin(16, 2);

  // Debug message to confirm the setup is working
  Serial.println("Setup complete, waiting for pulses...");
}

void loop() {
  // Wait for a HIGH pulse
  while (digitalRead(freqPin) == LOW);
  pulseStartTime = micros();  // Record the time at the start of the pulse
  
  // Wait for the pulse to go LOW again
  while (digitalRead(freqPin) == HIGH);
  pulseEndTime = micros();  // Record the time at the end of the pulse
  
  // Calculate the pulse duration in microseconds
  pulseDuration = pulseEndTime - pulseStartTime;
  
  // Calculate the frequency
  frequency = 1000000.0 / pulseDuration;  // Convert microseconds to Hz
  
  
  
  // Print the results
  Serial.print("Frequency: ");
  Serial.print(frequency);
  Serial.println(" Hz");
  
  // Display the frequency on the LCD
  lcd.clear(); // Clear the previous display
  lcd.setCursor(0, 0); // Set cursor to the first line
  lcd.print("Freq: ");
  lcd.print(frequency, 2); // Print frequency with 2 decimal places
  lcd.print(" Hz");
  
  delay(1000);  // Update once per second
}









