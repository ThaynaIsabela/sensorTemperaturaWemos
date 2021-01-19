#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

const char* ssid = "";
const char* password = "";

unsigned long lastCheck;

ESP8266WebServer server(80);

void handleRoot() {
  String message = "";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup() {
  Serial.begin(115200);

  dht.begin();
  lcd.begin();
  
  Serial.println("Iniciando...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Conexao falhou! Reiniciando...");
    delay(5000);
    ESP.restart();
  }
 
  ArduinoOTA.onStart([]() {
    Serial.println("Inicio...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nFim!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progresso: %u%%\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Autenticacao Falhou");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Falha no Inicio");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Falha na Conexao");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Falha na Recepcao");
    else if (error == OTA_END_ERROR) Serial.println("Falha no Fim");
  });
  ArduinoOTA.begin();
  Serial.println("Pronto");
  Serial.print("Endereco IP: ");
  Serial.println(WiFi.localIP());

  server.on("/remoto", handleRoot);
  server.begin();

  lcd.setBacklight(HIGH);

  lastCheck = millis();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  if (millis() - lastCheck > 1000) {
    float h = dht.readHumidity();
    // Temperature em Celsius (default)
    float t = dht.readTemperature();
  
    // Verifique se alguma leitura falhou e tenta novamente.
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Falha de leitura do sensor DHT!"));
    }
    else {
      Serial.print(F("Umidade: "));
      Serial.print(h);
      Serial.print(F("%  Temperatura: "));
      Serial.print(t);
      Serial.print(F("°C "));
    
      lcd.clear();
      
      lcd.setCursor(0, 0);
      lcd.print(F("Umidade: "));
      lcd.print(h);
      lcd.print(F(" %"));
    
      lcd.setCursor(0, 1);
      lcd.print(F("Temp: "));
      lcd.print(t);
      lcd.write(32);  // Caracter espaço
      lcd.write(223); // Caracter °
      lcd.print(F("C")); 
    }
    
    lastCheck = millis();
  }
  
}
