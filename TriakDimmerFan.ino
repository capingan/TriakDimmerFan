#include <PubSubClient.h>
#include <SPI.h>
#include <Ethernet.h>
#include "Dimmer.h"
#include <DHT.h>
// -------------------------------------- BEGIN - Пины Arduino ----------------------------------------------
#define LED_pin 5                         //Пин 5 для светодиодов
#define Relay1_pin 6                      //Пин 6 для реле 1
#define Relay2_pin 7                      //Пин 7 для реле 2  
// -------------------------------------- END - Пины Arduino 
// -------------------------------------- BEGIN - Нумерация диммеров ----------------------------------------                                          
#define Dimmer_1 0                       //#define <"имя димера"> <"порядковый номер"> (нумерация от 0!!!)
#define Dimmer_2 1                       //#define <"имя димера"> <"порядковый номер"> (нумерация от 0!!!)
// -------------------------------------- END - Нумерация диммеров ------------------------------------------
// -------------------------------------- BEGIN - Пины Arduino ----------------------------------------------
#define Dimmer_1_pin A1                   //Пин A1 (на плате диммера это пин DMR2)
#define Dimmer_2_pin A2                   //Пин A2 (на плате диммера это пин DMR1)
// -------------------------------------- END - Пины Arduino ------------------------------------------------

#define DHTPIN 3                 // Номер пина, к которому подсоединен датчик
#define DHTTYPE DHT22            // Задаём тип DHT датчика
DHT dht(DHTPIN, DHTTYPE);

// Установить режим работы диммера
Dimmer dimmer1(Dimmer_1_pin, DIMMER_NORMAL);   // DIMMER_NORMAL (обычный режим) , DIMMER_RAMP (плавное затухание) , DIMMER_COUNT (для высокоинерционных нагрузок, обогреватели и т.д.)
Dimmer dimmer2(Dimmer_2_pin, DIMMER_NORMAL);

// -------------------------------------- BEGIN - Глобальные переменные -------------------------------------
int Led = 0;                              //Переменная для хранения состояния светодиода 
boolean Relay1 = HIGH;                    //Переменная для хранения состояния Реле 1 
boolean Relay2 = HIGH;                    //Переменная для хранения состояния Реле 2
// -------------------------------------- END - Глобальные переменные ---------------------------------------
// -------------------------------------- BEGIN - Глобальные переменные -------------------------------------
int Dim1_value = 0;
int Dim2_value = 0;
boolean Switch1_value = 0;
boolean Switch2_value = 0;
// -------------------------------------- END - Глобальные переменные ---------------------------------------

int echoPin = 9;       // подкл. 9 pin - Echo  (Дальномер HC-SR04)
int trigPin = 8;       // подкл. 8 pin - Trig  (Дальномер HC-SR04)

// Обработка данных с датчика DHT22
  int h = dht.readHumidity();     // Переменная типа int для Влажности
  int t = dht.readTemperature();  // Переменная типа int для Температуры

  unsigned long timing; // Переменная для хранения точки отсчета

// -------------------------------------- BEGIN - Установка параметров сети ---------------------------------
void callback(char* topic, byte* payload, unsigned int length);

// Установить MAC адресс для этой Arduino (должен быть уникальным в вашей сети)
byte mac[] = { 0x90, 0xB2, 0xFB, 0x0D, 0x4E, 0x59 };

// Утановить IP адресс для этой Arduino (должен быть уникальным в вашей сети)
IPAddress ip(192, 168, 0, 62);

// Уставновить IP адресс MQTT брокера
byte server[] = { 192, 168, X, X };

// Уставновить ИмяКлиента, Логин и Пароль для подключения к MQTT брокеру
const char* clientID = "Arduino_Dimmer";
const char* mqtt_username = "xxxxxxxxx";
const char* mqtt_password = "xxxxxx";

// Настройки топиков для получения команд с брокера
const char* mqtt_Dimmer1 = "Dimmer/triac1";
const char* mqtt_Dimmer2 = "Dimmer/triac2";
const char* mqtt_Switch1 = "Dimmer/switch1";
const char* mqtt_Switch2 = "Dimmer/switch2";

// Настройки LWT топика для отображения статуса диммера
const char* mqtt_willTopic = "Dimmer/availability";
const char* mqtt_payloadAvailable = "Online";
const char* mqtt_payloaNotdAvailable = "Offline";

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);
// --------------------------------------- END - Установка параметров сети ----------------------------------


// --------------------------------------- BEGIN - Подключение и подписка на MQTT broker ----------------------------------
boolean reconnect() {
  //Serial.println("reconnect...");
  if (client.connect("Arduino_test", mqtt_username, mqtt_password)) 
 // {       
    
 //   Serial.println("MQTT connected");  
 //}
 
 if (client.connect(clientID, mqtt_username, mqtt_password, mqtt_willTopic, 0, true, mqtt_payloaNotdAvailable, true))
 {
    client.publish (mqtt_willTopic, mqtt_payloadAvailable, true) ;      
    client.subscribe(mqtt_Dimmer1); Serial.print("Connected to: "); Serial.println(mqtt_Dimmer1);
    client.subscribe(mqtt_Dimmer2); Serial.print("Connected to: "); Serial.println(mqtt_Dimmer2);
    client.subscribe(mqtt_Switch1); Serial.print("Connected to: "); Serial.println(mqtt_Switch1);
    client.subscribe(mqtt_Switch2); Serial.print("Connected to: "); Serial.println(mqtt_Switch2);
    client.subscribe("arduino/led"); Serial.println("Connected to: arduino/led");
    client.subscribe("arduino/relay1"); Serial.println("Connected to: arduino/relay1");
    client.subscribe("arduino/relay2"); Serial.println("Connected to: arduino/relay2");
    Serial.println("MQTT connected");    
  } 
 
  return client.connected();
}
// --------------------------------------- END - Подключение и подписка на MQTT broker ----------------------------------

// --------------------------------------- BEGIN - void setup() -------------------------------------------
void setup()
{
  digitalWrite(Relay1_pin, HIGH); // Решение проблемы с LOW статусом пинов при загрузке ардуино
  digitalWrite(Relay2_pin, HIGH); // Решение проблемы с LOW статусом пинов при загрузке ардуино
  pinMode(LED_pin, OUTPUT);
  pinMode(Relay1_pin, OUTPUT);
  pinMode(Relay2_pin, OUTPUT);                
 
  Serial.begin(9600); // Open serial communications

  // Start with a hard-coded address:
  Ethernet.begin(mac, ip);

  Serial.print("My ip address: ");
  Serial.println(Ethernet.localIP());
  
{
  dimmer1.begin();
  dimmer2.begin();       
 
  Serial.begin(9600); // Open serial communications

  

  reconnect(); // Подключение к брокеру, подписка на прописанные выше темы

}  

 

{
  // 1 бод равно 0.8 бит/сек
  // 1 бит/сек равно 1.25 бод
            // Задаём скорость порта в БОД'ах.
  Serial.println("DHT22 test!");  // Тестовое сообщ. при откр. Монитора порта

  pinMode(trigPin, OUTPUT);       // 8 pin Дальномера
  pinMode(echoPin, INPUT);        // 9 pin Дальномера

  dht.begin();

        // Инициализируем mac, ip
}

}
// --------------------------------------- END - void setup() ---------------------------------------------


// --------------------------------------- BEGIN - void loop() --------------------------------------------
void loop() {
if (!client.connected()) {
     reconnect();
   }

client.loop();

{
   if (millis() - timing > 10000){ // Вместо 10000 подставьте нужное вам значение паузы 
  timing = millis();
  
  // Обработка данных с Дальномера HC-SR04
  int duration, cm;     //Задаём переменные для продолжительности и См.
  digitalWrite(trigPin, LOW);          // Ультразвук Выкл.
  delayMicroseconds(2);                // Задержка 2 микросекунды
  digitalWrite(trigPin, HIGH);         // Ультразвук Вкл.
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);   // Получаем Эхо в ответ
  cm = duration / 58;                  // Производим расчёт в Сантиметры
  Serial.print(cm);                    // Готовое значение в См.
  Serial.println(" cm");

  static char char_Zvyk[10];    // Переменная для перевода (HC-SR04)
  dtostrf(cm, 4, 0, char_Zvyk); // Перевод из int в char (HC-SR04)


  // Обработка данных с датчика DHT22
  int h = dht.readHumidity();     // Переменная типа int для Влажности
  int t = dht.readTemperature();  // Переменная типа int для Температуры

  // Преобразуем переменные для отправки в MQTT в Брокер
  static char char_temp[10];      // Переменная для перевода из int в char
  dtostrf(t, 3, 0, char_temp);    // Перевод из int в char

  static char char_hum[10];
  dtostrf(h, 3, 0, char_hum);


  if (isnan(t) || isnan(h))     // Проверка удачно ли прошло считывание с DHT22
  {
    Serial.println("Failed to read from DHT22");  // Не удалось прочитать DHT22
  }
  else
  {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
  }


  //Отправка данных по MQTT в Брокер или MajorDoMo...
 if (client.connect("DHT_UltrasonicClient"))
  {
    //Отправка данных с датчика DHT22
    client.publish("home/sensor/temp", char_temp); //отправляем в Брокер значения
    client.publish("home/sensor/hum", char_hum);

    //Отправка данных с Ультразвукового дальномера HC-SR04
    client.publish("home/Ultrasonic/Vannaya", char_Zvyk);

    // Вместо 10000 подставьте нужное вам значение паузы 
   
    
   // delay(4000);      // Задержка в секундах
    //  client.disconnect();      // Отключиться
  }
}
}
}
// --------------------------------------- END - void loop() ----------------------------------------------


// --------------------------------------- BEGIN - void callback ------------------------------------------
// Чтение данных из MQTT брокера
void callback(char* topic, byte* payload, unsigned int length) {
  // проверка новых сообщений в подписках у брокера
    payload[length] = '\0';
    Serial.print("Topic: ");
    Serial.print(String(topic));
    Serial.println(" - ");
	
	 payload[length] = '\0';
    //Serial.print("Topic: ");
    //Serial.print(String(topic));
    //Serial.println(" - ");

  if (String(topic) == "arduino/led") {
    String value = String((char*)payload);
    Led = value.substring(0, value.indexOf(';')).toInt();
    Led = map(Led, 0, 100, 0, 255);   
    analogWrite(LED_pin, Led);
    Serial.print("Znachenie prisvoenoe peremennoy Led: ");
    Serial.println(Led);
  }
  
  if (String(topic) == "arduino/relay1") {
    String value = String((char*)payload);
    Relay1 = value.substring(0, value.indexOf(';')).toInt();
    Serial.print("Znachenie prisvoenoe peremennoy Relay1: ");
    Serial.println(Relay1);   
    digitalWrite(Relay1_pin, Relay1);
  }

  if (String(topic) == "arduino/relay2") {
    String value = String((char*)payload);
    Relay2 = value.substring(0, value.indexOf(';')).toInt();
    Serial.print("Znachenie prisvoenoe peremennoy Relay2: ");
    Serial.println(Relay2);   
    digitalWrite(Relay2_pin, Relay2);
  }
  
   if (String(topic) == "Dimmer/triac1") {
     String value = String((char*)payload);
     Dim1_value = value.substring(0, value.indexOf(';')).toInt();
     dimmer1.set(Dim1_value);
    
    Serial.print("Диммер 1: ");
    Serial.println(Dim1_value);

  }
  if (String(topic) == "Dimmer/switch1") {
     String value = String((char*)payload);
     Switch1_value = value.substring(0, value.indexOf(';')).toInt();
     dimmer1.set(Dim1_value, Switch1_value);
    
    Serial.print("Выключатель 1: ");
    Serial.println(Switch1_value);

  }

  if (String(topic) == "Dimmer/triac2") {
     String value = String((char*)payload);
     Dim2_value = value.substring(0, value.indexOf(';')).toInt();
     if (Dim2_value == 0) {dimmer2.set(0,Switch2_value);}
     else {
      Dim2_value=map(Dim2_value, 1, 100, 30, 100);
      dimmer2.set(Dim2_value, Switch2_value);
      }
     
    Serial.print("Диммер 2: ");
    Serial.println(Dim2_value);

  }
  if (String(topic) == "Dimmer/switch2") {
     String value = String((char*)payload);
     Switch2_value = value.substring(0, value.indexOf(';')).toInt();
     dimmer2.set(Dim2_value, Switch2_value);
    
    Serial.print("Выключатель 2: ");
    Serial.println(Switch2_value);

  }
  
{
  // Выделяем необходимое кол-во памяти для копии payload
  byte* p = (byte*)malloc(length);
  // Копирование payload в новый буфер
  memcpy(p, payload, length);
  client.publish("home/data/status/sensor", p, length);
  // Освобождаем память
  free(p);
}  
  
}
// ---------------------------------------- END - void callback -------------------------------------------
