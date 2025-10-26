#include <SPI.h>
#include <RF24.h>

#define MAX_MESSAGE_LEN 200        // Max celková dĺžka správy (v RAM)
#define FRAGMENT_DATA_SIZE 24      // Max znakov textu na fragment
#define MAX_PACKET_SIZE 32         // nRF24L01 limit

RF24 radio(9, 10); // CE, CSN

// Prijímacie buffery
char fullMessageBuffer[MAX_MESSAGE_LEN + 1] = {0};
uint8_t receivedFragments = 0;
uint8_t totalFragments = 0;
uint16_t totalMessageLength = 0;
bool messageInProgress = false;

// Pomocná funkcia: odoslanie jedného fragmentu
void sendFragment(const char* fullMsg, uint16_t totalLen, uint8_t fragIndex, uint8_t totalFrags) {
  char packet[MAX_PACKET_SIZE] = {0};
  int dataStart = fragIndex * FRAGMENT_DATA_SIZE;
  int dataLen = (dataStart + FRAGMENT_DATA_SIZE > totalLen) ? (totalLen - dataStart) : FRAGMENT_DATA_SIZE;

  // Formát: "DLZKA|INDEX|POCET|DATA"
  snprintf(packet, MAX_PACKET_SIZE, "%d|%d|%d|", totalLen, fragIndex, totalFrags);
  strncat(packet, fullMsg + dataStart, dataLen);

  radio.stopListening();
  radio.write(packet, strlen(packet) + 1); // +1 pre \0 (nRF to toleruje)
  radio.startListening();
}

// Spracovanie prijatého paketu
void processPacket(const char* packet) {
  // Rozdeľ podľa '|'
  char* token = strtok((char*)packet, "|");
  if (!token) return;

  uint16_t pktTotalLen = atoi(token);
  token = strtok(NULL, "|");
  if (!token) return;
  uint8_t fragIndex = atoi(token);
  token = strtok(NULL, "|");
  if (!token) return;
  uint8_t pktTotalFrags = atoi(token);
  char* data = strtok(NULL, "\0");
  if (!data) data = "";

  // Kontrola konzistencie
  if (pktTotalLen > MAX_MESSAGE_LEN || pktTotalFrags == 0 || fragIndex >= pktTotalFrags) {
    return;
  }

  // Nová správa?
  if (!messageInProgress || totalMessageLength != pktTotalLen || totalFragments != pktTotalFrags) {
    // Resetujeme buffer
    memset(fullMessageBuffer, 0, sizeof(fullMessageBuffer));
    receivedFragments = 0;
    totalMessageLength = pktTotalLen;
    totalFragments = pktTotalFrags;
    messageInProgress = true;
  }

  // Skopíruj dáta na správne miesto
  int offset = fragIndex * FRAGMENT_DATA_SIZE;
  int dataLen = strlen(data);
  if (offset + dataLen > MAX_MESSAGE_LEN) return;

  memcpy(fullMessageBuffer + offset, data, dataLen);

  // Označ ako prijatý
  // (jednoduché: predpokladáme, že každý fragment príde raz)
  receivedFragments++;

  // Ak máme všetky fragmenty
  if (receivedFragments >= totalFragments) {
    fullMessageBuffer[totalMessageLength] = '\0'; // ukonči reťazec
    Serial.print("Prijatá celá správa: ");
    Serial.println(fullMessageBuffer);
    
    // Reset
    messageInProgress = false;
    receivedFragments = 0;
    totalFragments = 0;
    totalMessageLength = 0;
    memset(fullMessageBuffer, 0, sizeof(fullMessageBuffer));
  }
}

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(0xF0F0F0E1LL);
  radio.openReadingPipe(1, 0xF0F0F0E1LL);
  radio.setPALevel(RF24_PA_HIGH);
  radio.startListening();
  Serial.println("Pripravený na odosielanie/prijímanie. Zadajte správu:");
}

void loop() {
  // Odosielanie: ak je niečo v Serial
  if (Serial.available() > 0) {
    static char inputBuffer[MAX_MESSAGE_LEN + 1];
    int len = Serial.readBytesUntil('\n', inputBuffer, MAX_MESSAGE_LEN);
    if (len > 0) {
      // Odstrániť \r ak je
      if (inputBuffer[len - 1] == '\r') len--;
      inputBuffer[len] = '\0';

      uint16_t totalLen = len;
      uint8_t totalFrags = (totalLen + FRAGMENT_DATA_SIZE - 1) / FRAGMENT_DATA_SIZE;

      Serial.print("Odosielam správu (");
      Serial.print(totalLen);
      Serial.print(" znakov) v ");
      Serial.print(totalFrags);
      Serial.println(" častiach...");

      for (uint8_t i = 0; i < totalFrags; i++) {
        sendFragment(inputBuffer, totalLen, i, totalFrags);
        delay(20); // Krátka pauza pre stabilitu
      }
    }
  }

  // Prijímanie
  if (radio.available()) {
    char packet[MAX_PACKET_SIZE] = {0};
    radio.read(packet, sizeof(packet));
    processPacket(packet);
  }
}