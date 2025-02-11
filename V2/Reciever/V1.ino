#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 8);  // CE, CSN pins
const int vibrationMotorPin = 7;  // Attach the motor here

const byte address[5] = {'a', 'b', 'c', 'd', 'e'};  // Same address as transmitter
const byte channel = 76;                            // Specific frequency channel

// 3 minutes delay + 1 second buffer for timeout detection
const int packetReceiveDelay = 60 * 1000 * 3 + 1000;  
unsigned long startTickTime = 0;

String textString;

void setup() {
  Serial.begin(9600);
  while (!Serial) { /* wait for serial */ }

  pinMode(vibrationMotorPin, OUTPUT);
  // Assuming HIGH means off and LOW means on for your motor
  digitalWrite(vibrationMotorPin, HIGH);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.setChannel(channel);
  radio.setDataRate(RF24_1MBPS);
  radio.setPayloadSize(6);  // Increased buffer size to match transmitter
  radio.startListening();

  startTickTime = millis();
}

void loop() {
  if (radio.available()) {
    char text[6] = "";  // Buffer for incoming data
    radio.read(&text, sizeof(text));
    Serial.println(text);
    textString = text;

    if (textString == "beep") {
      digitalWrite(vibrationMotorPin, LOW);
      Serial.println("Oh no... Vibrate!");
    } else if (textString == "fine") {
      digitalWrite(vibrationMotorPin, HIGH);
      Serial.println("All OK!");
    } else {
      Serial.print("Corrupted packet: ");
      Serial.println(textString);
    }
    
    // Reset timer whenever a packet (good or corrupted) is received
    startTickTime = millis();
  }

  // Check for timeout (no packets received for 3 mins)
  if (millis() - startTickTime > packetReceiveDelay) {
    Serial.println("[WARNING] Packets not being received for 3 times (3 min).");
    digitalWrite(vibrationMotorPin, HIGH);  // Ensure motor is off
  }
}
