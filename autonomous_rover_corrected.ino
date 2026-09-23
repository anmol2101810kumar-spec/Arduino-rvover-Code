#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define ENA 5
#define ENB 6
#define IN1 7
#define IN2 8
#define IN3 9
#define IN4 10

#define TRIG_PIN 12
#define ECHO_PIN 11

#define SERVO_PIN 3
#define BUZZER_PIN 4

#define FRONT_LEFT_LED  A0
#define FRONT_RIGHT_LED A1
#define REAR_LEFT_LED   A2
#define REAR_RIGHT_LED  A3

#define MAX_SPEED 220
#define MIN_SPEED 75
#define BACK_SPEED 120
#define TURN_SPEED 175

#define STOP_DISTANCE 18
#define SLOW_DISTANCE 75

Servo scannerServo;

void setup() {
  Serial.begin(9600);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(FRONT_LEFT_LED, OUTPUT);
  pinMode(FRONT_RIGHT_LED, OUTPUT);
  pinMode(REAR_LEFT_LED, OUTPUT);
  pinMode(REAR_RIGHT_LED, OUTPUT);

  allLEDsOff();

  scannerServo.attach(SERVO_PIN);
  scannerServo.write(90);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED ERROR");
    while (1);
  }

  stopMotors();
  showHappyFace();
  delay(1000);
}

void loop() {
  int distance = getDistance();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > SLOW_DISTANCE) {
    showHappyFace();
    rearLEDsOff();
    moveForward(MAX_SPEED);
  } 
  else if (distance > STOP_DISTANCE) {
    int speedValue = map(distance, STOP_DISTANCE, SLOW_DISTANCE,
                          MIN_SPEED, MAX_SPEED);
    speedValue = constrain(speedValue, MIN_SPEED, MAX_SPEED);

    showCarefulFace();
    rearLEDsOff();
    moveForward(speedValue);
  } 
  else {
    stopMotors();
    showShockedFace();
    rearLEDsOn();
    delay(300);
    obstacleAvoidance();
  }

  delay(50);
}

int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) return 200;

  int distance = duration * 0.0343 / 2;

  if (distance <= 0 || distance > 200) return 200;

  return distance;
}

void moveForward(int speedValue) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
}

void moveBackward(int speedValue) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  rearLEDsOn();
}

void obstacleAvoidance() {
  stopMotors();

  scannerServo.write(150);
  delay(500);
  int leftDistance = getDistance();

  scannerServo.write(30);
  delay(500);
  int rightDistance = getDistance();

  scannerServo.write(90);
  delay(200);

  Serial.print("Left: ");
  Serial.println(leftDistance);
  Serial.print("Right: ");
  Serial.println(rightDistance);

  if (leftDistance > rightDistance) {
    reverseBeforeTurn();
    turnLeft();
  } else {
    reverseBeforeTurn();
    turnRight();
  }
}

void reverseBeforeTurn() {
  showReverseFace();
  rearLEDsOn();

  beepShort();

  moveBackward(BACK_SPEED);
  delay(450);

  stopMotors();
  delay(100);

  beepShort();
  delay(150);
}

void turnLeft() {
  showTurnLeft();

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, TURN_SPEED);
  analogWrite(ENB, TURN_SPEED);

  for (int i = 0; i < 8; i++) {
    leftLEDsBlink();
    delay(80);
  }

  stopMotors();
}

void turnRight() {
  showTurnRight();

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, TURN_SPEED);
  analogWrite(ENB, TURN_SPEED);

  for (int i = 0; i < 8; i++) {
    rightLEDsBlink();
    delay(80);
  }

  stopMotors();
}

void beepShort() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(120);
  digitalWrite(BUZZER_PIN, LOW);
  delay(100);
}

void allLEDsOff() {
  digitalWrite(FRONT_LEFT_LED, LOW);
  digitalWrite(FRONT_RIGHT_LED, LOW);
  digitalWrite(REAR_LEFT_LED, LOW);
  digitalWrite(REAR_RIGHT_LED, LOW);
}

void rearLEDsOn() {
  digitalWrite(REAR_LEFT_LED, HIGH);
  digitalWrite(REAR_RIGHT_LED, HIGH);
}

void rearLEDsOff() {
  digitalWrite(REAR_LEFT_LED, LOW);
  digitalWrite(REAR_RIGHT_LED, LOW);
}

void leftLEDsBlink() {
  digitalWrite(FRONT_LEFT_LED, HIGH);
  digitalWrite(REAR_LEFT_LED, HIGH);
  digitalWrite(FRONT_RIGHT_LED, LOW);
  digitalWrite(REAR_RIGHT_LED, LOW);
  delay(80);
  digitalWrite(FRONT_LEFT_LED, LOW);
  digitalWrite(REAR_LEFT_LED, LOW);
}

void rightLEDsBlink() {
  digitalWrite(FRONT_RIGHT_LED, HIGH);
  digitalWrite(REAR_RIGHT_LED, HIGH);
  digitalWrite(FRONT_LEFT_LED, LOW);
  digitalWrite(REAR_LEFT_LED, LOW);
  delay(80);
  digitalWrite(FRONT_RIGHT_LED, LOW);
  digitalWrite(REAR_RIGHT_LED, LOW);
}

void showHappyFace() {
  display.clearDisplay();
  display.drawCircle(64, 32, 28, WHITE);
  display.fillCircle(54, 27, 3, WHITE);
  display.fillCircle(74, 27, 3, WHITE);
  display.drawLine(52, 40, 56, 43, WHITE);
  display.drawLine(56, 43, 60, 45, WHITE);
  display.drawLine(60, 45, 64, 46, WHITE);
  display.drawLine(64, 46, 68, 45, WHITE);
  display.drawLine(68, 45, 72, 43, WHITE);
  display.drawLine(72, 43, 76, 40, WHITE);
  display.display();
}

void showCarefulFace() {
  display.clearDisplay();
  display.drawCircle(64, 32, 28, WHITE);
  display.fillCircle(54, 27, 3, WHITE);
  display.fillCircle(74, 27, 3, WHITE);
  display.drawLine(53, 45, 58, 42, WHITE);
  display.drawLine(58, 42, 64, 41, WHITE);
  display.drawLine(64, 41, 70, 42, WHITE);
  display.drawLine(70, 42, 75, 45, WHITE);
  display.drawCircle(88, 22, 3, WHITE);
  display.drawLine(88, 18, 88, 13, WHITE);
  display.display();
}

void showShockedFace() {
  display.clearDisplay();
  display.drawCircle(64, 32, 28, WHITE);
  display.drawCircle(54, 27, 5, WHITE);
  display.drawCircle(74, 27, 5, WHITE);
  display.fillCircle(54, 27, 2, WHITE);
  display.fillCircle(74, 27, 2, WHITE);
  display.drawCircle(64, 45, 7, WHITE);
  display.display();
}

void showReverseFace() {
  display.clearDisplay();
  display.drawCircle(64, 32, 28, WHITE);
  display.fillCircle(54, 27, 3, WHITE);
  display.fillCircle(74, 27, 3, WHITE);
  display.drawLine(54, 44, 60, 41, WHITE);
  display.drawLine(60, 41, 68, 41, WHITE);
  display.drawLine(68, 41, 74, 44, WHITE);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(45, 2);
  display.print("BACK");
  display.display();
}

void showTurnLeft() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(25, 20);
  display.print("<");
  display.setCursor(55, 20);
  display.print("LEFT");
  display.display();
}

void showTurnRight() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(55, 20);
  display.print("RIGHT");
  display.setCursor(105, 20);
  display.print(">");
  display.display();
}
