#define BLYNK_PRINT Serial

// Библиотеки
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Servo.h>

// Blynk auth token
char auth[] = "af690efbe2a3460290e42cd5ef9873ae";
IPAddress blynk_ip(139, 59, 206, 133);

// Датчик DS18B20
#define DS18B20_1 8
#define DS18B20_2 9
OneWire oneWire1(DS18B20_1);
OneWire oneWire2(DS18B20_2);
DallasTemperature ds_sensor1(&oneWire1);
DallasTemperature ds_sensor2(&oneWire2);

// Датчик DHT11
#define DHTPIN 2
DHT dht1(DHTPIN, DHT11);

// Период для таймера обновления данных
#define UPDATE_TIMER 1000

// Таймер
BlynkTimer timer_update;

// Выход управления светодиодной лентой через MOS транзистор
#define PWM_LED 3

// Выход управления помпой через реле
#define PUMP 4

// Сервомотор для управления крышкой
#define SERVO1_PWM 5
Servo servo_1;

// Вход датчика влажности почвы
#define SOIL_SENSOR_1 A0
#define SOIL_SENSOR_2 A1

// Вход датчика света
#define LIGHT_SENSOR A2

void setup() {
  // Инициализация UART
  Serial.begin(115200);

  // Подключение к серверу Blynk
  Blynk.begin(auth, blynk_ip, 8442);

  // Инициализация датчика DHT11
  dht1.begin();

  // Инициализация датчиков DS18B20
  ds_sensor1.begin();
  ds_sensor2.begin();

  // Инициализация выхода реле
  pinMode(PUMP, OUTPUT);
  digitalWrite(PUMP, LOW);

  // Инициализация порта для управления сервомотором
  servo_1.attach(SERVO1_PWM);

  // Установка сервомотора в начальное положение
  servo_1.write(90);

  // Инициализация выхода на MOS транзистор
  pinMode(PWM_LED, OUTPUT);

  // Инициализация таймера
  timer_update.setInterval(UPDATE_TIMER, readSendData);
}

void loop() {
  Blynk.run();
  timer_update.run();
}

// Считывание датчиков и отправка данных на сервер Blynk
void readSendData() {
  float air_hum = dht1.readHumidity();                // Считывание влажности воздуха
  float air_temp = dht1.readTemperature();            // Считывание температуры воздуха
  Serial.print("Air temperature = ");
  Serial.println(air_temp);
  Serial.print("Air humidity = ");
  Serial.println(air_hum);
  Blynk.virtualWrite(V0, air_temp); delay(25);        // Отправка данных на сервер Blynk
  Blynk.virtualWrite(V1, air_hum); delay(25);         // Отправка данных на сервер Blynk

  ds_sensor1.requestTemperatures();                   // Считывание температуры почвы №1
  float soil_temp_1 = ds_sensor1.getTempCByIndex(0);
  Serial.print("Soil temperature 1 = ");
  Serial.println(soil_temp_1);
  Blynk.virtualWrite(V2, soil_temp_1); delay(25);     // Отправка данных на сервер Blynk

  ds_sensor2.requestTemperatures();                   // Считывание температуры почвы №2
  float soil_temp_2 = ds_sensor2.getTempCByIndex(0);
  Serial.print("Soil temperature 2 = ");
  Serial.println(soil_temp_2);
  Blynk.virtualWrite(V3, soil_temp_2); delay(25);     // Отправка данных на сервер Blynk

  float light = analogRead(LIGHT_SENSOR) / 1023.0 * 100.0;        // Считывание датчика света
  float soil_hum_1 = analogRead(SOIL_SENSOR_1) / 1023.0 * 100.0;  // Считывание влажности почвы №1
  float soil_hum_2 = analogRead(SOIL_SENSOR_2) / 1023.0 * 100.0;  // Считывание влажности почвы №2
  Serial.print("Light = ");
  Serial.println(light);
  Serial.print("Soil moisture 1 = ");
  Serial.println(soil_hum_1);
  Serial.print("Soil moisture 2 = ");
  Serial.println(soil_hum_2);
  Blynk.virtualWrite(V4, soil_hum_1); delay(25);       // Отправка данных на сервер Blynk
  Blynk.virtualWrite(V5, soil_hum_2); delay(25);       // Отправка данных на сервер Blynk
  Blynk.virtualWrite(V6, light); delay(25);          // Отправка данных на сервер Blynk
}

// Управление помпой с Blynk
BLYNK_WRITE(V7)
{
  int pump_ctl = param.asInt();   // Получение управляющего сигнала с сервера
  Serial.print("Pump power: ");
  Serial.println(pump_ctl);
  digitalWrite(PUMP, pump_ctl);   // Управление реле (помпой)
}

// Управление светом с Blynk
BLYNK_WRITE(V8)
{
  int light_ctl = param.asInt();   // Получение управляющего сигнала с сервера
  Serial.print("Light power: ");
  Serial.println(light_ctl);
  analogWrite(LIGHT_SENSOR, light_ctl);   // Управление MOS транзистором
}

// Управление сервомотором с Blynk
BLYNK_WRITE(V9)
{
  int servo_ctl = param.asInt();   // Получение управляющего сигнала с сервера
  Serial.print("Servo motor angle: ");
  Serial.println(servo_ctl);
  servo_1.write(servo_ctl);        // Управление сервомотором
}

