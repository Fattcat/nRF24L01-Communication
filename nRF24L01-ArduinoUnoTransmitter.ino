#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // Konfigurácia modulu nRF24L01, pripojeného k pinom 9 a 10
char inputText[32]; // Textový buffer pre vstupný text

void setup() {
  Serial.begin(9600); // Inicializácia sériovej komunikácie
  radio.begin();
  radio.openWritingPipe(0xF0F0F0E1LL); // Adresa pre odoslanie
  radio.setPALevel(RF24_PA_HIGH); // Nastavíme silu signálu na vysokú úroveň
}

void loop() {
  if (Serial.available() > 0) {
    // Čítame vstupný text zo sériovej komunikácie
    memset(inputText, 0, sizeof(inputText)); // Vynulujeme textový buffer
    int bytesRead = Serial.readBytesUntil('\n', inputText, sizeof(inputText) - 1); // Čítame text až do znaku nového riadku
    inputText[bytesRead] = '\0'; // Pridáme ukončujúci nulový znak

    radio.write(inputText, sizeof(inputText)); // Odošleme vstupný text
    Serial.print("Odoslané: ");
    Serial.println(inputText); // Vypíšeme odoslaný text do terminálu
  }
}
