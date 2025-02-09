#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 8);  // CE, CSN pins
const int vibrationMotorPin = 7;

const byte address[5] = {'a', 'b', 'c', 'd', 'e'};
const byte channel = 76;

const int packetReceiveDelay = 181000;  // 3 minutes + 1 second
unsigned long startTickTime;

char text[5] = "";  // Ensure fixed buffer size

void setup() {
  Serial.begin(9600);
  pinMode(vibrationMotorPin, OUTPUT);
  digitalWrite(vibrationMotorPin, HIGH);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(channel);
  radio.setDataRate(RF24_1MBPS);
  radio.setPayloadSize(5);
  radio.setAutoAck(true);
  radio.startListening();

  startTickTime = millis();
}

void loop() {
  if (radio.available()) {
    radio.read(&text, 5);
    Serial.println(text);

    if (strcmp(text, "beep") == 0) {
      digitalWrite(vibrationMotorPin, LOW);
      Serial.println("Oh no... Vibrate!");
    } else if (strcmp(text, "fine") == 0) {
      digitalWrite(vibrationMotorPin, HIGH);
      Serial.println("All OK!");
    } else {
      Serial.print("Corrupted packet: ");
      Serial.println(text);
    }
    startTickTime = millis();
  }

  if (millis() - startTickTime > packetReceiveDelay) {
    Serial.println("[WARNING] Packets not being received for 3 times (3 min).");
    digitalWrite(vibrationMotorPin, HIGH);
  }
}
