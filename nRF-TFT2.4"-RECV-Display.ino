#include <SPI.h>
#include <RF24.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS     10
#define TFT_RST    9
#define TFT_DC     8

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

RF24 radio(7, 6); // Pripojenie nRF24L01 k pinom 7 a 6 na Arduine Uno

// Obmedzenie pre ODOSLANIE NAJVIAC 32 ZNAKOV !
char inputText[32]; // Textový buffer pre vstupný text
// Obmedzenie pre ODOSLANIE NAJVIAC 32 ZNAKOV !

void setup() {
  Serial.begin(9600); // Inicializácia sériovej komunikácie

  pinMode(5, OUTPUT); // Pin pre úrovňovú konverziu z 5V na 3.3V pre TFT displej
  digitalWrite(5, HIGH); // Nastavenie pinu na výstupný a logická 1

  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(1); // Nastavenie rotácie displeja na horizontálnu

  tft.fillScreen(ST77XX_BLACK); // Nastavenie čierneho pozadia

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

    // Vymažeme starú správu
    tft.fillRect(10, 10, TFT_WIDTH - 20, 100, ST77XX_BLACK);

    // Zobrazíme novú správu
    tft.drawRect(10, 10, TFT_WIDTH - 20, 100, ST77XX_BLUE); // Nakreslíme obdĺžnik
    tft.setTextColor(ST77XX_ORANGE); // Nastavíme oranžovú farbu textu
    tft.setTextSize(1); // Nastavíme veľkosť písma
    tft.setCursor(20, 20); // Nastavíme pozíciu textu
    tft.print(inputText); // Vypíšeme prijatý text na displej
  }

  if (radio.available()) {

    // Obmedzenie pre PRIJATIE NAJVIAC 32 ZNAKOV !
    char receivedText[32] = ""; // Textový buffer pre prijatý text
    // Obmedzenie pre PRIJATIE NAJVIAC 32 ZNAKOV !

    radio.read(receivedText, sizeof(receivedText)); // Prijme správu
    Serial.print("Prijaté: ");
    Serial.println(receivedText); // Vypíšeme prijatý text do terminálu

    // Vymažeme starú správu
    tft.fillRect(10, 10, TFT_WIDTH - 20, 100, ST77XX_BLACK);

    // Zobrazíme novú správu
    tft.drawRect(10, 10, TFT_WIDTH - 20, 100, ST77XX_BLUE); // Nakreslíme obdĺžnik
    tft.setTextColor(ST77XX_ORANGE); // Nastavíme oranžovú farbu textu
    tft.setTextSize(1); // Nastavíme veľkosť písma
    tft.setCursor(20, 20); // Nastavíme pozíciu textu
    tft.print(receivedText); // Vypíšeme prijatý text na displej
  }
}
