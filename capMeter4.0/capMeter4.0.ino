#include <LiquidCrystal.h>
#include <FreqCount.h>

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

//Calulating Variables
float frequency;
float capacitance = 0.00;
float CorrectedCap = 0.00;
float tripleFiveTimerConstant = 1.44729;
float secResistor = 90000;

//label for capacitance
volatile bool capacitorDetected = false; // Flag set by interrupt when capacitor is detected
String capLabel;
String rangeSelected;

volatile bool begIntroRan = false; // Flag set by interrupt when capacitor is detected
volatile bool rangeFound = false; // Flag set if range is found or not
volatile bool errorCapHandling = false; // Flag set if range is found or not
int autoSenseCounter = 0;


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
  calculateCapacitance(frequency, secResistor);
  // Print the results
  Serial.print("Frequency: ");
  Serial.print(frequency);
  Serial.println(" Hz");

  Serial.print("Capacitance: ");
  Serial.print(capacitance, 10);
  Serial.println(capLabel);

  Serial.print("Range Selected: ");
  Serial.println(rangeSelected);
  Serial.println("Resistor: " + String(secResistor));
  Serial.println("corrected cap: " + String(CorrectedCap));
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
  Serial.println("____________________________________________-");
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
        secResistor = RB_One;
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
        secResistor = RB_Two;
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
        secResistor = RB_Three;
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
        secResistor = RB_Four;
        rangeSelected = "Fourth range";
        return;
    }
    else{
      secResistor = RB_One;
      rangeSelected = "Unknown range";
      digitalWrite(relayFour, LOW);
      digitalWrite(relayOne, HIGH);  // Activate relay four
      return;
    }
}

float capacitanceCorrections(float capacitance , String capLabel) {
  if(rangeSelected =="First range"){
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