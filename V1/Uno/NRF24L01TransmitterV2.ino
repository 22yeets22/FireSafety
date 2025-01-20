#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 8);  // CE, CSN pins
const int lightLevelPin = A1;

const int brightnessThreshold = 880;  // 0 - 1023 brightness threshold
const int packetSendDelay = 60 * 1000;  // 60 seconds (60 * 1000)

const byte address[6] = "000001";  // Set your own unique address here
const byte channel = 76;

String stringData;
String lastStringData;
unsigned int packetCheckNoRecieved;
unsigned int differentLightSignalNoRecieved;
unsigned long lastPacketTime;
bool recieved;

void setup() {
  pinMode(lightLevelPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_HIGH);  // Set power level
  radio.setChannel(channel);  // Set a specific channel
  radio.setDataRate(RF24_1MBPS);
  radio.stopListening();  // Start transmitting
}

void loop() {
  int lightLevel = analogRead(lightLevelPin);
  // Light level greater than threshold, oh no
  if (lightLevel >= brightnessThreshold) {
    stringData = "1111";  // 1111 means it is ok
    delay(200);
  } else {
    stringData = "0000";  // Not ok signal
  }

  bool packetCheck = millis() - lastPacketTime > packetSendDelay;
  bool differentLightSignal = lastStringData != stringData;
  if (packetCheck || differentLightSignal) {
    const char* dataToSend = stringData.c_str();  // Convert to char array
    
    recieved = radio.write(dataToSend, strlen(dataToSend));  // Transmit message
    digitalWrite(LED_BUILTIN, HIGH);
    lastPacketTime = millis();  // Set the last packet time to current
    if (recieved) {
      Serial.println("[OK] Packet recieved");
      packetCheckNoRecieved = 0;
      differentLightSignalNoRecieved = 0;
    } else {
      Serial.println("[ALERT] Packet not recieved");
      if (packetCheck) packetCheckNoRecieved++;
      if (differentLightSignal) differentLightSignalNoRecieved++;
    }

    if (packetCheckNoRecieved >= 3)
      Serial.println("[MAJOR ALERT] Packet has not been recieved for 3 times (3 mins).");

    if (differentLightSignalNoRecieved >= 3)
      Serial.println("[MAJOR ALERT] Packet has not been recieved for 3 times with changing light signals!");
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  lastStringData = stringData;
}
