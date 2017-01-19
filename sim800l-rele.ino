#include <SoftwareSerial.h>
#include <Arduino.h>
SoftwareSerial sim800(2, 3); // установка контактов 2 и 3 для программного порта (для микроконтроллера ардуино убать букву D)
bool rele = HIGH;
int relpin = 13; //пин реле
int LED = 12; //светодиод
char* adminNumber[] = {"+79005004041", "+78004123252"};
char i;

void setup()
{
  Serial.begin(115200);
  sim800.begin(115200);
  pinMode(relpin, OUTPUT);
  digitalWrite(relpin, HIGH);

  //проверка работы модуля

  sim800.println("AT");                         //  смотрим есть ли ответ от модема
  delay(100);
  if (!sim800.find("OK")) {                     // если нет, дергаем ногу включения
    digitalWrite(LED, LOW);
    delay(1000);
    digitalWrite(LED, HIGH);
  }
  // нужно дождатся включения модема и соединения с сетью
  delay(2000);

  sim800.println("ATE0");                  // выключаем эхо

  while (1) {                           // ждем подключение модема к сети стати вайл1 это бесконечный цикл и выход  из него будет коггда когда сработает брэйк
    sim800.println("AT+COPS?"); //отправляем в сим800 команду проверки подключился ли он к сети
    if (sim800.find("+COPS: 1")) break;
    /*уловие если в выводе ответа на АТ запрос есть то что в условии проверки то делается брейк и вайл прекращается или если строка в выводе не находится то
      блымаем светодиодом и идем опять по бесконечному вайл цикру с начала шлем команду АТ ну и т.д.
    */
    digitalWrite(LED, LOW);
    delay(50);
    digitalWrite(LED, HIGH);
    delay(500);
  }

  Serial.println("Modem OK ZBS");  // ну дождались мы когда все таки наступит брэйк и пишем в консоль что все ок
  digitalWrite(LED, LOW);               // блымаем светодиодом
  delay(1500);
  digitalWrite(LED, HIGH);
  delay(250);
  digitalWrite(LED, LOW);

  // настройка приема сообщений

  sim800.print("AT+CMGF=1\r"); // устанавливаем текстовый режим смс-сообщения
  delay(500); //
  sim800.print("AT+IFC=1, 1\r"); // устанавливаем программный контроль потоком передачи данных
  delay(500);
  sim800.print("AT+CPBS=\"SM\"\r"); // открываем доступ к данным телефонной книги SIM-карты
  delay(500);
  sim800.print("AT+CNMI=1,2,2,1,0\r"); // включает оповещение о новых сообщениях, новые сообщения приходят в следующем формате: +CMT: "<номер телефона>", "", "<дата, время>",
  //на следующей строчке с первого символа идёт содержимое сообщения
  delay(700);
}

String currStr = ""; // если эта строка сообщение,

boolean isStringMessage = false; //  то ставим флаг на разрешение оброботки SMS


void loop()
{
  if (!sim800.available())// если не данных от модуля SIM800l, то дальше не идем.
    return;

  char currSymb = sim800.read(); // записываем в переменую символы, которые получили от модуля.

  if ('\r' == currSymb) // если получили символ перевода коректи в начало строки, это означает что передача сообщения от модуля завершена.
  {
    if (isStringMessage) { // если текущая строка – сообщение, то…

      if (!currStr.compareTo("on")) { // если текст сообщения совпадает с "on",
        rele = LOW;                        // то включаем низкоуровневое реле.
        digitalWrite(relpin, rele);
      }
      if (!currStr.compareTo("off")) { // если текст сообщения совпадает с "off",
        rele = HIGH;                        // то выключаем низкоуровневое реле.
        digitalWrite(relpin, rele);
      }

      isStringMessage = false;
    }

    else {
      if (currStr.startsWith("+CMT"))
      { // если текущая строка начинается с "+CMT", то следующая сообщение

        for (i = 0; i <= 1; i++)
        { // число 3 - это количество заданных тел.номеров.
          if (currStr.indexOf(adminNumber[i])) //сверяем номер с номером прешедним SMS
          {
            Serial.println(adminNumber[i]);
            isStringMessage = true;
            break;
          }
        }

      }
    }
    currStr = "";
  }

  else if ('\n' != currSymb) { //  игнорируем второй символ в последовательности переноса строки: \r\n , и
    currStr += String(currSymb); //дополняем текущую команду новым сиволом
  }
}
