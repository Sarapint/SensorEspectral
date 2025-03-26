#include "DFRobot_AS7341.h"   //Sensor
#include "DFRobot_SD3031.h"   //RTC SD3031
#include <Wire.h>             //Comunicación I2C con RTC
#include <WiFi.h>             //Para ESP32
#include <ThingSpeak.h>       //Librería ThingSpeak

DFRobot_AS7341 as7341;
DFRobot_SD3031 rtc;           //Objeto RTC

int ledPin = 13;              //Pin para el LED
int buttonPin = D7;           //Conexión botón al pin 3

#define SerialLogger Serial2  //Usamos Serial2 para el Data Logger

const char* ssid = " --- ";              //Nombre de la red Wi-Fi
const char* password = "****";           //Contraseña de la red Wi-Fi

unsigned long channelID = 0000000;       //ID del canal ThingSpeak
const char* writeAPIKey = "**********";  //API Write Key de ThingSpeak

WiFiClient client;

void setup() {
  Serial.begin(115200);
  SerialLogger.begin(9600, SERIAL_8N1, 17, 16);  //Conexión para el Data Logger

  //Inicializar el RTC DS3031
  if (rtc.begin() != 0) {
    Serial.println("No se pudo encontrar el RTC DS3031!");
    delay(1000);
  }
  rtc.setHourSystem(rtc.e24hours);  //Formato horas
  //rtc.setTime(2025,3,21,8,15,0);  //Iniciar hora (solo si es necesario)

  //Detectar si IIC se comunica correctamente
  while (as7341.begin() != 0) {
    Serial.println("Fallo iniciando IIC, por favor comprueba si la conexión es correcta.");
    delay(1000);
  }

  pinMode(buttonPin, INPUT);     //Declarar botón como input

  //Imprimir encabezado para el archivo en el Serial Data Logger
  SerialLogger.println("Fecha,Hora,F1;F2;F3;F4;F5;F6;F7;F8");

  //Conectar a Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a Wi-Fi...");
  }
  Serial.println("Conectado a Wi-Fi!");
  
  ThingSpeak.begin(client); //Inicializar ThingSpeak
}

void loop() {
  if (digitalRead(buttonPin) == HIGH) {    //Cuando se pulsa el botón
    mediciones();
  }
  delay(1000);  //Breve delay para no saturar el serial monitor
}

void mediciones() {
  //Leer la hora actual
  sTimeData_t sTime = rtc.getRTCTime();
  Serial.println("Inicio medición");

  DFRobot_AS7341::sModeOneData_t data1;
  DFRobot_AS7341::sModeTwoData_t data2;

  //Empezar mediciones 
  as7341.enableLed(true);
  as7341.startMeasure(as7341.eF1F4ClearNIR);
  data1 = as7341.readSpectralDataOne();
  as7341.enableLed(false);
  delay(500);
  as7341.enableLed(true);
  as7341.startMeasure(as7341.eF5F8ClearNIR);
  data2 = as7341.readSpectralDataTwo();
  as7341.enableLed(false);

  //Imprimir en el Serial Data Logger
  SerialLogger.print(sTime.day, DEC);
  SerialLogger.print('/');
  SerialLogger.print(sTime.month, DEC);
  SerialLogger.print('/');
  SerialLogger.print(sTime.year, DEC);
  SerialLogger.print(";");
  SerialLogger.print(sTime.hour, DEC);
  SerialLogger.print(':');
  SerialLogger.print(sTime.minute, DEC);
  SerialLogger.print(':');
  SerialLogger.print(sTime.second, DEC);
  SerialLogger.print(";");
  
  SerialLogger.print(data1.ADF1);
  SerialLogger.print(";");
  SerialLogger.print(data1.ADF2);
  SerialLogger.print(";");
  SerialLogger.print(data1.ADF3);
  SerialLogger.print(";");
  SerialLogger.print(data1.ADF4);
  SerialLogger.print(";");
  SerialLogger.print(data2.ADF5);
  SerialLogger.print(";");
  SerialLogger.print(data2.ADF6);
  SerialLogger.print(";");
  SerialLogger.print(data2.ADF7);
  SerialLogger.print(";");
  SerialLogger.println(data2.ADF8);

  Serial.println("Fin medición.");

  //Asigna datos a campos de ThingSpeak
  ThingSpeak.setField(1, data1.ADF1);
  ThingSpeak.setField(2, data1.ADF2);
  ThingSpeak.setField(3, data1.ADF3);
  ThingSpeak.setField(4, data1.ADF4);
  ThingSpeak.setField(5, data2.ADF5);
  ThingSpeak.setField(6, data2.ADF6);
  ThingSpeak.setField(7, data2.ADF7);
  ThingSpeak.setField(8, data2.ADF8);
  
  //Enviar los datos a ThingSpeak
  int x = ThingSpeak.writeFields(channelID, writeAPIKey);
  if(x == 200) {
    Serial.println("Datos enviados correctamente a ThingSpeak.");
  } else {
    Serial.println("Error al enviar los datos a ThingSpeak.");
  }
}
