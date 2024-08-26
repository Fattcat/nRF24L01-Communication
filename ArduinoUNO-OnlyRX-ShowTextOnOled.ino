#include <SPI.h>
#include <Wire.h>
#include <RF24.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // Šírka OLED displeja v pixeloch
#define SCREEN_HEIGHT 64 // Výška OLED displeja v pixeloch
#define OLED_RESET -1    // Reset pin pre OLED (ak je pripojený k VCC)

// Vytvárame objekt pre OLED displej
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

RF24 radio(9, 10); // Konfigurácia modulu nRF24L01, pripojeného k pinom 9 a 10

char receivedText[32] = ""; // Textový buffer pre prijatý text

unsigned long lastMessageTime = 0; // Čas, kedy bola naposledy prijatá správa
const unsigned long displayDuration = 15000; // Trvanie zobrazenia správy na OLED (v milisekundách)

void setup() {
  Serial.begin(9600); // Inicializácia sériovej komunikácie

  // Inicializácia OLED displeja
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Adresa displeja môže byť 0x3C alebo 0x3D
    Serial.println(F("OLED displej sa nepodarilo inicializovať"));
    for (;;); // Zastavíme program
  }
  display.clearDisplay(); // Vymažeme displej
  display.display(); // Aktualizujeme displej
  
  // Inicializácia RF modulu
  radio.begin();
  radio.openReadingPipe(1, 0xF0F0F0E1LL); // Adresa pre prijímanie
  radio.setPALevel(RF24_PA_HIGH); // Nastavíme silu signálu na vysokú úroveň
  radio.startListening(); // Spustíme režim prijímania
}

void loop() {
  // Kontrola, či je k dispozícii nová správa
  if (radio.available()) {
    memset(receivedText, 0, sizeof(receivedText)); // Vyčistíme buffer pre prijatý text
    radio.read(receivedText, sizeof(receivedText)); // Prijmeme správu

    Serial.print("Prijaté: ");
    Serial.println(receivedText); // Vypíšeme prijatý text do terminálu

    // Zobrazíme prijatý text na OLED displeji
    display.clearDisplay(); // Vymažeme displej
    display.setTextSize(1); // Nastavíme veľkosť textu na 1
    display.setTextColor(SSD1306_WHITE); // Nastavíme farbu textu na bielu
    display.setCursor(0, 0); // Nastavíme kurzor na začiatok
    display.println(receivedText); // Vypíšeme text
    display.display(); // Aktualizujeme displej

    lastMessageTime = millis(); // Uložíme čas prijatia správy
  }

  // Skontrolujeme, či uplynulo 15 sekúnd od prijatia poslednej správy
  if (millis() - lastMessageTime > displayDuration && lastMessageTime != 0) {
    display.clearDisplay(); // Vymažeme displej
    display.display(); // Aktualizujeme displej
    lastMessageTime = 0; // Resetujeme čas prijatia správy, aby sa displej nevymazával opakovane
  }
}
