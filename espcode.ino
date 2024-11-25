#include <Wire.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <time.h>
#include <TinyGPS++.h>

#include <DHT.h>
#include <Battery.h>

const char* ssid = "SATC IOT";
const char* password = "IOT2024@#";

const char* serverName = "http://localhost:3000/documentos";

#define MQ2_PIN 35      // Sensor de gás MQ-2
#define MQ7_PIN 34      // Sensor de gás MQ-7
// defini os pinos analogicos 

#define DHT_PIN 4       // Pino do sensor DHT22
#define GPS_RX 16       // Pino RX do GPS
#define GPS_TX 17       // Pino TX do GPS
#define EMERGENCY_BTN 2 // Pino do botão de emergência
#define BATTERY_PIN 33  // Pino para monitoramento da bateria
// defini os pinos digitais

// pinos dos leds
#define LED_NORMAL 13    // LED Verde
#define LED_WARNING 12   // LED Amarelo
#define LED_DANGER 14    // LED Vermelho
#define LED_EMERGENCY 27 // LED Azul

// Constantes
#define DHT_TYPE DHT22
#define BATTERY_MAX_VOLTAGE 4.2
#define BATTERY_MIN_VOLTAGE 3.3
#define GAS_SAMPLES 10
#define WARNING_THRESHOLD 300  
// definir constantes

#define DANGER_THRESHOLD 700 //limite do gas e tal

DHT dht(DHT_PIN, DHT_TYPE);
TinyGPSPlus gps;
HardwareSerial GPSSerial(1);
Battery battery(BATTERY_PIN, BATTERY_MIN_VOLTAGE, BATTERY_MAX_VOLTAGE);
// Inicializa os objetos para cada sensor e componente do sistema

// Variaveis globais
float latitude = 0;
float longitude = 0;
bool emergencyMode = false;

float readGasSensor(int pin) {
  float sum = 0;
  for(int i = 0; i < GAS_SAMPLES; i++) {
    sum += analogRead(pin);
    delay(10);
  }
  return sum / GAS_SAMPLES;
}
// func que faz varias leituras do sensor de gas e retorna a media p maior precisao

void IRAM_ATTR emergencyButtonPressed() {
  emergencyMode = true;
}
// define a func chamada quando o botão de emergencia for apertado

void setup() {
  Serial.begin(115200);
  
  pinMode(MQ2_PIN, INPUT);
  pinMode(MQ7_PIN, INPUT);
  pinMode(EMERGENCY_BTN, INPUT_PULLUP);

  pinMode(LED_NORMAL, OUTPUT);
  pinMode(LED_WARNING, OUTPUT);
  pinMode(LED_DANGER, OUTPUT);
  pinMode(LED_EMERGENCY, OUTPUT);

// confi dos pinos dos LEDs como saída e prepara para inicializar os sensores

  dht.begin();
  GPSSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  battery.begin();
  
  attachInterrupt(digitalPinToInterrupt(EMERGENCY_BTN), emergencyButtonPressed, FALLING);
// incializa os botoes com o attach interrupt 

  // conecta no wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Conectando ao WiFi");
  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
//tenta conectar no wifi 20 vezes
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha na conexão WiFi!");
  }
// se deu certo a conexao ele mostra aqui
  configTime((-3 * 3600), 0, "pool.ntp.org");
}
// fuso horario de brasilia

void updateLEDs(float mq2_value, float mq7_value) {
  bool warning = (mq2_value > WARNING_THRESHOLD || mq7_value > WARNING_THRESHOLD);
  bool danger = (mq2_value > DANGER_THRESHOLD || mq7_value > DANGER_THRESHOLD);
  
  digitalWrite(LED_NORMAL, !warning && !danger && !emergencyMode);
  digitalWrite(LED_WARNING, warning && !danger && !emergencyMode);
// func para atualizar os leds baseado nas leituras dos sensores de gas

  digitalWrite(LED_DANGER, danger && !emergencyMode);
  digitalWrite(LED_EMERGENCY, emergencyMode);
}

void loop() {
  float mq2_value = readGasSensor(MQ2_PIN);
  float mq7_value = readGasSensor(MQ7_PIN);

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float batteryLevel = battery.level();
  
  while (GPSSerial.available() > 0) {
// le os dados dos sensores 

    if (gps.encode(GPSSerial.read())) {
      if (gps.location.isValid()) {
        latitude = gps.location.lat();
        longitude = gps.location.lng();
      }
    }
  }
// processa os dados do GPS e atualiza as coordenadas se válidas

  updateLEDs(mq2_value, mq7_value);
  
  // verifica a conexao do wifi e tal
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconectando ao WiFi...");
    WiFi.disconnect();

    WiFi.reconnect();
    delay(1000);
    return;
  }
  
  // pega a hora atual
  time_t now;
  struct tm* localTime;

  time(&now);
  localTime = localtime(&now);
  
  char dataHora[20];
  sprintf(dataHora, "%04d-%02d-%02d %02d:%02d:%02d",
          localTime->tm_year + 1900, localTime->tm_mon + 1, localTime->tm_mday,
// FOrmata a data e a hora e tal

          localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
  
  // Prepara e envia dados
  HTTPClient http;
  http.setConnectTimeout(1000);
  http.setTimeout(1000);
  http.begin(serverName);
// configura a conexao pra postar os dados na Api

  http.addHeader("Content-Type", "application/json");
  
  String jsonPayload = "{\"data\": \"" + String(dataHora) + "\", "
                   "\"mq2\": " + String(mq2_value, 2) + ", "
                   "\"mq7\": " + String(mq7_value, 2) + ", "
                   "\"temperature\": " + String(temperature, 2) + ", "
// prepara o JSON com os dados dos sensores

                   "\"humidity\": " + String(humidity, 2) + ", "
                   "\"latitude\": " + String(latitude, 6) + ", "
                   "\"longitude\": " + String(longitude, 6) + ", "
                   "\"battery\": " + String(batteryLevel, 2) + ", "
                   "\"emergency\": " + String(emergencyMode) + "}";

  Serial.println("Enviando dados: " + jsonPayload);
  
  int httpResponseCode = http.POST(jsonPayload);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Código HTTP: " + String(httpResponseCode));
// Envia os dados e verifica a resposta do servidor

    Serial.println("Resposta: " + response);
  } else {
    Serial.print("Erro na requisição HTTP: ");
    Serial.println(httpResponseCode);
    Serial.println("Erro: " + http.errorToString(httpResponseCode));
  }
// caso tenha erros, eles serao tratados aqui

  http.end();
  delay(5000); //espera 5000 milisegundos ate enviar dnv as infos
}
