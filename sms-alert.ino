#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// --- Configuration ---
#define DHTPIN 2
#define DHTTYPE DHT22
#define SOIL_PIN A0
#define PHONE_NUMBER "09603496584" // Replace sa inyong mobile number, para mka receive ug SMS

// Pins for SIM800L (RX, TX) 
// Arduino D10 (RX) -> SIM800L TX
// Arduino D11 (TX) -> SIM800L RX
SoftwareSerial sim800l(10, 11); 

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Thresholds for Alerts
float tempLimit = 35.0; 
int moistureLimit = 30; // Percent

unsigned long lastSMSMillis = 0;
const unsigned long smsInterval = 300000; // 5 min lockout to prevent spam

void setup() {
  Serial.begin(9600);
  sim800l.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();
  
  delay(5000); // Wait for SIM800L to find network
  Serial.println("System Ready");
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();//Celsius
  int soilValue = analogRead(SOIL_PIN);
  int soilPercent = map(soilValue, 1023, 0, 0, 100);

  // Update LCD
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(t, 1);
  lcd.print("C");
  lcd.print(" H:");lcd.print(h, 1);
  lcd.setCursor(0, 1);
  lcd.print("Soil:"); lcd.print(soilPercent); lcd.print("%");

  // Check for Critical Conditions
  if (t > tempLimit || soilPercent < moistureLimit) {
    if (millis() - lastSMSMillis > smsInterval) {
      sendSMS(t, h, soilPercent);
      lastSMSMillis = millis();
    }
  }

  delay(2000);
}

void sendSMS(float temp, float hum, int soil) {
  Serial.println("Sending SMS...");
  
  sim800l.println("AT+CMGF=1"); // Set SMS to text mode
  delay(100);
  sim800l.print("AT+CMGS=\"");
  sim800l.print("09603496584");
  sim800l.println("\"");
  delay(100);
  
  sim800l.print("ALERT! Greenhouse Status");
  sim800l.print("\nTemp: "); sim800l.print(temp);
  sim800l.print("C\nHum: "); sim800l.print(hum);
  sim800l.print("%\nSoil: "); sim800l.print(soil);
  sim800l.print("%\nCheck system!");
  
  delay(100);
  sim800l.write(26); // ASCII code for CTRL+Z to send
  delay(1000);
  Serial.println("SMS Sent.");
}
