/**
 * CAPACITANCE METER PROGRAM
 *
 * This program measures capacitance using a 555 timer circuit and a Teensy microcontroller.
 * 
 * 
 * Features:
 * - Auto-ranging for capacitance measurement from 20 pF to 200 uF.
 * - Accuracy with a 555 timer constant-based formula from less than 15% to 2% tolarance based on range.
 * - Real-time results displayed on a 16x2 LCD.
 *
 * Author: Aaron Guerrero & Hoang Long Bui
 * Date: DECEMBER 13TH 2024 - (last official update)
 * UW Bothell - B EE 425: Microprocessor system design
 * Version: 4.0
 *
 *
 * 
 * Usage Notes:
 * - Ensure proper connections of all hardware components before running the program.
 * - Debugging messages can be read through the serial monitor.
 * 
 */
#include <LiquidCrystal.h>
#include <FreqCount.h>


//======================== PIN CONFIGURATION ========================
// Pins connecting the teensy microcontroller to various components
const int timerOutput = 9; // Pin connected to the 555 timer output
const int relayOne = 28, relayTwo = 29, relayThree = 30, relayFour = 31; // Relay pins for selecting resistor ranges
const int rs = 3, en = 4, d4 = 24, d5 = 25, d6 = 26, d7 = 27; // LCD screen pins
//========================================================================


//======================== RESISTOR VALUES ==========================
// Resistor values (in ohms) used for calculating capacitance in different ranges
const float RA = 1000;       // Fixed resistor value
const float RB_One = 90000;  // Resistor for 10pF to 2nF range (Relay One)
const float RB_Two = 400;    // Resistor for 22nF to 2.2uF range (Relay Two)
const float RB_Three = 100;  // Resistor for 3.3uF to 200uF range (Relay Three)
const float RB_Four = 50;    // Resistor for above 200uF range (Relay Four)
//========================================================================

//======================== MEASUREMENT VARIABLES ====================
// Variables for storing time and frequency measurements
unsigned long pulseDuration; // Time duration of the signal pulse
float frequency;             // Frequency of the signal in Hz
//========================================================================


//======================== CALCULATION VARIABLES ====================
// Variables for capacitance calculation
float capacitance = 0.00;     // Measured capacitance value
float CorrectedCap = 0.00;    // Corrected capacitance value
float tripleFiveTimerConstant = 1.44729; // Constant derived from 555 timer calculations
float selectedResistorBValue = 90000;    // Secondary resistor value (updated dynamically based on range)
//========================================================================


//======================== CAPACITANCE LABELS & SYSTEM FLAGS =======================
// Labels and flags for displaying capacitance and handling ranges
volatile bool capacitorDetected = false; // Set when a capacitor is detected
String capLabel;                         // Label for displaying capacitance units
String rangeSelected;                    // String to indicate selected range
// Flags for controlling program flow and error handling
volatile bool isIntroductionDisplayed = false;     // Indicates if the introduction message ran
volatile bool rangeFound = false;     // Indicates if the correct range has been identified; True if there is an error and false if none is found
volatile bool errorCapHandling = false; // Indicates an error in handling capacitance measurement
int autoSenseCounter = 0;             // Counter for auto-sensing retries
//========================================================================


//======================== LCD CONFIGURATION ========================
// LCD object initialization with defined pins
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // Specifies LCD connections to the microcontroller
//========================================================================


void setup() {
  // Configure the timer output pin as input
  pinMode(timerOutput, INPUT);

  // Configure relay pins as output
  pinMode(relayOne, OUTPUT);
  pinMode(relayTwo, OUTPUT);
  pinMode(relayThree, OUTPUT);
  pinMode(relayFour, OUTPUT);

  // Set all relays to LOW (inactive)
  digitalWrite(relayOne, LOW);
  digitalWrite(relayTwo, LOW);
  digitalWrite(relayThree, LOW);
  digitalWrite(relayFour, LOW);

  // Initialize the LCD screen
  lcd.begin(16, 2);
  lcd.print("B EE 425");
  lcd.setCursor(0, 1);
  lcd.print("Design Project");
  delay(2000); // Brief delay to display the initial message

  // Start serial communication
  Serial.begin(9600);

  // Attach interrupt to detect capacitor
  attachInterrupt(digitalPinToInterrupt(timerOutput), detectCapacitor, FALLING);
}


void loop() {
  if(!isIntroductionDisplayed){
    introMessage();
    isIntroductionDisplayed = true;
  }

  if(capacitance == 0.00 || abs(capacitance - 15.99) < 0.01 || abs(capacitance - 23.98) < 0.01){ // mabybe add "!capacitorDetected ||" later
    noCapMessageLite();
    Serial.println("No Capacitor Detected");
  }
  if(!rangeFound){
    findRange();
    rangeFound = true;
    return;
  }
  
  calcDuration();
  calculateFrequency();
  calculateCapacitance(frequency, selectedResistorBValue);


  // Print the results
  Serial.print("Frequency: ");
  Serial.print(frequency);
  Serial.println(" Hz");
  Serial.print("Calculated Capacitance: ");
  Serial.print(capacitance, 5);
  Serial.println(capLabel);
  Serial.print("Range Selected: ");
  Serial.println(rangeSelected);
  Serial.println("Resistor Selected: " + String(selectedResistorBValue) + " ohms");
  Serial.println("Corrected capacitance: " + String(CorrectedCap) + capLabel);


  String errorCap = String(capacitance,2) + capLabel; 
  if(errorCap == "15.99pF" || errorCap == "23.99pF"){
    errorCapHandling = true;
    Serial.print(errorCapHandling);
  }

  capacitanceCorrections(capacitance, capLabel);
  
  // Update LCD only on significant change
  static float lastCapacitance = 0;
  if (abs(CorrectedCap - lastCapacitance) > 0.01 || !errorCapHandling) { // Update only on significant changes
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Capacitance: ");
    lcd.setCursor(0, 1);
    lcd.print(CorrectedCap, 2);
    lcd.print(capLabel);
    lastCapacitance = CorrectedCap;
}
  delay(1000);  // Update once per second
  if(capacitance == 0.00 || abs(capacitance - 15.99) < 0.01 || abs(capacitance - 23.98) < 0.01 || abs(capacitance - lastCapacitance) > 100.00){
    rangeFound = false;
  }
  Serial.println("____________________________________________");
  delay(500);
}


void introMessage() {
  lcd.setCursor(0,0);
  lcd.print("Hoang Long Bui");
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
float calculateCapacitance(float frequency, float selectedResistorBValue) {
  if (frequency == 0) {
    capacitance = 0; // Or some default value
    return capacitance;
  }
  capacitance = tripleFiveTimerConstant / ((RA + 2 * selectedResistorBValue) * frequency);
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

String findLabel(float capacitance){
  if (capacitance >= 1e-6) {
        return "uF";
    } else if (capacitance >= 1e-9) {
        return "nF";
    } else if (capacitance >= 1e-12) {
        return "pF";
    } else {
        return "Unknown";
    }
}

void findRange() {
    if (rangeFound) {
        return; // Exit early if the range has already been found
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Let me sense.......");
    // Reset all relays to avoid conflicts
    digitalWrite(relayOne, LOW);
    digitalWrite(relayTwo, LOW);
    digitalWrite(relayThree, LOW);
    digitalWrite(relayFour, LOW);

    // Test first range
    digitalWrite(relayOne, HIGH);  // Activate relay one
    delay(100);  // Allow time for the signal to stabilize
    calcDuration();
    calculateFrequency();
    if (frequency > 2000.00 && frequency < 550000.00) {
        selectedResistorBValue = RB_One;
        rangeSelected = "First range";
        return;
    }

    // Test second range
    digitalWrite(relayOne, LOW);
    digitalWrite(relayTwo, HIGH);  // Activate relay two
    delay(100);
    calcDuration();
    calculateFrequency();
    Serial.println(frequency);
    if (frequency > 330 && frequency <= 50000) {
        selectedResistorBValue = RB_Two;
        rangeSelected = "Second range";
        return;
    }

    // Test third range
    digitalWrite(relayTwo, LOW);
    digitalWrite(relayThree, HIGH);  // Activate relay three
    delay(400);
    calcDuration();
    calculateFrequency();
    if (frequency  < 500 && frequency > 6) {
        selectedResistorBValue = RB_Three;
        rangeSelected = "Third range";
        return;
    }

    // Test fourth range
    digitalWrite(relayThree, LOW);
    digitalWrite(relayFour, HIGH);  // Activate relay four
    delay(100);
    calcDuration();
    calculateFrequency();
    if (frequency < 6 && frequency != 0.00) {
        selectedResistorBValue = RB_Four;
        rangeSelected = "Fourth range";
        return;
    }
    else{
      selectedResistorBValue = RB_One;
      rangeSelected = "Unknown range";
      digitalWrite(relayFour, LOW);
      digitalWrite(relayOne, HIGH);  // Activate relay four
      return;
    }
}

float capacitanceCorrections(float capacitance , String capLabel) {
  if(rangeSelected == "First range"){
    CorrectedCap = (capacitance * 1.869);
    Serial.println("Cap Correct line 1 detected");
  } else if(rangeSelected == "Second range"){
    CorrectedCap = (capacitance * 1.2620) + (0.2);
    Serial.println("Cap Correct line 2 detected");
  } else if(rangeSelected == "Third range"){
    CorrectedCap = (capacitance * 0.705) + (0.8535 * 1e-9);
    Serial.println("Cap Correct line 3 detected");
  } else if(rangeSelected == "Fourth range"){
    CorrectedCap = (capacitance);
    Serial.println("Cap Correct line 4 detected");
  }
  delay(500);
  return capacitance;
}