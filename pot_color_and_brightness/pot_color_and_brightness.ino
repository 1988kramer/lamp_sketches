#define RED 9 // white wire
#define BLUE 11 // purple wire
#define GREEN 10 // gray wire
#define MAX_BRIGHT 255
#define COLOR_POT A3 // pin for potentiometer controlling color temp (red wire)
#define BRIGHT_POT A2 // pin for potentiometer controlling brightness (brown wire)
#define LED_PIN 4 // pin for transistor controlling power to LED driver

// pwm output levels for red, blue, and green
uint8_t redLevel, blueLevel, greenLevel; 

// analog readings from the color and brightness potentiometer
int colorPotLevel, brightPotLevel;

void setup() {
  for (int i = 9; i < 12; i++){
    pinMode(i, OUTPUT);
  }
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  colorPotLevel = analogRead(COLOR_POT); 
  brightPotLevel = map(analogRead(BRIGHT_POT), 0, 1023, 0, 255);
  if (colorPotLevel < 5){
    candleFlame();
  } else {
    adjustColor();
  }
}

// change color levels 
void adjustColor() {
  unsigned int colorTemp = map(colorPotLevel, 0, 1023, 1700, 10000); // map potentiometer reading to color temp range
  // calculate RGB levels (0-255) from given color temperature
  calculateRGB(colorTemp);
  // adjust brightness by scaling RGB levels to brightPotLevel
  redLevel = map(redLevel, 0, 255, 0, brightPotLevel);
  blueLevel = map(blueLevel, 0, 255, 0, brightPotLevel);
  greenLevel = map(greenLevel, 0, 255, 0, brightPotLevel);
  analogWrite(BLUE, blueLevel);
  analogWrite(RED, redLevel);
  analogWrite(GREEN, greenLevel);
}

// creates a candle flame effect by holding the red level
// constant and randomly varying the green level
void candleFlame() {
  analogWrite(RED, brightPotLevel);
  uint8_t lastGreen = greenLevel;
  greenLevel = random(brightPotLevel/3) + (brightPotLevel/3);
  // delay longer if brightness is lower and shorter if 
  // brightness is higher
  int delayTime = (355 - brightPotLevel) / 100;
  // fade to new green level
  while (lastGreen != greenLevel) {
    if (lastGreen < greenLevel) lastGreen++;
    else if (lastGreen > greenLevel) lastGreen--;
    analogWrite(GREEN, lastGreen);
    delay(delayTime);
  }
  delay(random(100));
}

// calculate new RGB values from given color temperature
// RGB values are unsigned 8 bit ints (0 - 255)
void calculateRGB(unsigned int colorTemp) {
  // calculate red value
  colorTemp /= 100;
  double red;
  if (colorTemp <= 66) {
    redLevel = 255;
  } else {
    red = colorTemp - 60;
    redLevel = 329.698727446 * pow(red, -0.1332047592);
    if (redLevel < 0) redLevel = 0;
    if (redLevel > 255) redLevel = 0;
  }
  // calculate green value
  double green;
  if (colorTemp <= 66) {
    green = colorTemp;
    greenLevel = 99.4708025861 * log(green) - 161.1195681661;
    if (greenLevel < 0) greenLevel = 0;
    if (greenLevel > 255) greenLevel = 255;
  } else {
    green = colorTemp - 60;
    greenLevel = 288.1221695283 * pow(green, -0.0755148492);
    if (greenLevel < 0) greenLevel = 0;
    if (greenLevel > 255) greenLevel = 255;
  }
  // calculate blue value
  double blue;
  if (colorTemp >= 66) {
    blueLevel = 255;
  } else if (colorTemp <= 19){
    blueLevel = 0;
  } else {
    blue = colorTemp - 10;
    blueLevel = 138.5177312231 * log(blue) - 305.0447927307;
    if (blueLevel < 0) blueLevel = 0;
    if (blueLevel > 255) blueLevel = 255;
  }
}
