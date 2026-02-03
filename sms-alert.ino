#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// --- Configuration ---
#define DHTPIN 2
#define DHTTYPE DHT22
#define SOIL_PIN A0
#define PHONE_NUMBER "09603496584" 

SoftwareSerial sim800l(10, 11); 
DHT dht(DHTPIN, DHTTYPE);

// If 0x27 doesn't work, try 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

float tempLimit = 35.0; 
int moistureLimit = 30; 
unsigned long lastSMSMillis = 0;
const unsigned long smsInterval = 300000; 

void setup() {
  Serial.begin(9600);
  sim800l.begin(9600);
  dht.begin();
  
  // FIXED: Standard initialization sequence
  lcd.init();           
  lcd.backlight();
  lcd.clear();
  lcd.print("Initializing...");
  
  delay(2000); 
  Serial.println("System Ready");
  lcd.clear();
}

void loop() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int soilValue = analogRead(SOIL_PIN);
  int soilPercent = map(soilValue, 1023, 0, 0, 100);

  // Check if DHT sensor is returning valid data
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error   "); // Spaces clear old text
  } else {
    // Update LCD Row 1
    lcd.setCursor(0, 0);
    lcd.print("T:"); lcd.print(t, 1); lcd.print("C "); 
    lcd.print("H:"); lcd.print(h, 1); lcd.print("% ");
    
    // Update LCD Row 2
    lcd.setCursor(0, 1);
    lcd.print("Soil: "); lcd.print(soilPercent); lcd.print("%   ");
  }

  // Check for Critical Conditions
  if (!isnan(t) && (t > tempLimit || soilPercent < moistureLimit)) {
    if (millis() - lastSMSMillis > smsInterval) {
      sendSMS(t, h, soilPercent);
      lastSMSMillis = millis();
    }
  }

  delay(2000); // 2 second delay between readings
}

void sendSMS(float temp, float hum, int soil) {
  Serial.println("Sending SMS...");
  lcd.setCursor(0,1);
  lcd.print("Sending SMS...  ");

  sim800l.println("AT+CMGF=1"); 
  delay(500);
  sim800l.print("AT+CMGS=\"");
  sim800l.print(PHONE_NUMBER);
  sim800l.println("\"");
  delay(500);
  
  sim800l.print("ALERT! Greenhouse Status");
  sim800l.print("\nTemp: "); sim800l.print(temp);
  sim800l.print("C\nHum: "); sim800l.print(hum);
  sim800l.print("%\nSoil: "); sim800l.print(soil);
  sim800l.print("%");
  
  delay(500);
  sim800l.write(26); // CTRL+Z
  delay(1000);
  Serial.println("SMS Sent.");
}