#include <WebUSB.h>

const WebUSBURL URLS[] = {
  { 1, "webusb.github.io/arduino/demos/" },
  { 0, "localhost:8000" },
};

const uint8_t ALLOWED_ORIGINS[] = { 1, 2 };

WebUSB WebUSBSerial(URLS, 2, 1, ALLOWED_ORIGINS, 2);

#define Serial WebUSBSerial

const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;
int color[3];
int index;

void setup() {
  while (!Serial) {
    ;
  }
  Serial.begin(9600);
  Serial.write("Sketch begins.\r\n");
  index = 0;
}

void loop() {
  if (Serial && Serial.available()) {
    color[index++] = Serial.read();
    if (index == 3) {
      analogWrite(redPin, color[0]);
      analogWrite(greenPin, color[1]);
      analogWrite(bluePin, color[2]);
      Serial.print("Set LED to ");
      Serial.print(color[0]);
      Serial.print(", ");
      Serial.print(color[1]);
      Serial.print(", ");
      Serial.print(color[2]);
      Serial.print(".\r\n");
      Serial.flush();
      index = 0;
    }
  }
}
