#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RF24.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

int Keyboard = A7;
int New_X = 0;
int Old_X = 0;
int New_Y = 0;
int Old_Y = 0;
int Key_read = 0;
int Prev_Key_read = 1023;
boolean Key_pressed = false;
String To_Transmit = "";
int To_Transmit_Length = 0;

#define OLED_RESET 4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char Letters[3][9] = {
  "ABCDEFGHI",
  "JKLMNOPQR",
  "STUVWXYZ_"
};

RF24 radio(9, 10);
char inputText[32];
boolean messageReceived = false;
boolean sendMessage = false;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(0xF0F0F0E1LL);
  radio.openReadingPipe(1, 0xF0F0F0E1LL);
  radio.setPALevel(RF24_PA_HIGH);
  radio.startListening();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;) {}
  }

  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
  display.fillRect(0, 0, 128, 15, SSD1306_INVERSE);
  display.drawRect(110, 2, 16, 12, SSD1306_BLACK);
  display.setTextSize(1);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(113, 4);
  display.println("OK");
  display.display();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < 9; i++) {
      display.setCursor(i * 12 + 2 * i + 1, j * 16 + 17);
      display.println(Letters[j][i]);
      delay(10);
      display.display();
    }
  }
  display.fillRect(0, 16, 12, 16, SSD1306_INVERSE);
  display.display();
}

void Highlight_letter(int X, int X_Old, int Y, int Y_Old) {
  display.fillRect(X * 12 + 2 * X, Y * 16 + 16, 12, 16, SSD1306_INVERSE);
  display.fillRect(X_Old * 12 + 2 * X_Old, Y_Old * 16 + 16, 12, 16, SSD1306_INVERSE);
  display.display();
}

void displayReceivedMessage() {
  display.setTextSize(1);
  display.setCursor(3, 1);
  display.setTextColor(SSD1306_BLACK);
  display.fillRect(0, 0, 100, 15, SSD1306_WHITE);
  display.println(To_Transmit);
  display.display();
  while (true) {
    Key_read = analogRead(Keyboard);
    if (Key_read < 10) {
      display.clearDisplay();
      To_Transmit = "";
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      for (int j = 0; j < 3; j++) {
        for (int i = 0; i < 9; i++) {
          display.setCursor(i * 12 + 2 * i + 1, j * 16 + 17);
          display.println(Letters[j][i]);
          delay(10);
          display.display();
        }
      }
      display.fillRect(0, 16, 12, 16, SSD1306_INVERSE);
      display.display();
      break;
    }
  }
}

void loop() {
  if (messageReceived) {
    displayReceivedMessage();
    messageReceived = false;
  }

  Key_read = analogRead(Keyboard);
  if (Prev_Key_read > 1000 && Key_read < 1000) {
    Key_pressed = true;
    if (Key_read < 10 && Old_X > 0) New_X = Old_X - 1;
    if (Key_read > 160 && Key_read < 170 && Old_X < 8) New_X = Old_X + 1;
    if (Key_read > 25 && Key_read < 35 && Old_Y > -1) New_Y = Old_Y - 1;
    if (Key_read > 80 && Key_read < 90 && Old_Y < 2) New_Y = Old_Y + 1;
    if (Key_read > 350 && Key_read < 360) {
      if (New_Y != -1) {
        To_Transmit += Letters[New_Y][New_X];
        To_Transmit_Length++;
        display.setTextSize(1);
        display.setCursor(3, 1);
        display.setTextColor(SSD1306_BLACK);
        display.fillRect(0, 0, 100, 15, SSD1306_WHITE);
        display.println(To_Transmit);
        display.display();
      } else {
        for (int i = 1; i < 9; i++) {
          display.fillRect(0, 0, 128, 15, SSD1306_INVERSE);
          delay(300);
          display.display();
        }
      }
    }

    if (New_Y == -1 && Old_Y == 0) {
      display.fillRect(110, 2, 16, 12, SSD1306_INVERSE);
      display.fillRect(Old_X * 12 + 2 * Old_X, Old_Y * 16 + 16, 12, 16, SSD1306_INVERSE);
    }
    if (New_Y == 0 && Old_Y == -1) {
      display.fillRect(110, 2, 16, 12, SSD1306_INVERSE);
      display.fillRect(New_X * 12 + 2 * New_X, New_Y * 16 + 16, 12, 16, SSD1306_INVERSE);
      Prev_Key_read = Key_read;
      Old_X = New_X;
      Old_Y = New_Y;
    }
    if ((Old_X != New_X || Old_Y != New_Y) && Old_Y != -1) {
      if (New_Y != -1) Highlight_letter(New_X, Old_X, New_Y, Old_Y);
      Old_X = New_X;
      Old_Y = New_Y;
    }
  }
  display.display();
  Prev_Key_read = Key_read;

  if (sendMessage) {
    radio.stopListening();
    radio.write(To_Transmit.c_str(), To_Transmit_Length + 1);
    radio.startListening();
    Serial.print("Odesláno: ");
    Serial.println(To_Transmit);
    sendMessage = false;
  }

  if (radio.available()) {
    char receivedText[32] = "";
    radio.read(receivedText, sizeof(receivedText));
    Serial.print("Přijato: ");
    Serial.println(receivedText);
    To_Transmit = receivedText;
    To_Transmit_Length = strlen(receivedText);
    messageReceived = true;
  }
}

void serialEvent() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') {
      sendMessage = true;
    } else {
      To_Transmit += c;
      display.setTextSize(1);
      display.setCursor(3, 1);
      display.setTextColor(SSD1306_BLACK);
      display.fillRect(0, 0, 100, 15, SSD1306_WHITE);
      display.println(To_Transmit);
      display.display();
    }
  }
}
