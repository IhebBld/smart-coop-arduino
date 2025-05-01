#include <Servo.h>

const int ldrPin = A1;    
const int trigPin = 5;     
const int echoPin = 6;     
const int ledPin = 7;     
const int servoPin = 3;  

const int lightThreshold = 600;
const int predatorDistance = 30;  

const int doorOpen = 180;
const int doorClosed = 0;

Servo doorServo;
bool isDoorOpen = false;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  
  doorServo.attach(servoPin);
  doorServo.write(doorClosed); 
}

void loop() {
  int light = analogRead(ldrPin);
  
  if (light > lightThreshold && !isDoorOpen) {
    doorServo.write(doorOpen);
    isDoorOpen = true;
    digitalWrite(ledPin, LOW);
  }
  else if (light <= lightThreshold && isDoorOpen) {
    doorServo.write(doorClosed);
    isDoorOpen = false;
  }
  
  if (!isDoorOpen) {
    int distance = getDistance();
    if (distance > 0 && distance < predatorDistance) {
      digitalWrite(ledPin, HIGH);
      delay(600);
      digitalWrite(ledPin, LOW);
      delay(100);
    } else {
      digitalWrite(ledPin, LOW);
    }
  }
  
  delay(500); 
}

int getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration * 0.0343 / 2; 
}