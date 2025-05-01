#include <Wire.h>
#include <RTClib.h>
#include <Servo.h>
#include <DHT.h>
#include <LiquidCrystal.h>

// ========== PIN DEFINITIONS ==========
#define CAPTEUR_EAU A0
#define CAPTEUR_LUMIERE A1
#define BROCHE_DHT A2
#define RELAIS_POMPE 7
#define RELAIS_CHAUFFAGE 8
#define VENTILATEUR 6
#define TRIG_ULTRASON A3
#define ECHO_ULTRASON 13
#define SERVO_NOURRITURE 9
#define SERVO_PORTE 10

// ========== LCD INITIALIZATION ==========
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// ========== SYSTEM PARAMETERS ==========
#define TYPE_DHT DHT11
#define ANGLE_OUVERT_NOURRITURE 180
#define ANGLE_FERME_NOURRITURE 0
#define ANGLE_OUVERT_PORTE 180
#define ANGLE_FERME_PORTE 0
#define SEUIL_EAU 300
#define DUREE_NOURRITURE 3000
#define SEUIL_LUMIERE 600
#define DISTANCE_PREDATEUR 30
#define SEUIL_TEMP 30
#define HUMID_BAS 30
#define HUMID_HAUT 60

// ========== OBJECT INITIALIZATION ==========
RTC_DS1307 rtc;
Servo servoNourriture;
Servo servoPorte;
DHT dht(BROCHE_DHT, TYPE_DHT);

// ========== FEEDING SCHEDULE ==========
const int heuresRepas[] = {8, 16};
const int minutesRepas[] = {0, 0};

// ========== SYSTEM STATE VARIABLES ==========
bool pompeActive = false;
unsigned long tempsDemarrePompe = 0;
const unsigned long dureeMaxPompe = 10000;
unsigned long dernierRepas = 0;
bool repasServi[2] = {false, false};
bool porteOuverte = false;
bool predatorDetected = false;

// ========== SETUP FUNCTION ==========
void setup() {
  Serial.begin(9600);
  
  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("RTC non trouve");
    while (1);
  }
  
  rtc.adjust(DateTime(2025, 4, 28, 7, 58, 0));
  
  dht.begin();
  
  servoNourriture.attach(SERVO_NOURRITURE);
  servoNourriture.write(ANGLE_FERME_NOURRITURE);
  
  servoPorte.attach(SERVO_PORTE);
  servoPorte.write(ANGLE_FERME_PORTE);
  
  pinMode(RELAIS_POMPE, OUTPUT);
  pinMode(RELAIS_CHAUFFAGE, OUTPUT);
  pinMode(VENTILATEUR, OUTPUT);
  pinMode(TRIG_ULTRASON, OUTPUT);
  pinMode(ECHO_ULTRASON, INPUT);
  
  digitalWrite(RELAIS_POMPE, LOW);
  digitalWrite(RELAIS_CHAUFFAGE, HIGH);
  digitalWrite(VENTILATEUR, LOW);
  
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Demarrage...");
  delay(1000);
  lcd.clear();
}

// ========== MAIN LOOP ==========
void loop() {
  DateTime maintenant = rtc.now();
  
  verifierRepas(maintenant);
  controlerEau();
  controlerPorte();
  controlerClimat();
  mettreAJourLCD();
  
  delay(500);
}

// ========== FEEDING CONTROL ==========
void verifierRepas(DateTime maintenant) {
  for (int i = 0; i < sizeof(heuresRepas)/sizeof(heuresRepas[0]); i++) {
    if (maintenant.hour() == heuresRepas[i] && maintenant.minute() == minutesRepas[i] && !repasServi[i]) {
      servoNourriture.write(ANGLE_OUVERT_NOURRITURE);
      delay(DUREE_NOURRITURE);
      servoNourriture.write(ANGLE_FERME_NOURRITURE);
      
      repasServi[i] = true;
      dernierRepas = millis();
    }
    
    if ((maintenant.hour() > heuresRepas[i]) || 
        (maintenant.hour() == heuresRepas[i] && maintenant.minute() > minutesRepas[i] + 1)) {
      if (repasServi[i]) {
        repasServi[i] = false;
      }
    }
  }
}

// ========== WATER CONTROL ==========
void controlerEau() {
  int niveauEau = analogRead(CAPTEUR_EAU);
  
  if (niveauEau < SEUIL_EAU) {
    if (!pompeActive) {
      digitalWrite(RELAIS_POMPE, HIGH);
      pompeActive = true;
      tempsDemarrePompe = millis();
    }
  } else {
    if (pompeActive) {
      digitalWrite(RELAIS_POMPE, LOW);
      pompeActive = false;
    }
  }
  
  if (pompeActive && (millis() - tempsDemarrePompe > dureeMaxPompe)) {
    digitalWrite(RELAIS_POMPE, LOW);
    pompeActive = false;
  }
}

// ========== DOOR CONTROL ==========
void controlerPorte() {
  int lumiere = analogRead(CAPTEUR_LUMIERE);
  
  if (lumiere > SEUIL_LUMIERE && !porteOuverte) {
    servoPorte.write(ANGLE_OUVERT_PORTE);
    porteOuverte = true;
    predatorDetected = false; 
  }
  else if (lumiere <= SEUIL_LUMIERE && porteOuverte) {
    servoPorte.write(ANGLE_FERME_PORTE);
    porteOuverte = false;
  }
  
  if (!porteOuverte) {
    int distance = obtenirDistance();
    if (distance > 0 && distance < DISTANCE_PREDATEUR) {
      predatorDetected = true;
    }else{
            predatorDetected = false;
    }
  }
}

// ========== ULTRASONIC SENSOR ==========
int obtenirDistance() {
  digitalWrite(TRIG_ULTRASON, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_ULTRASON, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_ULTRASON, LOW);
  
  long duree = pulseIn(ECHO_ULTRASON, HIGH, 30000);
  return duree * 0.0343 / 2;
}

// ========== CLIMATE CONTROL ==========
void controlerClimat() {
  float humidite = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  if (isnan(humidite) || isnan(temperature)) {
    return;
  }
  
  if (isnan(humidite) || isnan(temperature)) {
    return;
  }
  
  if (temperature >= SEUIL_TEMP) {
    digitalWrite(RELAIS_CHAUFFAGE, LOW);  
    digitalWrite(VENTILATEUR, HIGH);       
  } else {
    if (humidite > HUMID_HAUT) {
      digitalWrite(VENTILATEUR, HIGH);     
      digitalWrite(RELAIS_CHAUFFAGE, HIGH); 
    } else {
      digitalWrite(RELAIS_CHAUFFAGE, HIGH);
      digitalWrite(VENTILATEUR, LOW);      
    }
  } 
}

// ========== LCD DISPLAY ==========
void mettreAJourLCD() {
  lcd.clear();
    if (predatorDetected) {
    lcd.setCursor(0, 0);
    lcd.print("ALERT!");
    lcd.setCursor(0, 1);
    lcd.print("Danger!");
    return;  
  }
  
  float humidite = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print("c");
  
  lcd.print("H:");
  lcd.print(humidite);
  lcd.print("%");
  
  lcd.setCursor(0, 1);
  
  bool chauffageOn = false;
  bool ventilOn = false;
  
  if (temperature >= SEUIL_TEMP) {
    chauffageOn = false;
    ventilOn = true;
    lcd.print("Fan:ON");
  } 
  else {
    if (humidite > HUMID_HAUT) {
      ventilOn = true;
      chauffageOn = true;
      lcd.print("Heat&Fan:ON");
    } 
    else {
      lcd.print("Heat:ON");
    }
  }

  Serial.print("T:");
  Serial.print(temperature);
  Serial.print(" H:");
  Serial.println(humidite);
}