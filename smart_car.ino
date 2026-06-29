#include <WiFi.h>
#include <WebServer.h>


// ===== MOTOR KANAN A =====
int enable = 21;
int input1 = 4;
int input2 = 5;


// ===== MOTOR KIRI B =====
int enable_01 = 22;
int input_01 = 19;
int input_02 = 20;


// ===== PWM =====
#define PWM_FREQ 1000
#define PWM_RES 8


// ===== SENSOR JARAK =====
#include <Wire.h>
#include <VL53L0X.h>


VL53L0X sensor;
int jarak_cm = 0;


// ===== WIFI =====
const char* ssid = "ESP32_ROBOT";
const char* password = "12345678";
WebServer server(80);


// ===== GEAR SYSTEM =====
int gear = 3;
int currentSpeed = 150;  // default gear


// =============================
// UPDATE SPEED BERDASARKAN GEAR
// =============================
void updateSpeed() {
  switch (gear) {
    case 1: currentSpeed = 100; break;
    case 2: currentSpeed = 130; break;
    case 3: currentSpeed = 160; break;
    case 4: currentSpeed = 190; break;
    case 5: currentSpeed = 220; break;
    case 6: currentSpeed = 255; break;
  }


  Serial.print("Gear: ");
  Serial.print(gear);
  Serial.print(" | Speed: ");
  Serial.println(currentSpeed);
}


void setup() {


  Serial.begin(115200);
  delay(3000);


  Wire.begin(15, 18);
  sensor.setTimeout(500);


  if (!sensor.init()) {
    Serial.println("VL53L0X tidak terdeteksi");
    while (1)
      ;
  }


  sensor.startContinuous();


  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.println(WiFi.softAPIP());


  // ===== GERAK =====
  server.on("/forward", []() {
    // CEK SENSOR
    if (jarak_cm < 7 && jarak_cm > 0) {
      stopMotor();
      server.send(200, "text/plain", "Terlalu Dekat! Tidak Bisa Forward");
    } else {
      maju(currentSpeed);
      server.send(200, "text/plain", "Moving Forward");
    }
  });


  server.on("/backward", []() {
    mundur(currentSpeed);
    server.send(200, "text/plain", "Moving Backward");
  });


  server.on("/stop", []() {
    stopMotor();
    server.send(200, "text/plain", "Stopped");
  });


  server.on("/right", []() {
    belok_kanan(currentSpeed);
    server.send(200, "text/plain", "Turning Right");
  });


  server.on("/left", []() {
    digitalWrite(input1, HIGH);
    digitalWrite(input2, LOW);


    digitalWrite(input_01, LOW);
    digitalWrite(input_02, LOW);


    ledcWrite(enable, currentSpeed);
    ledcWrite(enable_01, 0);


    server.send(200, "text/plain", "Turning Left");
  });


  // ===== GEAR 1–6 =====
  server.on("/gear1", []() {
    gear = 1;
    updateSpeed();
    server.send(200, "text/plain", "Gear 1");
  });
  server.on("/gear2", []() {
    gear = 2;
    updateSpeed();
    server.send(200, "text/plain", "Gear 2");
  });
  server.on("/gear3", []() {
    gear = 3;
    updateSpeed();
    server.send(200, "text/plain", "Gear 3");
  });
  server.on("/gear4", []() {
    gear = 4;
    updateSpeed();
    server.send(200, "text/plain", "Gear 4");
  });
  server.on("/gear5", []() {
    gear = 5;
    updateSpeed();
    server.send(200, "text/plain", "Gear 5");
  });
  server.on("/gear6", []() {
    gear = 6;
    updateSpeed();
    server.send(200, "text/plain", "Gear 6");
  });


  server.begin();


  pinMode(input1, OUTPUT);
  pinMode(input2, OUTPUT);
  pinMode(input_01, OUTPUT);
  pinMode(input_02, OUTPUT);


  ledcAttach(enable, PWM_FREQ, PWM_RES);
  ledcAttach(enable_01, PWM_FREQ, PWM_RES);


  updateSpeed();
}


void loop() {


  int jarak_mm = sensor.readRangeContinuousMillimeters();
  jarak_cm = jarak_mm / 10;


  server.handleClient();


  if (!sensor.timeoutOccurred()) {
    Serial.print("Jarak: ");
    Serial.print(jarak_cm);
    Serial.println(" cm");
  }


  delay(10);
}


// =============================
// ===== MOTOR FUNCTIONS ======
// =============================


void maju(int speed) {


  digitalWrite(input_01, HIGH);
  digitalWrite(input_02, LOW);


  digitalWrite(input1, HIGH);
  digitalWrite(input2, LOW);


  ledcWrite(enable_01, speed);
  ledcWrite(enable, speed);
}


void mundur(int speed) {


  digitalWrite(input1, LOW);
  digitalWrite(input2, HIGH);


  digitalWrite(input_01, LOW);
  digitalWrite(input_02, HIGH);


  ledcWrite(enable, speed);
  ledcWrite(enable_01, speed);
}


void stopMotor() {


  ledcWrite(enable_01, 0);
  ledcWrite(enable, 0);


  digitalWrite(input1, LOW);
  digitalWrite(input2, LOW);


  digitalWrite(input_01, LOW);
  digitalWrite(input_02, LOW);
}


void belok_kanan(int speed) {


  digitalWrite(input_01, HIGH);
  digitalWrite(input_02, LOW);


  digitalWrite(input1, LOW);
  digitalWrite(input2, LOW);


  ledcWrite(enable_01, speed);
  ledcWrite(enable, 0);
}
