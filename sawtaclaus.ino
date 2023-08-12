#include <Servo.h> // https://github.com/RoboticsBrno/ServoESP32/
#include <Ps3Controller.h> // https://github.com/jvpernis/esp32-ps3
//#include <analogWrite.h> // https://github.com/ERROPiX/ESP32_AnalogWrite, causa errores de compilación pero arreglable(?)
// on second look, parece que no es necesario incluirla??? wtf???

class Rueda {
  public:
    int fwdPin, backwdPin;
    int fwdCH, backwdCH;
    String name;
    Rueda(int _fwdPin, int _backwdPin, int _fwdCH, int _backwdCH, String _name) {
      pinMode(fwdPin, OUTPUT);
      pinMode(backwdPin, OUTPUT);
      fwdPin = _fwdPin;
      backwdPin = _backwdPin;
      name = _name;
    }
    void desplazar(int spd) {  // spd: -255 ~ 255

      spd = map(spd, -128, 128, -255, 255);
      spd = min(255, spd);
      spd = max(-255, spd);

      if (spd) {
        Serial.print("Moviendo rueda: ");
        Serial.print(name);
        Serial.print(" a velocidad: ");
        Serial.print(spd, DEC);
        Serial.println();
      }
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
    int pin, channel;
    Servo servo;
    int minMus, maxMus, stopMus;  // (microSeconds)
    Arma(int _pin, int _channel, int _minMus, int _maxMus, int _stopMus, int _freq) {
      pinMode(pin, OUTPUT);
      pin = _pin;
      minMus = _minMus;
      maxMus = _maxMus;
      stopMus = _stopMus;
      channel = _channel;
      lock = true;

      ledcSetup(channel, _freq, 8);

      servo.attach(pin, channel);
      servo.writeMicroseconds(stopMus);
    }
    void activar(int spd) {  //spd: 0 ~ 255
      if (!lock) {
        spd = map(spd, 0, 255, minMus, maxMus);
        spd = min(maxMus, spd);
        spd = max(minMus, spd);

        if (spd != stopMus) {
          Serial.print("Arma activada a velocidad: ");
          Serial.print(spd, DEC);
          Serial.println();
        }

        servo.writeMicroseconds(spd);
      } else {
        if (spd)
          Serial.println("El arma no se activó porque estaba lockeada");
      }
    }
    void lockOn() {
      Serial.println("Lock del arma activado");
      lock = true;
      this->stop();
    }
    void lockOff() {
      Serial.println("Lock del arma desactivado");
      lock = false;
    }
    void stop(){
      servo.writeMicroseconds(stopMus);
      Serial.println("Arma detenida");
    }
  private:
    bool lock;
};

// CONSTANTES
const int PWM_FREQ = 50;
const int MOVS[4] = { 16, 17, 19, 18 };  //RF, RB, LF, LB ->  IN4, IN3, IN2, IN1
const int ARMA = 15;
const int minArma = 1300, maxArma = 2000, stopArma = 1300;  // calibración manual
Rueda ruedaDer = Rueda(MOVS[0], MOVS[1], 0, 1, "Derecha"), ruedaIzq = Rueda(MOVS[2], MOVS[3], 2, 3, "Izquierda");
Arma arma = Arma(ARMA, 4, minArma, maxArma, stopArma, PWM_FREQ);
// CONSTANTES

void onConnect() {
  Serial.println("Connected.");
}

void controles() {
  sticks_auto();
  controles_arma();
}

void sticks_tanque() {
  int der = 0, izq = 0, deadzone = 15;
  // ------------------------------ Sticks (Analógicos) ------------------------------
  izq = -Ps3.data.analog.stick.ly;
  izq = (abs(izq) > deadzone) ? izq : 0;
  der = -Ps3.data.analog.stick.ry;
  der = (abs(der) > deadzone) ? der : 0;
  
  ruedaIzq.desplazar(izq);
  ruedaDer.desplazar(der);
}

void sticks_auto() {
  int v = 0, omega = 0, deadzone = 15, der = 0, izq = 0;
  // ------------------------------ Sticks (Analógicos) ------------------------------
  omega = Ps3.data.analog.stick.lx;
  omega = (abs(omega) > deadzone) ? omega : 0;
  v = -Ps3.data.analog.stick.ry;
  v = (abs(v) > deadzone) ? v : 0;
  
  // Cinemática inversa (por definir)
  float om_rate = 1, radius = 1;
  izq = (v - 0.5 * omega * om_rate) / radius;
  der = (v + 0.5 * omega * om_rate) / radius;

  ruedaIzq.desplazar(int(izq));
  ruedaDer.desplazar(int(der));
}

void controles_arma() {
  // ------------------------------ Triggers (Analógicos) ------------------------------
  arma.activar(Ps3.data.analog.button.r2);
  // ------------------------------ Pad Derecho (Digital) ------------------------------
  if (Ps3.event.button_down.cross) {
    arma.lockOn();
  }
  if (Ps3.event.button_down.circle) {
    arma.lockOff();
  }
}

void btconnect() {
  Ps3.begin("00:22:02:01:11:0e");
}

void setup() {
  Serial.begin(115200);

  Ps3.attach(controles);
  Ps3.attachOnConnect(onConnect);
  btconnect();

  Serial.println("Ready.");
}

void loop() {
  if (!Ps3.isConnected())
    btconnect();
  delay(2000);
}
