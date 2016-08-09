#define RED 9
#define BLUE 11
#define GREEN 10
#define MAX_BRIGHT 255
#define POT A3

int redLevel;
int blueLevel;
int greenLevel;

void setup() {
  for (int i = 9; i < 12; i++){
    pinMode(i, OUTPUT);
    analogWrite(i, 10);
  }
  Serial.begin(9600);
}

void loop() {
  int potRead; // potentiometer reading
  potRead = analogRead(POT); 
  unsigned int colorTemp = map(potRead, 0, 1023, 1700, 10000); // map potentiometer reading to color temp range
  calculateRGB(colorTemp);
  redLevel = map(redLevel, 0, 255, 0, 25);
  blueLevel = map(blueLevel, 0, 255, 0, 25);
  greenLevel = map(greenLevel, 0, 255, 0, 25);
  analogWrite(RED, redLevel);
  analogWrite(GREEN, greenLevel);
  analogWrite(BLUE, blueLevel);
  //Serial.print("Color Temperature: ");
  //Serial.println(colorTemp);
  //Serial.print("Sensor Reading: ");
  //Serial.println(potRead);
  //Serial.print("red value: ");
  //Serial.println(redLevel);
  //Serial.print("green value: ");
  //Serial.println(greenLevel);
  //Serial.print("blue value: ");
  //Serial.println(blueLevel);
  //delay(2000);
}

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
