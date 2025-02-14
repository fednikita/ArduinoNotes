//GSM Модуль SIM800/SIM900
//Звоним с номера из белого списка, получаем в ответ sms
#include <SoftwareSerial.h> //Программный порт
SoftwareSerial gsm(7, 8); // D7(TX) D8(RX) пины SIM900 // программный UART
String _response = "";    // Переменная для хранения ответа модуля
String whiteListPhones = "+79112223344, +79112223344, +79112223344"; // Белый список номеров
String smsText = "Hello world!"; //Текст СМС латиницей

void setup() {
  gsm.begin(9600); // Выставляем скорость общения с GSM-модулем
  _response = sendATCommand("AT", true);  // Проверка связи с SIM900
  _response.trim();                       // Убираем пробельные символы в начале и конце
  if (_response != "OK"){                 //Если нет ответа
    powerUpOrDown();                      //Включаем SIM900
    delay(30000);                         //Ждём 30 секунд для поиска сети
    }
  _response = sendATCommand("AT+CCID", true);     // Прочитать информацию о SIM карте, чтобы убедиться, что SIM карта подключена
  _response = sendATCommand("AT+CREG?", true);    // Проверяем, зарегистрирован ли модуль в сети 
  _response = sendATCommand("AT+CSQ", true);      // Тест качества сигнала, диапазон значений 0-31, 31 - лучший 
  _response = sendATCommand("AT+CSCB=1", true);   // Приём рассылок 1=запрещено, 0=разрешено 
  _response = sendATCommand("AT+CMGF=1", true);   // Включение текствого режима
  _response = sendATCommand("AT+CNMI=1,2,0,0,0", true);   // Как обрабатывать новые входящие SMS сообщения 
  _response = sendATCommand("AT+CMGD=1,4", true);   // Удалить все СМС из памяти сим
  _response = sendATCommand("AT+CLIP=1", true);   // Включаем АОН 
}

void loop() {
  if (gsm.available())   {                   // Если модем, что-то отправил...
    _response = waitResponse();                 // Получаем ответ от модема
    _response.trim();                           // Убираем лишние пробелы в начале и конце
      if (_response.startsWith("RING")) {         // Есть входящий вызов
        int phoneindex = _response.indexOf("+CLIP: \"");// Есть ли информация об определении номера, если да, то phoneindex>-1
        String innerPhone = "";                   // Переменная для хранения определенного номера
        if (phoneindex >= 0) {                    // Если информация была найдена
          phoneindex += 8;                        // Парсим строку и ...
          innerPhone = _response.substring(phoneindex, _response.indexOf("\"", phoneindex)); // ...получаем номер
        }
        // Проверяем, чтобы длина номера была больше 6 цифр, и номер должен быть в списке
        // Если сразу отклонять вызов, SIM900 не сможет определить номер, сперва проверка, затем действие 
        if (innerPhone.length() >= 7 && whiteListPhones.indexOf(innerPhone) >= 0) {
          sendATCommand("ATH0", true);        // Если да, то //ATH - отклоняем вызов ATA - отвечаем на вызов
          sendSMS(innerPhone, smsText);       // Отправляем SMS
        }
        else {
          sendATCommand("ATH0", true);        // Иначе //ATH - отклоняем вызов ATA - отвечаем на вызов
        }
      }
    }
  }
}

void powerUpOrDown() //Включение или Выключение SIM900
{
  pinMode(9, OUTPUT); // D9 пин включение/выключение SIM900
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(2000);
  digitalWrite(9,LOW);
  delay(3000);
}

String sendATCommand(String cmd, bool waiting) {
  String _resp = "";                            // Переменная для хранения результата
  gsm.println(cmd);                          // Отправляем команду модулю
  if (waiting) {                                // Если необходимо дождаться ответа...
    _resp = waitResponse();                     // ... ждем, когда будет передан ответ
    // Если Echo Mode выключен (ATE0), то эти 3 строки можно закомментировать
    if (_resp.startsWith(cmd)) {                // Убираем из ответа дублирующуюся команду
      _resp = _resp.substring(_resp.indexOf("\r", cmd.length()) + 2);
    }
  }
  return _resp;                                 // Возвращаем результат. Пусто, если проблема
}

String waitResponse() {                         // Функция ожидания ответа и возврата полученного результата
  String _resp = "";                            // Переменная для хранения результата
  long _timeout = millis() + 10000;             // Переменная для отслеживания таймаута (10 секунд)
  while (!gsm.available() && millis() < _timeout)  {}; // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  if (gsm.available()) {                     // Если есть, что считывать...
    _resp = gsm.readString();                // ... считываем и запоминаем
  }
  return _resp;                                 // ... возвращаем результат. Пусто, если проблема
}

void sendSMS(String phone, String message)
{
  sendATCommand("AT+CMGS=\"" + phone + "\"", true);             // Переходим в режим ввода текстового сообщения
  sendATCommand(message + "\r\n" + (String)((char)26), true);   // После текста отправляем перенос строки и Ctrl+Z
}
