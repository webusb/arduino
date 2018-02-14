#include <WebUSB.h>

WebUSB WebUSBSerial(1, "webusb.github.io/arduino/demos");

#define Serial WebUSBSerial

const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;
int color[3];
int colorIndex;

void setup() {
  while (!Serial) {
    ;
  }
  Serial.begin(9600);
  Serial.write("Sketch begins.\r\n");
  Serial.flush();
  colorIndex = 0;
}

void loop() {
  if (Serial && Serial.available()) {
    color[colorIndex++] = Serial.read();
    if (colorIndex == 3) {
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
      colorIndex = 0;
    }
  }
}
