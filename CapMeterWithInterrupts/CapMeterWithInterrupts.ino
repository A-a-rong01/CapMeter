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




//Calulating Variables
float selectedResistor;
float frequency;
float capacitance;
String capLabel;


// State flags
volatile bool capacitorDetected = false; // Flag set by interrupt when capacitor is detected
bool relayCheckInProgress = false;       // Flag to manage auto-sensing process
unsigned long lastRelayCheckTime = 0;    // Timestamp for non-blocking relay checks
bool relayOneChecked = false;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //tells the teensy where the pins are


void setup() {
  // put your setup code here, to run once:
  pinMode(timerOutput, INPUT);
  pinMode(relayOne, OUTPUT);
  pinMode(relayTwo, OUTPUT);
  pinMode(relayThree, OUTPUT);
  pinMode(relayFour, OUTPUT);

// Initialize relays to OFF
  digitalWrite(relayOne, LOW);
  digitalWrite(relayTwo, LOW);
  digitalWrite(relayThree, LOW);
  digitalWrite(relayFour, LOW);

  lcd.begin(16, 2);
  lcd.print("Capacitor meter:");

  Serial.begin(9600);

  // Attach interrupt for capacitor detection
  attachInterrupt(digitalPinToInterrupt(timerOutput), detectCapacitor, CHANGE);

}

void loop() {
  if (!relayOneChecked) {
    // Turn on relayOne once to check if a capacitor is attached
    digitalWrite(relayOne, HIGH);
    delay(10); // Allow the signal to stabilize
    relayOneChecked = true; // Mark that we've checked using relayOne
  }

  if(!capacitorDetected){
    relayOneChecked = false; // Allow re-checking when a capacitor is removed
    lcd.setCursor(0,1);
    lcd.print("No capacitor ");
    delay(2000);
    lcd.setCursor(0,1);
    lcd.print("Please insert");
    delay(2000);
    return;
  } else {
    // Ensure relayOne is turned off once a capacitor is detected
    digitalWrite(relayOne, LOW);
  }
  
  digitalWrite(relayTwo, HIGH);
  // selectedResistor = selectResistor();
  // Serial.println(selectedResistor);
  calcDuration();
  calculateFrequency();
  calculateCapacitance(frequency, RB_Two);
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
  lcd.print(capacitance, 2); // Print frequency with 2 decimal places
  lcd.print(capLabel);
  
  delay(1000);  // Update once per second

}


// Interrupt Service Routine (ISR) for capacitor detection
void detectCapacitor() {
  if (digitalRead(timerOutput) == HIGH) {
    capacitorDetected = true;
  } else {
    capacitorDetected = false;
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
  capacitance = 1.44 / ((RA + 2 * secResistor) * frequency);
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
