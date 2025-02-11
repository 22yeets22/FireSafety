#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 8);  // CE, CSN pins
const int lightLevelPin = A1;

const int brightnessThreshold = 880;        // 0 - 1023 brightness threshold
const int packetSendDelay = 60 * 1000;        // 60 seconds delay between packets

const byte address[5] = {'a', 'b', 'c', 'd', 'e'};  // Matching address for the receiver
const byte channel = 76;                             // Specific frequency channel

String stringData;
String lastStringData;
unsigned int packetCheckNoReceived = 0;
unsigned int differentLightSignalNoReceived = 0;
unsigned long lastPacketTime = 0;
bool received;

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* wait for serial */ }

  pinMode(lightLevelPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setChannel(channel);
  radio.setDataRate(RF24_1MBPS);
  radio.setPayloadSize(6);  // Increased payload to fit "beep" or "fine" with null terminator
  radio.setRetries(2, 3);
  radio.stopListening();
}

void loop() {
  int lightLevel = analogRead(lightLevelPin);

  // Determine status based on brightness
  if (lightLevel >= brightnessThreshold) {
    stringData = "beep";  // "beep" indicates a problem
  } else {
    stringData = "fine";  // "fine" indicates all is good
  }

  bool packetCheck = millis() - lastPacketTime > packetSendDelay;
  bool differentLightSignal = (lastStringData != stringData);

  // Send packet if it's time or if the signal changed
  if (packetCheck || differentLightSignal) {
    const char* dataToSend = stringData.c_str();
    // Send including the null terminator for safety
    received = radio.write(dataToSend, strlen(dataToSend) + 1);
    
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

  lastStringData = stringData;
}
