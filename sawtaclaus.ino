#include <ESP32Servo.h>
#include <analogWrite.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>
#include <Ps3Controller.h>

Serial.begin(9600)


  class Rueda {
public:
  int fwdPin, backwdPin;
  Rueda(int _fwdPin, int _backwdPin) {
    pinMode(fwdPin, OUTPUT);
    pinMode(backwdPin, OUTPUT);
    fwdPin = _fwdPin;
    backwdPin = _backwdPin;
  }
  void desp(int spd) {  // spd: -255 ~ 255
    spd = map(spd, -128, 127, -255, 255);
    spd = min(255, spd);
    spd = max(-255, spd);
    if (spd >= 0) {
      analogWrite(backwdPin, 0);
      analogWrite(fwdPin, spd);
    }
    if (spd <= 0) {
      analogWrite(fwdPin, 0);
      analogWrite(backwdPin, -spd);
    }
  }
};

class Arma {
public:
  int pin;
  Servo servo;
  int minMus, maxMus, stopMus;  // (microSeconds)
  bool lock;
  Arma(int _pin, int _minMus, int _maxMus, int _stopMus) {
    pinMode(pin, OUTPUT);
    pin = _pin;
    minMus = _minMus;
    maxMus = _maxMus;
    stopMus = _stopMus;
    servo.attach(pin);
    servo.writeMicroseconds(stopMus);
  }
  void activar(int spd) {  //spd: 0 ~ 100
    if (!lock) {
      spd = map(spd, 0, 100, minMus, maxMus);
      spd = min(maxMus, spd);
      spd = max(minMus, spd);
      servo.writeMicroseconds(spd);
    }
  }
  void lockOn() {
    lock = true;
  }
  void lockOff() {
    lock = false;
  }
};

const int MOVS[4] = { 11, 10, 9, 3 };  //RF, RB, LF, LB ->  IN4, IN3, IN2, IN1
const int LED = 4;
const int ARMA = 5;
const int minArma = 1300, maxArma = 2000, stopArma = 1300;  // calibración manual
const Rueda ruedaDer = Rueda(MOVS[0], MOVS[1]), ruedaIzq = Rueda(MOVS[2], MOVS[3]);
const Arma arma = Arma(ARMA, minArma, maxArma, stopArma);

void onConnect() {
  Serial.println("Connected.");
}

void controles() {
}

void setup() {
  Serial.begin(115200);

  Ps3.attach(controles);
  Ps3.attachOnConnect(onConnect);
  Ps3.begin("00:22:02:01:11:0e");

  Serial.println("Ready.");
}

void loop() {
}
