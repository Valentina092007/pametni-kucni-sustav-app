#include <Servo.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define vent 6
#define led 7
#define senzor A0

DHT dht(DHTPIN, DHTTYPE);
Servo servovent;
SoftwareSerial BTSerial(10, 11);

const float temperaturniPrag = 28.0;
const float pragVlaznosti = 40.0;
const int pragSuhoce = 300;

unsigned long lastReport = 0;
const unsigned long reportInterval = 10000UL;

// Servo sweep timing
unsigned long lastServoMove = 0;
const unsigned long servoInterval = 10UL;
int currentAngle = 0;
int sweepDir = 1;
bool autoSweep = false;

bool soilAlertSent = false;

void setup() {
  Serial.begin(9600);
  BTSerial.begin(9600);
  dht.begin();

  servovent.attach(vent);
  servovent.write(0);

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  Serial.println("Sustav je inicijaliziran.");
}

void loop() {
  unsigned long now = millis();

  if (now - lastReport >= reportInterval) {
    lastReport = now;
    float temperatura = dht.readTemperature();
    float vlaznost = dht.readHumidity();
    int stanjeZemlje = analogRead(senzor);

    String th = "Temperatura:" + String(temperatura, 1)
               + " °C Vlažnost zraka:" + String(vlaznost, 1)
               + " % ";
    Serial.println(th);
    BTSerial.println(th);

    String soil = "Vlažnost zemlje:" + String(stanjeZemlje);
    Serial.println(soil);
    BTSerial.println(soil);

    if (temperatura >= temperaturniPrag) {
      BTSerial.println("Visoka temperatura! Uključite ventilator!");
    }
    if (vlaznost >= pragVlaznosti) {
      BTSerial.println("Visoka vlažnost zraka! Uključite ventilator");
    }
  }

  int stanjeZemlje = analogRead(senzor);
  if (stanjeZemlje < pragSuhoce) {
    if (!soilAlertSent) {
      BTSerial.println("Niska vlažnost zemlje! Morate zaliti biljke.");
      soilAlertSent = true;
    }
  } else {
    soilAlertSent = false;
  }

  if (BTSerial.available()) {
    String command = BTSerial.readStringUntil('\n');
    command.trim();
    Serial.print("Primljeni command: ");
    Serial.println(command);

    if (command.equalsIgnoreCase("ON")) {
      autoSweep = true;
      BTSerial.println("Ventilator je UKLJUČEN.");
      Serial.println("Ventilator je UKLJUČEN.");
    }
    else if (command.equalsIgnoreCase("OFF")) {
      autoSweep = false;
      currentAngle = 0;
      servovent.write(currentAngle);
      BTSerial.println("Ventilator je ISKLJUČEN.");
      Serial.println("Ventilator je ISKLJUČEN.");
    }
    else if (command.equalsIgnoreCase("UKLJUCI")) {
      digitalWrite(led, HIGH);
      BTSerial.println("Svjetlo UKLJUČENO");
    }
    else if (command.equalsIgnoreCase("ISKLJUCI")) {
      digitalWrite(led, LOW);
      BTSerial.println("Svjetlo ISKLJUČENO");
    }
    else {
      BTSerial.println("ERROR: Nepoznat command");
    }
  }

  if (autoSweep && (now - lastServoMove >= servoInterval)) {
    lastServoMove = now;
    currentAngle += sweepDir;
    if (currentAngle >= 180) {
      currentAngle = 180;
      sweepDir = -1;
    } else if (currentAngle <= 0) {
      currentAngle = 0;
      sweepDir = 1;
    }
    servovent.write(currentAngle);
  }

  delay(10);
}
