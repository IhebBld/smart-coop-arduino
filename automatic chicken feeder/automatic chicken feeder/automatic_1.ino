//smartkembo discovery science
#include<LiquidCrystal.h>
#include<Servo.h>
#include<RTClib.h>
RTC_DS1307 rtc;
int echoPin = 6;
int trigPin= 5;
int waterPump = 4;
int relayPin = 2;
int lm35Pin = A3;
int temp_adc_val;
float temp_val;

const int waterSensor = A0;
int waterVal =0;
char daysOfTheWeek[7][12]={"Sun","mon","Tue","wed","Thu","Fri","Sat"};
LiquidCrystal lcd(12,11,10,9,8,7);
Servo myservo;

void setup()
{
  rtc.begin();
  if(! rtc.isrunning())
  {
    Serial.println("RTC is Not running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));    
  }
  lcd.begin(16,2);
  myservo.attach(3);
  myservo.write(0);
  pinMode(echoPin,INPUT);
  pinMode(trigPin,OUTPUT);
  pinMode(waterPump,OUTPUT);
  pinMode(waterSensor,INPUT);
  pinMode(relayPin,OUTPUT);
  pinMode(lm35Pin,INPUT);
  digitalWrite(relayPin,HIGH);
}

void loop()
{

  long duration,distance;
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  duration = pulseIn(echoPin,HIGH);
  distance = (duration/2)/29.412;
  waterVal = analogRead(waterSensor);
  
  temp_adc_val = analogRead(lm35Pin);
  temp_val = (temp_adc_val*4.88);
  temp_val = (temp_val/10);


  DateTime now = rtc.now();
  Serial.print(now.year(),DEC);
  Serial.print("/");
  Serial.print(now.month(),DEC);
  Serial.print("(");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(")");
  lcd.print(now.hour(),DEC);
  lcd.print(':');
  lcd.print(now.minute(),DEC);
  lcd.print(':');
  lcd.print(now.second(),DEC);
  if(now.hour()==11 &&now.minute()==24)
  {
    digitalWrite(waterPump,LOW);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    lcd.print(")");
    lcd.print(now.hour(),DEC);
    lcd.print(':');
    lcd.print(now.minute(),DEC);
    lcd.print(':');
    lcd.print(now.second(),DEC);
    lcd.println();
    lcd.setCursor(0,1);
    lcd.print("feeding is ON");
    if(distance > 20 )
   {
    myservo.write(180);
   }
   else
   {
    myservo.write(0);
   }
  }
  else if(now.hour()==11 && now.minute()==26)
  {
   digitalWrite(waterPump,LOW);
    lcd.setCursor(0,0);
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    lcd.print(")");
    lcd.print(now.hour(),DEC);
    lcd.print(":");
    lcd.print(now.minute(),DEC);
    lcd.print(":");
    lcd.print(now.second(),DEC);
    lcd.println();
    lcd.setCursor(0,1);
    lcd.print("Feeding is ON");
     
     if(distance > 20 )
   {
    myservo.write(180);
   }
   else
   {
    myservo.write(0);
   }
  }
  else
  {
    myservo.write(0);
    digitalWrite(waterPump,LOW);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    lcd.print(") ");
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    lcd.print(now.second(), DEC);
    lcd.println();
    lcd.setCursor(0,1);
    lcd.print("Feeding is OFF");
    temper();
   if(waterVal>500)
  {
    digitalWrite(waterPump,LOW);
  }
  else
  {
    digitalWrite(waterPump,HIGH);
  }
 }

 temper();
 delay(100);
}


void temper()
{
  if(temp_val >= 30){
    lcd.setCursor(0,1);
    lcd.print("Temperature: ");
    lcd.print(temp_val);
    lcd.print("'C");
    delay(1000);
    digitalWrite(relayPin,LOW);
    lcd.clear();
     }
}
