//temperature 

#include <LiquidCrystal.h>
#include <DHT.h>

#define DHTPIN 13          
#define DHTTYPE DHT11      
#define RELAY_PIN 2        
#define FAN_PIN 3          

#define TEMP_THRESHOLD 30  
#define HUMID_LOW 30        
#define HUMID_HIGH 60      

DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

void setup() {
  Serial.begin(9600);
  Serial.println("Starting DHT11 temperature and humidity monitor");

  dht.begin();
 
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, HIGH); 
  digitalWrite(FAN_PIN, LOW);  
 
  lcd.begin(16, 2); 
 
  lcd.clear();
  lcd.print("Climate Control");
  lcd.setCursor(0, 1);
  lcd.print("System Starting");
 
  delay(2000);
  lcd.clear();
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
 
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");
 
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    lcd.setCursor(0, 1);
    lcd.print("Check Wiring");
    delay(2000);
    return;
  }
 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print("C ");
  
  lcd.print("H:");
  lcd.print(humidity);
  lcd.print("%");
  
  lcd.setCursor(0, 1);
  
  bool heaterOn = false;
  bool fanOn = false;
  
  if (temperature >= TEMP_THRESHOLD) {
    digitalWrite(RELAY_PIN, LOW);  
    digitalWrite(FAN_PIN, HIGH); 
    heaterOn = true;
    fanOn = true;
    lcd.print("Heat&Fan: ON");
  } 
  else {
    digitalWrite(RELAY_PIN, HIGH); 
    
    if (humidity > HUMID_HIGH) {
      digitalWrite(FAN_PIN, HIGH); 
      fanOn = true;
      lcd.print("HumidHigh,Fan:ON");
    } 
    else {
      digitalWrite(FAN_PIN, LOW); 
      lcd.print("All Systems: OFF");
    }
  }
  
  Serial.print("Heater: ");
  Serial.print(heaterOn ? "ON" : "OFF");
  Serial.print(", Fan: ");
  Serial.println(fanOn ? "ON" : "OFF");
 
  delay(2000);
}