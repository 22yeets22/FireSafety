#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 8);  // CE, CSN pins

const byte address[6] = "000001"; // Same address as the transmitter
const byte channel = 76;

void setup() {
  pinMode(7, OUTPUT);
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
    char text[32] = "";  // Allocate buffer for incoming data
    radio.read(&text, sizeof(text));
    Serial.println(text);  // Print received message
    if (text == "on") {
      digitalWrite(7, HIGH);
    } else {
      digitalWrite(7, LOW);
    }
  }
}
