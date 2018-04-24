#include <WebUSB.h>

/**
 * Creating an instance of WebUSBSerial will add an additional USB interface to
 * the device that is marked as vendor-specific (rather than USB CDC-ACM) and
 * is therefore accessible to the browser.
 *
 * The URL here provides a hint to the browser about what page the user should
 * navigate to to interact with the device.
 */
WebUSB WebUSBSerial(1 /* https:// */, "webusb.github.io/arduino/demos/console");

#define Serial WebUSBSerial

const int ledPin = 13;

void setup() {
  while (!Serial) {
    ;
  }
  Serial.begin(9600);
  Serial.write("Sketch begins.\r\n> ");
  Serial.flush();
  pinMode(ledPin, OUTPUT);
}

void loop() {
  if (Serial && Serial.available()) {
    int byte = Serial.read();
    Serial.write(byte);
    if (byte == 'H') {
      Serial.write("\r\nTurning LED on.");
      digitalWrite(ledPin, HIGH);
    } else if (byte == 'L') {
      Serial.write("\r\nTurning LED off.");
      digitalWrite(ledPin, LOW);
    }
    Serial.write("\r\n> ");
    Serial.flush();
  }
}
