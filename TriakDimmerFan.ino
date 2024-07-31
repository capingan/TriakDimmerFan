/*
  Скетч для вывода в Монитор порта теспературы в Цельсиях и Влажности
  с датчика DHT22 и отправки их по MQTT в брокер и MajorDoMo.
*/

#include <SPI.h>                 // Библиотека SPI шины
#include <Ethernet.h>            // Ethernet библиотека
#include <PubSubClient.h>        // Библиотека MQTT
#include <DHT.h>                 // Библиотека для датчиков DHT11/22

#define DHTPIN 2                 // Номер пина, к которому подсоединен датчик
#define DHTTYPE DHT22            // Задаём тип DHT датчика
DHT dht(DHTPIN, DHTTYPE);


// Задаём mac и ip адреса в Локальной сети
byte mac[]    = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip{192, 168, 1, 74};      //ip Адрес Ethernet Shild'a Arduino
IPAddress server{192, 168, 1, 70};  //ip Адрес для MQTT Брокера

// Шапка Функции Callback (обратный вызов)
void callback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;                                 //Инициализируем Ethernet клиент
PubSubClient client(server, 1883, callback, ethClient);   //Инициализируем MQTT клиент


// Функция Callback
void callback(char* topic, byte* payload, unsigned int length)
{
  // Выделяем необходимое кол-во памяти для копии payload
  byte* p = (byte*)malloc(length);
  // Копирование payload в новый буфер
  memcpy(p, payload, length);
  client.publish("home/data/status/sensor", p, length);
  // Освобождаем память
  free(p);
}


void setup()
{
  // 1 бод равно 0.8 бит/сек
  // 1 бит/сек равно 1.25 бод
  Serial.begin(9600);             // Задаём скорость порта в БОД'ах.
  Serial.println("DHT22 test!");  // Тестовое сообщ. при откр. Монитора порта

  dht.begin();

  Ethernet.begin(mac, ip);        // Инициализируем mac, ip
}


void loop()
{
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

  if (client.connect("DHTClient"))
  {
    //Отправка данных по MQTT в Брокер
    client.publish("home/data/status/sensor/temp", char_temp); //отправляем в Брокер значения
    client.publish("home/data/status/sensor/hum", char_hum);
    client.subscribe("inhome/data/status/sensor/#");       // (тестовая) для отпр. данных в MajorDoMo вручную
    delay(3000);              // Отправка данных в Брокер раз в 5 секунд

    client.disconnect();      // Отключиться
  }
}
