#include <WiFi.h>
#include "DHT.h"

// Definição de pinos
#define MQ7_PIN 34        // Pino analógico do sensor MQ-7
#define BOTAO_PANICO_PIN 35 // Pino do botão de pânico
#define LDR_PIN 32         // Pino analógico do LDR
#define LED_GREEN_PIN 2    // LED verde (sistema ligado)
#define LED_RED_PIN 4      // LED vermelho (emergência ou gás)
#define LANTERNA_PIN 15    // LED da lanterna (ativado pela baixa luminosidade)
#define DHT_PIN 27         // Pino do DHT11

// Configuração do DHT11
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// Configuração Wi-Fi
const char* ssid = "SATC IOT";
const char* password = "IOT2024@#";

// Limites
#define LIMIAR_GAS 300 // Limite de perigo para o gás
#define LIMIAR_LDR 400 // Limite de baixa luminosidade (ajustável)

void setup() {
  // Inicializa comunicação serial
  Serial.begin(115200);

  // Inicializa pinos
  pinMode(MQ7_PIN, INPUT);
  pinMode(BOTAO_PANICO_PIN, INPUT_PULLUP); // Botão com pull-up interno
  pinMode(LDR_PIN, INPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LANTERNA_PIN, OUTPUT);

  // Inicializa DHT11
  dht.begin();

  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi!");

  // Liga LED verde indicando que o sistema está funcionando
  digitalWrite(LED_GREEN_PIN, HIGH);
}

void loop() {
  // Leitura do botão de pânico
  bool botaopanicoapertado = digitalRead(PANIC_BUTTON_PIN) == LOW;

  // Leitura do sensor MQ-7
  int mq7Value = analogRead(MQ7_PIN);
  Serial.print("MQ-7 (gás): ");
  Serial.println(mq7Value);

  // Leitura do LDR
  int ldrValue = analogRead(LDR_PIN);
  Serial.print("LDR (luz): ");
  Serial.println(ldrValue);

  // Leitura do DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Falha na leitura do DHT11");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.print(" °C | Umidade: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  // Verifica condições de emergência (botão de pânico ou gás perigoso)
  if (botaopanicoapertado || mq7Value > LIMIAR_GAS) {
    digitalWrite(LED_RED_PIN, HIGH); // Liga LED vermelho
    Serial.println("ALERTA: Nível de gás perigoso ou botão de emergência pressionado!");
  } else {
    digitalWrite(LED_RED_PIN, LOW); // Desliga LED vermelho
  }

  // Verifica baixa luminosidade para ativar a lanterna
  if (ldrValue < LIMIAR_LDR) {
    digitalWrite(LANTERNA_PIN, HIGH); // Liga a lanterna
    Serial.println("Lanterna ativada (baixa luminosidade detectada).");
  } else {
    digitalWrite(LANTERNA_PIN, LOW); // Desliga a lanterna
  }

  // Aguarda 1 segundo antes de repetir
  delay(1000);
}
