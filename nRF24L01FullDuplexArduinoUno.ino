// ----------------------------------------------------------------------------------------------------------------------------------
// Full Duplex Arduino UNO Code for Transmit and Receive CUSTOM Messsage.


//         : Components :
// nRF24L01 
// Cost : About 2 €  

// Arduino UNO
// Cost : About 3 €

//
// Some Wires for Connect it
// Cost : About 1 €
 ----------------------------------------------------------------------------------------------------------------------------------


#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // Konfigurácia modulu nRF24L01, pripojeného k pinom 9 a 10

// Obmedzenie pre ODOSLANIE NAJVIAC 32 ZNAKOV !
char inputText[32]; // Textový buffer pre vstupný text
// Obmedzenie pre ODOSLANIE NAJVIAC 32 ZNAKOV !

void setup() {
  Serial.begin(9600); // Inicializácia sériovej komunikácie
  radio.begin();
  radio.openWritingPipe(0xF0F0F0E1LL); // Adresa pre odoslanie
  radio.openReadingPipe(1, 0xF0F0F0E1LL); // Adresa pre prijímanie (rovnaká adresa ako odoslanie)
  radio.setPALevel(RF24_PA_HIGH); // Nastavíme silu signálu na vysokú úroveň
  radio.startListening(); // Spustíme režim prijímania
}

void loop() {
  if (Serial.available() > 0) {
    // Čítame vstupný text zo sériovej komunikácie
    memset(inputText, 0, sizeof(inputText)); // Vynulujeme textový buffer
    int bytesRead = Serial.readBytesUntil('\n', inputText, sizeof(inputText) - 1); // Čítame text až do znaku nového riadku
    inputText[bytesRead] = '\0'; // Pridáme ukončujúci nulový znak

    radio.stopListening(); // Ukončíme režim prijímania
    radio.write(inputText, sizeof(inputText)); // Odošleme vstupný text
    radio.startListening(); // Spustíme režim prijímania znovu
    Serial.print("Odoslané: ");
    Serial.println(inputText); // Vypíšeme odoslaný text do terminálu
  }

  if (radio.available()) {
    
    // Obmedzenie pre PRIJATIE NAJVIAC 32 ZNAKOV !
    char receivedText[32] = ""; // Textový buffer pre prijatý text
    // Obmedzenie pre PRIJATIE NAJVIAC 32 ZNAKOV !
    
    radio.read(receivedText, sizeof(receivedText)); // Prijme správu
    Serial.print("Prijaté: ");
    Serial.println(receivedText); // Vypíšeme prijatý text do terminálu
  }
}
