//Работа с датчиками DS18B20 без библиотеки Dallas
#include <OneWire.h> 
OneWire  ds(2); // датчик подключен к выводу 2
byte addr[8]={0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}; //Адрес датчика
unsigned long timerDS; //Таймер 
int temperatureDS = 0; //Показания датчика

void setup() {
  ds.reset();
  ds.write(0xCC); //Обращение ко всем DS18B20
  ds.write(0x44); // Команда на первое измерение
  delay(1000);//1 секунда для инициализации всех датчиков, согласно документации DS18B20
}

void loop() {
  // Обработка датчика DS18B20 задержка 2 секунды, чем больше интервал, тем меньше саморазогрев
  if (millis() - timerDS > 2000)   
  {
    timerDS = millis();
    getTemperature();
  }
}

  void getTemperature()
{
  static bool flagDall = 1;
  flagDall = !flagDall;
  if (flagDall)
  {
    ds.reset();
    ds.select(addr);
    ds.write(0x44); // Команда на измерение
  }
  else
  {
    ds.reset();
    ds.select(addr);
    ds.write(0xBE);                      // Считывание значения
    temperatureDS = (ds.read() | ds.read() << 8) / 16.0; // Принимаем два байта температуры
  }
}