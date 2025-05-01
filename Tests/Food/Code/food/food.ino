#include <Wire.h>
#include <RTClib.h>
#include <Servo.h>

#define WATER_LEVEL_SENSOR A0 
#define PUMP_RELAY 8            
#define SERVO_PIN 9         
#define FEEDER_OPEN_ANGLE 180
#define FEEDER_CLOSED_ANGLE 0

#define WATER_LEVEL_THRESHOLD 300
#define FEEDING_DURATION 3000     

RTC_DS1307 rtc;
Servo feederServo;

const int feedingHours[] = {8, 16};  
const int feedingMinutes[] = {0, 0}; 

bool pumpActive = false;
unsigned long pumpStartTime = 0;
const unsigned long pumpMaxRuntime = 10000;
unsigned long lastFeedingTime = 0;
bool fedToday[2] = {false, false};  

void setup() {
  Serial.begin(9600);
  
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  rtc.adjust(DateTime(2025, 4, 28, 7, 58, 0));

  feederServo.attach(SERVO_PIN);
  feederServo.write(FEEDER_CLOSED_ANGLE);
  
  pinMode(PUMP_RELAY, OUTPUT);
  digitalWrite(PUMP_RELAY, LOW);  
  
  Serial.println("Chicken Coop System Starting");
}

void loop() {
  DateTime now = rtc.now();
  
  checkFeedingSchedule(now);
  
  checkWaterLevel();
  
  updateSerialStatus(now);
  
  delay(200);
}

void checkFeedingSchedule(DateTime now) {
  for (int i = 0; i < sizeof(feedingHours)/sizeof(feedingHours[0]); i++) {
    if (now.hour() == feedingHours[i] && now.minute() == feedingMinutes[i] && !fedToday[i]) {
      activateFeeder();
      
      fedToday[i] = true;
      
      lastFeedingTime = millis();
      
      Serial.print("Feeding #");
      Serial.print(i+1);
      Serial.println(" activated!");
    }
    
    if ((now.hour() > feedingHours[i]) || 
        (now.hour() == feedingHours[i] && now.minute() > feedingMinutes[i] + 1)) {
      if (fedToday[i]) {
        fedToday[i] = false;
        Serial.print("Reset feeding flag for slot ");
        Serial.println(i+1);
      }
    }
  }
}

void activateFeeder() {
  feederServo.write(FEEDER_OPEN_ANGLE);
  
  Serial.println("Feeding Time! Dispensing Food");
  
  delay(FEEDING_DURATION);
  
  feederServo.write(FEEDER_CLOSED_ANGLE);
}

void checkWaterLevel() {
  int waterLevel = analogRead(WATER_LEVEL_SENSOR);
  
  Serial.print("Water level: ");
  Serial.println(waterLevel);
  
  if (waterLevel < WATER_LEVEL_THRESHOLD) {
    if (!pumpActive) {
      digitalWrite(PUMP_RELAY, HIGH); 
      pumpActive = true;
      pumpStartTime = millis();
      Serial.println("Water level low. Pump activated.");
    }
  } else {
    if (pumpActive) {
      digitalWrite(PUMP_RELAY, LOW); 
      pumpActive = false;
      Serial.println("Water level sufficient. Pump deactivated.");
    }
  }
  
  if (pumpActive && (millis() - pumpStartTime > pumpMaxRuntime)) {
    digitalWrite(PUMP_RELAY, LOW); 
    pumpActive = false;
    Serial.println("Pump timeout for safety. Check water supply!");
  }
}

void updateSerialStatus(DateTime now) {
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate < 1000) return;
  lastStatusUpdate = millis();
  
  int waterLevel = analogRead(WATER_LEVEL_SENSOR);
  int waterPercent = map(waterLevel, 0, 1023, 0, 100);
  
  char timeStr[20];
  sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  
  Serial.print("Time: ");
  Serial.print(timeStr);
  Serial.print(" | Water Level: ");
  Serial.print(waterPercent);
  Serial.println("%");
  
  if (pumpActive) {
    Serial.println("Pump Status: ON");
  } else {
    Serial.print("Pump Status: OFF | Next Feeding: ");
    
    int nextFeedingIndex = -1;
    for (int i = 0; i < sizeof(feedingHours)/sizeof(feedingHours[0]); i++) {
      if (now.hour() < feedingHours[i] || 
         (now.hour() == feedingHours[i] && now.minute() < feedingMinutes[i])) {
        nextFeedingIndex = i;
        break;
      }
    }
    
    if (nextFeedingIndex == -1) {
      nextFeedingIndex = 0;
    }
    
    char feedTimeStr[6];
    sprintf(feedTimeStr, "%02d:%02d", feedingHours[nextFeedingIndex], feedingMinutes[nextFeedingIndex]);
    Serial.println(feedTimeStr);
  }
  
  Serial.println("------------------------------");
}