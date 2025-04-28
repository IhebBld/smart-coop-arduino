#include <Wire.h>
#include <RTClib.h>
#include <Servo.h>
#include <LiquidCrystal.h>

// Pin definitions
#define WATER_LEVEL_SENSOR A0   // Water sensor connected to A0
#define PUMP_RELAY 8            // Relay for pump control on pin 8
#define SERVO_PIN 9             // Servo for feeder on pin 9
#define FEEDER_OPEN_ANGLE 180
#define FEEDER_CLOSED_ANGLE 0

// LCD pin definitions (matching your circuit)
#define LCD_RS 12
#define LCD_E 11
#define LCD_D4 7
#define LCD_D5 6
#define LCD_D6 5
#define LCD_D7 4

// Threshold values
#define WATER_LEVEL_THRESHOLD 300  // Adjust based on testing with your potentiometer
#define FEEDING_DURATION 3000      // Time to keep feeder open in milliseconds

// Initialize objects
RTC_DS1307 rtc;
Servo feederServo;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Feeding schedule (24-hour format)
const int feedingHours[] = {8, 16};  // 8 AM and 4 PM
const int feedingMinutes[] = {0, 0}; // At 0 minutes

// Variables
bool pumpActive = false;
unsigned long pumpStartTime = 0;
const unsigned long pumpMaxRuntime = 10000;  // Maximum pump runtime in ms (10 seconds)
unsigned long lastFeedingTime = 0;
bool fedToday[2] = {false, false};  // Tracks if feeding has occurred

void setup() {
  // Initialize Serial communication
  Serial.begin(9600);
  
  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  // For Proteus simulation, set a specific time for testing
  // Comment this out when uploading to real hardware
  rtc.adjust(DateTime(2025, 4, 28, 7, 58, 0)); // Set to 7:58 AM for testing morning feeding
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("Chicken Coop");
  lcd.setCursor(0, 1);
  lcd.print("System Starting");
  delay(2000);
  
  // Initialize Servo
  feederServo.attach(SERVO_PIN);
  feederServo.write(FEEDER_CLOSED_ANGLE);
  
  // Initialize Relay pin
  pinMode(PUMP_RELAY, OUTPUT);
  digitalWrite(PUMP_RELAY, LOW);  // Turn off pump initially
  
  // Initialize water level sensor (analog input, no pinMode needed)
}

void loop() {
  // Get current time from RTC
  DateTime now = rtc.now();
  
  // Check if it's feeding time
  checkFeedingSchedule(now);
  
  // Check water level and control pump
  checkWaterLevel();
  
  // Update the LCD display
  updateLCD(now);
  
  // Add a small delay to prevent rapid cycling
  delay(200);
}

void checkFeedingSchedule(DateTime now) {
  // Check for each scheduled feeding time
  for (int i = 0; i < sizeof(feedingHours)/sizeof(feedingHours[0]); i++) {
    // Check if it's feeding time and if we haven't fed for this slot today
    if (now.hour() == feedingHours[i] && now.minute() == feedingMinutes[i] && !fedToday[i]) {
      // Activate feeder
      activateFeeder();
      
      // Mark as fed for this slot
      fedToday[i] = true;
      
      // Record feeding time
      lastFeedingTime = millis();
      
      Serial.print("Feeding #");
      Serial.print(i+1);
      Serial.println(" activated!");
    }
    
    // Reset fedToday flag if we're past the feeding time
    if ((now.hour() > feedingHours[i]) || 
        (now.hour() == feedingHours[i] && now.minute() > feedingMinutes[i] + 1)) {
      // Only reset if it was previously set (prevents multiple resets)
      if (fedToday[i]) {
        fedToday[i] = false;
        Serial.print("Reset feeding flag for slot ");
        Serial.println(i+1);
      }
    }
  }
}

void activateFeeder() {
  // Open the feeder
  feederServo.write(FEEDER_OPEN_ANGLE);
  
  // Display on LCD
  lcd.clear();
  lcd.print("Feeding Time!");
  lcd.setCursor(0, 1);
  lcd.print("Dispensing Food");
  
  // Keep the feeder open for the feeding duration
  delay(FEEDING_DURATION);
  
  // Close the feeder
  feederServo.write(FEEDER_CLOSED_ANGLE);
}

void checkWaterLevel() {
  // Read water level from sensor
  int waterLevel = analogRead(WATER_LEVEL_SENSOR);
  
  // Debug output (optional)
  Serial.print("Water level: ");
  Serial.println(waterLevel);
  
  // Check if water level is below threshold
  if (waterLevel < WATER_LEVEL_THRESHOLD) {
    // If pump is not already active, start it
    if (!pumpActive) {
      digitalWrite(PUMP_RELAY, HIGH);  // Turn on pump
      pumpActive = true;
      pumpStartTime = millis();
      Serial.println("Water level low. Pump activated.");
    }
  } else {
    // If pump is active and water level is sufficient, stop it
    if (pumpActive) {
      digitalWrite(PUMP_RELAY, LOW);  // Turn off pump
      pumpActive = false;
      Serial.println("Water level sufficient. Pump deactivated.");
    }
  }
  
  // Safety check: don't run the pump too long
  if (pumpActive && (millis() - pumpStartTime > pumpMaxRuntime)) {
    digitalWrite(PUMP_RELAY, LOW);  // Turn off pump
    pumpActive = false;
    Serial.println("Pump timeout for safety. Check water supply!");
  }
}

void updateLCD(DateTime now) {
  // Update every second (not every loop iteration)
  static unsigned long lastLCDUpdate = 0;
  if (millis() - lastLCDUpdate < 1000) return;
  lastLCDUpdate = millis();
  
  // Read water level from sensor
  int waterLevel = analogRead(WATER_LEVEL_SENSOR);
  
  // Format time string
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  
  // Clear and update LCD
  lcd.clear();
  lcd.print(timeStr);
  lcd.print(" W:");
  
  // Calculate water percentage (0-100%)
  int waterPercent = map(waterLevel, 0, 1023, 0, 100);
  lcd.print(waterPercent);
  lcd.print("%");
  
  // Second row
  lcd.setCursor(0, 1);
  
  // Show status
  if (pumpActive) {
    lcd.print("Pump: ON");
  } else {
    // Display next feeding time
    lcd.print("Next Feed: ");
    
    // Find the next feeding time
    int nextFeedingIndex = -1;
    for (int i = 0; i < sizeof(feedingHours)/sizeof(feedingHours[0]); i++) {
      if (now.hour() < feedingHours[i] || 
         (now.hour() == feedingHours[i] && now.minute() < feedingMinutes[i])) {
        nextFeedingIndex = i;
        break;
      }
    }
    
    // If we didn't find a next feeding time today, the next one is the first one tomorrow
    if (nextFeedingIndex == -1) {
      nextFeedingIndex = 0;
    }
    
    // Display the next feeding time
    char feedTimeStr[6];
    sprintf(feedTimeStr, "%02d:%02d", feedingHours[nextFeedingIndex], feedingMinutes[nextFeedingIndex]);
    lcd.print(feedTimeStr);
  }
}