#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 8);  // CE, CSN pins
const int vibrationMotorPin = 7;  // Blue wire here

const byte address[6] = "000001";  // Same address as the transmitter
const byte channel = 76;

const int packetRecieveDelay = 60 * 1000 * 3 + 1000;  // Three minutes and 1 second
unsigned long startTickTime;

String textString;

void setup() {
  pinMode(vibrationMotorPin, OUTPUT);
  digitalWrite(vibrationMotorPin, HIGH);  // LOW and HIGH values are inverted somehow

  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);  // Set power level
  radio.setChannel(channel);
  radio.setDataRate(RF24_1MBPS);
  radio.startListening();  // Start listening for incoming data
}

void loop() {
  if (radio.available()) {
    char text[4] = "";  // Allocate buffer for incoming data, [32] before
    radio.read(&text, sizeof(text));
    Serial.println(text);
    textString = text;
    if (textString == "0000") {
      // 0000 means that something is wrong
      digitalWrite(vibrationMotorPin, LOW);
      Serial.println("Oh no... Vibrate!");
    } else if (textString == "1111") {
      // 1111 means that everything is fine
      digitalWrite(vibrationMotorPin, HIGH);
      Serial.println("All OK!");
    } else {
      // Packet is not 0000 or 1111
      Serial.print("Corrupted packet: ");
      Serial.println(text);
    }
    startTickTime = millis();
  }

  if (millis() - startTickTime > packetRecieveDelay) {
    // More than 3 times no recieved
    Serial.println("Packets not being recieved for 3 times (3 min).");
    digitalWrite(vibrationMotorPin, HIGH);
  }
}
