#include <SPI.h>
#include <Wire.h>
#include <RF24.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // Šírka OLED displeja v pixeloch
#define SCREEN_HEIGHT 64 // Výška OLED displeja v pixeloch
#define OLED_RESET -1    // Reset pin pre OLED (ak je pripojený k VCC)

#define BuzzerPin 2

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RF24 radio(9, 10); // Konfigurácia modulu nRF24L01, pripojeného k pinom 9 a 10

char receivedText[32] = ""; // Textový buffer pre prijatý text
unsigned long lastMessageTime = 0; // Čas, kedy bola naposledy prijatá správa
const unsigned long displayDuration = 15000; // Trvanie zobrazenia správy na OLED (v milisekundách)

void setup() {
  Serial.begin(9600); // Inicializácia sériovej komunikácie

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Adresa displeja môže byť 0x3C alebo 0x3D
    Serial.println(F("OLED displej sa nepodarilo inicializovať"));
    for (;;); // Zastavíme program
  }
  display.clearDisplay(); 
  display.display(); 
  
  radio.begin();
  radio.openReadingPipe(1, 0xF0F0F0E1LL); 
  radio.setPALevel(RF24_PA_HIGH); 
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    memset(receivedText, 0, sizeof(receivedText)); 
    radio.read(receivedText, sizeof(receivedText)); 
    
    tone(BuzzerPin, 100, 100);
    tone(BuzzerPin, 1000);
    delay(300);
    noTone(BuzzerPin);
    
    Serial.print("Prijaté: ");
    Serial.println(receivedText); 

    display.clearDisplay(); 
    display.setTextSize(1); 
    display.setTextColor(SSD1306_WHITE); 
    display.setCursor(0, 0);
    display.print("(!MAX 30!) Prijate:");
    display.setCursor(0, 20);
    display.println(receivedText); 
    display.display(); 

    lastMessageTime = millis(); 
  }

  if (millis() - lastMessageTime > displayDuration && lastMessageTime != 0) {
    display.clearDisplay(); 
    display.display(); 
    lastMessageTime = 0; 
  }
}
