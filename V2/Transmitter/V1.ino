#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 8);  // CE, CSN pins
const int lightLevelPin = A1;

const int brightnessThreshold = 880;
const int packetSendDelay = 60000;  // 60 seconds

const byte address[5] = {'a', 'b', 'c', 'd', 'e'};
const byte channel = 76;

char stringData[5] = "fine";  // Ensure fixed size
char lastStringData[5] = "fine";
unsigned int packetCheckNoReceived;
unsigned int differentLightSignalNoReceived;
unsigned long lastPacketTime;

void setup() {
  Serial.begin(9600);
  pinMode(lightLevelPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(channel);
  radio.setDataRate(RF24_1MBPS);
  radio.setPayloadSize(5);  // Ensure same payload size as receiver
  radio.setAutoAck(true);
  radio.setRetries(5, 15);
  radio.stopListening();
}

void loop() {
  int lightLevel = analogRead(lightLevelPin);
  if (lightLevel >= brightnessThreshold) {
    strcpy(stringData, "beep");
  } else {
    strcpy(stringData, "fine");
  }

  bool packetCheck = millis() - lastPacketTime > packetSendDelay;
  bool differentLightSignal = strcmp(lastStringData, stringData) != 0;

  if (packetCheck || differentLightSignal) {
    bool received = radio.write(stringData, 5);
    digitalWrite(LED_BUILTIN, HIGH);
    lastPacketTime = millis();

    if (received) {
      Serial.println("[OK] Packet received");
      packetCheckNoReceived = 0;
      differentLightSignalNoReceived = 0;
    } else {
      Serial.println("[ALERT] Packet not received");
      if (packetCheck) packetCheckNoReceived++;
      if (differentLightSignal) differentLightSignalNoReceived++;
    }

    if (packetCheckNoReceived >= 3)
      Serial.println("[MAJOR ALERT] Packet has not been received for 3 times (3 mins).");

    if (differentLightSignalNoReceived >= 3)
      Serial.println("[MAJOR ALERT] Packet has not been received for 3 times with changing light signals!");
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  strcpy(lastStringData, stringData);
  delay(10);
}
