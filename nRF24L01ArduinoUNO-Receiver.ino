#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // Konfigurácia modulu nRF24L01, pripojeného k pinom 9 a 10
int ledPin = 13; // Pripojenie LED diódy k pinu 13

void setup() {
  Serial.begin(9600); // Inicializácia sériovej komunikácie
  radio.begin();
  radio.openReadingPipe(1, 0xF0F0F0E1LL); // Adresa pre prijímanie
  radio.startListening();
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Vypne LED diódu na začiatku
}

void loop() {
  if (radio.available()) {
    char receivedText[32] = ""; // Textový buffer pre prijatý text
    radio.read(receivedText, sizeof(receivedText)); // Prijme správu
    Serial.print("Prijaté: ");
    Serial.println(receivedText); // Vypíšeme prijatý text do terminálu
    if (strcmp(receivedText, "Ahoj") == 0) {
      // Ak bola prijatá správa "Ahoj", rozsvietime LED diódu 3x
      for (int i = 0; i < 3; i++) {
        digitalWrite(ledPin, HIGH);
        delay(500);
        digitalWrite(ledPin, LOW);
        delay(500);
      }
    }
  }
}
