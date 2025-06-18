#include <LiquidCrystal_I2C.h>
#include <Servo.h>

Servo solarServo;
Servo solarServo1;

// Servos
const int servoPin = 3;
const int servoWip = 9;
const int pump = 13;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// Sensors
const int voltagePin = A6;  // Voltage sensor output
const int currentPin = A7;  // ACS712 output
const int ldrLeftPin = A2;
const int ldrRightPin = A3;

// Constants
int pos = 90;               // Initial servo position (middle)
const int threshold = 150;  // Light difference threshold to trigger movement
const int stepSize = 1;
int difference;
const float vRef = 3.3;
const float voltageSensorFactor = 5.14;
const float ACS712_SENSITIVITY = 0.066;  // V/A for ACS712 20A
float ACS712_ZERO_OFFSET = 2.110;
float actualVoltage;
float actualCurrent;
unsigned long startTime;
String valueV, valueA;

byte cellU[] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B00000,
  B11111,
  B11111,
  B11111
};
byte cellD[] = {
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
byte cellM[] = {
  B11111,
  B11111,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100,
  B01110
};
byte a[] = {
  B00000,
  B00000,
  B01110,
  B10001,
  B11111,
  B10001,
  B10001,
  B00000
};

void setup() {
  lcd.init();  // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();
  lcd.createChar(0, cellU);
  lcd.createChar(1, cellM);
  lcd.createChar(2, cellD);
  lcd.createChar(3, a);
  lcd.home();
  pinMode(13, OUTPUT);
  solarServo.attach(servoPin);
  solarServo1.attach(servoWip);
  solarServo1.write(0);
  delay(500);
}

void loop() {
  Wiping();
  intro();
  read();
  valueV = String(actualVoltage) + "v";
  valueA = String(actualCurrent);
  text(0, 1, valueV);
  text(6, 1, valueA);
  if (abs(difference) > threshold) {
    if (difference > 0 && pos < 180) {
      pos += stepSize;  // Move right
    } else if (difference < 0 && pos > 0) {
      pos -= stepSize;  // Move left
    }
    solarServo.write(pos);
    delay(50);  // Small delay for servo to move
  }
  delay(200);
}
void read() {
  long voltageSum = 0;
  long currentSum = 0;
  const int samples = 50;
  int ldrLeft = analogRead(ldrLeftPin);
  int ldrRight = analogRead(ldrRightPin);

  difference = ldrLeft - ldrRight;

  for (int i = 0; i < samples; i++) {
    voltageSum += analogRead(voltagePin);
    currentSum += analogRead(currentPin);
    delay(2);  // small delay between samples
  }

  float avgVoltageRaw = voltageSum / float(samples);
  float avgCurrentRaw = currentSum / float(samples);

  float sensorVoltage = (avgVoltageRaw * vRef) / 1023.0;
  actualVoltage = sensorVoltage * voltageSensorFactor;

  float sensorCurrentVoltage = (avgCurrentRaw * vRef) / 1023.0;
  actualCurrent = (sensorCurrentVoltage - ACS712_ZERO_OFFSET) / ACS712_SENSITIVITY;
}
void Wiping() {
  // Change Time 15000ms
  int pos;
  if (millis() - startTime < 15000) {  // Run for 15 seconds only
    for (pos = 0; pos <= 120; pos++) {
      solarServo1.write(pos);
      digitalWrite(pump, HIGH);
      delay(15);
    }
    for (pos = 120; pos >= 20; pos--) {
      solarServo1.write(pos);
      digitalWrite(pump, HIGH);
      delay(15);
    }
  } else {
    // Stop servo and pump after 15 seconds
    digitalWrite(pump, LOW);
    solarServo1.write(0);  // Move to resting position (optional)
    solarServo1.detach();
  }
}
void text(int x, int y, String a) {
  lcd.setCursor(x, y);
  lcd.print(a);
}
void icon(int x, int y, int z) {
  lcd.setCursor(x, y);
  lcd.write(z);
}
void intro() {
  text(0, 0, "SOLAR SYSTEM");
  icon(11, 1, 3);
  icon(13, 0, 0);
  icon(14, 0, 0);
  icon(15, 0, 0);
  icon(13, 1, 2);
  icon(14, 1, 1);
  icon(15, 1, 2);
}