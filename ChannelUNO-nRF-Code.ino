#include <SPI.h>
#include <RF24.h>

RF24 radio(9, 10);

char inputText[32];

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(0xF0F0F0E1LL);
  radio.openReadingPipe(1, 0xF0F0F0E1LL);
  radio.setPALevel(RF24_PA_HIGH);
  
  radio.setChannel(90);  // Nastavíme modul na kanál 90 (2,49 GHz)
  // You Can Change it from Channel 0 to 125 line this : radio.setChannel(0); - radio.setChannel(125); it will be change TX and RX frequency
  radio.startListening();
}

void loop() {
  if (Serial.available() > 0) {
    memset(inputText, 0, sizeof(inputText));
    int bytesRead = Serial.readBytesUntil('\n', inputText, sizeof(inputText) - 1);
    inputText[bytesRead] = '\0';

    radio.stopListening();
    radio.write(inputText, sizeof(inputText));
    radio.startListening();
    Serial.print("Odoslané: ");
    Serial.println(inputText);
  }

  if (radio.available()) {
    char receivedText[32] = "";
    radio.read(receivedText, sizeof(receivedText));
    Serial.print("Prijaté: ");
    Serial.println(receivedText);
  }
}
