#include <Wire.h>
#include "Adafruit_TCS34725.h"

/* Example code for the Adafruit TCS34725 breakout library */

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground */
   
/* Initialise with default values (int time = 2.4ms, gain = 1x) */
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();

/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

void setup(void) {
  Serial.begin(9600);
  
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  // Now we're ready to get readings!
}

void loop(void) {
  uint16_t red, green, blue, clear, colorTemp, lux;
  
  tcs.getRawData(&red, &green, &blue, &clear);
  colorTemp = tcs.calculateColorTemperature(red, green, blue);
  lux = tcs.calculateLux(red, green, blue);
  uint32_t sum = clear;

  int max;
  if (red > green && red > blue) max = red;
  else if (blue > green && blue > red) max = blue;
  else max = green;
  
  red = map(red, 0, max, 0, 255);
  green = map(green, 0, max, 0, 255);
  blue = map(blue, 0, max, 0, 255);
  
  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(red, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(green, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(blue, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(clear, DEC); Serial.print(" ");
  Serial.println(" ");
}
