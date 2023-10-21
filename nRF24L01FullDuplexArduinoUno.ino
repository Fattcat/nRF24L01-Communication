// ----------------------------------------------------------------------------------------------------------------------------------
// Full Duplex Arduino UNO Code for Transmit and Receive CUSTOM Messsage.
// Created By Dominik Hulin
//         : LINKS :
// GitHub  : github.com/Fattcat
// YouTube : https://www.youtube.com/channel/UCKfToKJFq-uvI8svPX0WzYQ

//         : Components :
// nRF24L01 
// Cost : About 2 €  
// AliExpress Link : https://www.aliexpress.com/item/32517849393.html?spm=a2g0o.order_list.order_list_main.5.53341802zR7CBu
//
// Arduino UNO
// Cost : About 3 €
// AliExpress Link : https://www.aliexpress.com/item/32932088536.html?spm=a2g0o.order_list.order_list_main.15.272c1802A1wYyH
//
// Some Wires for Connect it
// Cost : About 1 €
// AliExpress Link : https://www.aliexpress.com/item/1005005945668553.html?spm=a2g0o.productlist.main.5.3ee96589bktPwT&algo_pvid=10843895-becd-41e7-bb9e-6ba514305112&algo_exp_id=10843895-becd-41e7-bb9e-6ba514305112-2&pdp_npi=4%40dis%21EUR%215.02%211.74%21%21%2137.99%21%21%40211b600e16979140499187979e18b2%2112000034970736149%21sea%21SK%214031371332%21&curPageLogUid=jRvt0BDum3lC
// ----------------------------------------------------------------------------------------------------------------------------------


#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10); // Konfigurácia modulu nRF24L01, pripojeného k pinom 9 a 10
char inputText[32]; // Textový buffer pre vstupný text

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
    char receivedText[32] = ""; // Textový buffer pre prijatý text
    radio.read(receivedText, sizeof(receivedText)); // Prijme správu
    Serial.print("Prijaté: ");
    Serial.println(receivedText); // Vypíšeme prijatý text do terminálu
  }
}
