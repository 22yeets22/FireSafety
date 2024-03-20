#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// https://forum.arduino.cc/t/simple-nrf24l01-2-4ghz-transceiver-demo/405123
RF24 radio(9, 8);  // CE, CSN pins
const int vibrationMotorPin = 7;  // Blue wire here

const byte address[5] = {'a', 'b', 'c', 'd', 'e'};  // Same address as the transmitter, will ignore different adresses
const byte channel = 76;  // The precise frequency that reciever will listen to

const int packetRecieveDelay = 60 * 1000 * 3 + 1000;  // Three minutes and 1 second
unsigned long startTickTime;

String textString;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    // Wait until Serial is avalible
  }

  pinMode(vibrationMotorPin, OUTPUT);
  digitalWrite(vibrationMotorPin, HIGH);  // LOW and HIGH values are inverted somehow
  
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);  // Set power level
  radio.setChannel(channel);
  radio.setDataRate(RF24_1MBPS);
  radio.setPayloadSize(4);
  radio.startListening();  // Start listening for incoming data
}

void loop() {
  if (radio.available()) {
    char text[4] = "";  // Allocate buffer for incoming data, [32] before
    radio.read(&text, sizeof(text));
    Serial.println(text);
    textString = text;
    if (textString == "beep") {
      // beep means that something is wrong
      digitalWrite(vibrationMotorPin, LOW);
      Serial.println("Oh no... Vibrate!");
    } else if (textString == "fine") {
      // fine means that everything is fine
      digitalWrite(vibrationMotorPin, HIGH);
      Serial.println("All OK!");
    } else {
      // Packet is not beep or fine
      Serial.print("Corrupted packet: ");
      Serial.println(textString);
    }
    startTickTime = millis();
  }

  if (millis() - startTickTime > packetRecieveDelay) {
    // More than 3 times no recieved
    Serial.println("[WARNING] Packets not being recieved for 3 times (3 min).");
    digitalWrite(vibrationMotorPin, HIGH);
  }
}
