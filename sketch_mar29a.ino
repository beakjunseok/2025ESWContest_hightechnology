#include <Wire.h>
#include <MPU6050_light.h>
#include <Servo.h>

MPU6050 mpu(Wire);
Servo lidServo;
#define SERVO_PIN 10

float calibrationX = 0;
float calibrationY = 0;


bool isLidClosed = true;    
bool wasOverturned = false;
bool fastLock = false;

int openAngle = 0;
int closeAngle = 30;

float prevAngleX = 0;
float prevAngleY = 0;

void setLid(int targetAngle) {
  static int lastAngle = -1;
  if (lastAngle != targetAngle) {
    lidServo.write(targetAngle);
    lastAngle = targetAngle;
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);

  mpu.begin();
  mpu.calcGyroOffsets();

  lidServo.attach(SERVO_PIN);
  setLid(closeAngle);

  mpu.update();
  calibrationX = mpu.getAngleY();
  calibrationY = mpu.getAngleX();

  prevAngleX = calibrationX;
  prevAngleY = calibrationY;

  Serial.println("시작 완료");
}

void loop() {
  mpu.update();

  float angleX = mpu.getAngleY() - calibrationX;
  float angleY = mpu.getAngleX() - calibrationY;


  bool isRecovered = angleX > -12 && angleX < 12 && angleY > -12 && angleY < 12;


  float deltaX = angleX - prevAngleX;
  float deltaY = angleY - prevAngleY;
  bool fastOverturn = abs(deltaX) > 15 || abs(deltaY) > 15;
  if (fastOverturn) {
    fastLock = true;
    Serial.println("급격한 기울기 감지 → fastLock 활성화");
  }
  if (fastLock && isRecovered) {
    fastLock = false;
    Serial.println("정상 복귀 → fastLock 해제");
  }

  prevAngleX = angleX;
  prevAngleY = angleY;


  bool angleOverturn = angleY > 120 || angleX >= 30 || angleX <= -30 || angleY <= -30;


  bool isPouring = !isRecovered && !angleOverturn && !fastLock && angleY > 0;


  if (angleOverturn) {
    setLid(closeAngle);
    isLidClosed = true;
    wasOverturned = true;
    Serial.println("전복 상태 → 닫힘");
  }
  else if (fastLock) {
    setLid(closeAngle);
    isLidClosed = true;
    Serial.println("fastLock → 닫힘 유지");
  }
  else if (angleY <= 0) {
    setLid(closeAngle);
    isLidClosed = true;
    Serial.println("입구 방향 아님 → 닫힘");
  }
  else if (isPouring) {
    setLid(openAngle);
    isLidClosed = false;
    wasOverturned = false;
    Serial.println("따르는 중 → 열림");
  }
  else if (isRecovered) { 
    setLid(closeAngle);
    isLidClosed = true;
    wasOverturned = false;
  }


  Serial.print("X: "); Serial.print(angleX);
  Serial.print(" | Y: "); Serial.print(angleY);
  Serial.print(" | Closed: "); Serial.print(isLidClosed);
  Serial.print(" | Overturned: "); Serial.print(wasOverturned);
  Serial.print(" | fastLock: "); Serial.print(fastLock);
  Serial.print(" | Pouring: "); Serial.println(isPouring);

  delay(50);
}


