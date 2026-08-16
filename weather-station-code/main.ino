#define TINY_GSM_MODEM_SIM7000

#include <TinyGsmClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

// lilygo pin definitions
#define UART_BAUD 115200
#define PIN_DTR   25
#define PIN_TX    27
#define PIN_RX    26
#define PWR_PIN   4

HardwareSerial modemSerial(1);
TinyGsm modem(modemSerial);

const char TARGET_PHONE_NUMBER[] = "+xxxxxxxxxxx";
const char apn[] = "hologram";

// dht11 sensor (temperature & humidity)
#define DHTPIN 33
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// bmp180 (air pressure)
Adafruit_BMP085 bmp;

// adafruit anemometer (wind speed)
#define ANEMOMETER_PIN 34

// gps
float latitude = xxxxx;
float longitude = yyyyy;

void setup() {

  Serial.begin(115200);

  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(300);
  digitalWrite(PWR_PIN, LOW);

  modemSerial.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  delay(6000);

  Serial.println("Initializing modem");

  if (!modem.restart()) {
    Serial.println("Failed to restart");
    return;
  }

  Serial.print("Connecting to ");
  Serial.println(apn);

  if (!modem.gprsConnect(apn, "", "")) {
    Serial.println("Failed to connect");
    return;
  }

  Serial.println("Network Connected");

  // initialize sensors
  dht.begin();

  Wire.begin(21,22);

  if (!bmp.begin()) {
    Serial.println("BMP180 not found");
  }
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  float pressure = bmp.readPressure() / 100.0;   // Pa -> hPa

  // battery voltage placeholder
  float voltage = 4.20;

  // read anemometer ADC
  int adc = analogRead(ANEMOMETER_PIN);

  // convert ADC to voltage
  float sensorVoltage = adc * (3.3 / 4095.0);

  // replace with adafruit's calibration curve
  float wind_speed = sensorVoltage * 9.0;

  // build json payload
  StaticJsonDocument<200> doc;

  doc["t"]   = temperature;
  doc["h"]   = humidity;
  doc["v"]   = voltage;
  doc["p"]   = pressure;
  doc["w"]   = wind_speed;
  doc["lat"] = latitude;
  doc["lng"] = longitude;

  String payload;
  serializeJson(doc, payload);

  Serial.println(payload);

  if(modem.sendSMS(TARGET_PHONE_NUMBER, payload))
      Serial.println("SMS sent");
  else
      Serial.println("SMS failed");

  delay(1800000);   // 15 minutes
}
