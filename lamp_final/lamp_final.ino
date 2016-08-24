#include<Arduino.h>
#include<avr/power.h>
#include<avr/sleep.h>


#define RED               7  // white wire
#define BLUE              6  // purple wire
#define GREEN             5  // gray wire
#define COLOR_POT         2  // pin for potentiometer controlling color temp (red wire)
#define BRIGHT_POT        1  // pin for potentiometer controlling brightness (orange wire)
#define COLOR_SWITCH      3  // pin for color switch (white wire)
#define SLEEP_BTN         8  // pin for sleep button (brown wire)

#define MIN_BRIGHTNESS    10
#define MAX_BRIGHTNESS    180
#define MAX_POT           1023
#define MIN_COLOR_TEMP    1700
#define MAX_COLOR_TEMP    10000
#define FADE_DELAY        10
#define FADE_INCREMENT    1 // must be 1 because uint8_t cannot be negative

// candle states
#define BRIGHT            0 // brightness is at a local maximum
#define UP                1 // brightness is increasing
#define DOWN              2 // brightness is decreasing
#define DIM               3 // brightness is at a local minimum
#define BRIGHT_HOLD       4 // holding at local maximum brightness
#define DIM_HOLD          5 // holding at local minimum brightness

// Percent chance the LED will suddenly fall to minimum brightness
#define RED_LEVEL_BOTTOM_PERCENT    10
// Percent chance the color will hold unchanged after brightening
#define BRIGHT_HOLD_PERCENT         20
// Percent chance the color will hold unchanged after dimming
#define DIM_HOLD_PERCENT            5

 
#define MINVAL(A,B)             (((A) < (B)) ? (A) : (B))
#define MAXVAL(A,B)             (((A) > (B)) ? (A) : (B))

// pwm output levels for red, blue, and green
uint8_t redLevel, blueLevel, greenLevel;

// for candle flame routine
uint8_t state, startRedLevel, endRedLevel;
unsigned long flickerTime;
unsigned long flickerStart;

uint8_t brightness;

void calculateRGBFromTemp(unsigned int colorTemp)
{
	// calculate red value
  colorTemp /= 100;
  double red;
  if (colorTemp <= 66) 
	{
    redLevel = 255;
  } 
	else 
	{
    red = colorTemp - 60;
    redLevel = 329.698727446 * pow(red, -0.1332047592);
  }
  // calculate green value
  double green;
  if (colorTemp <= 66) 
	{
    green = colorTemp;
    greenLevel = 99.4708025861 * log(green) - 161.1195681661;
  } 
	else 
	{
    green = colorTemp - 60;
    greenLevel = 288.1221695283 * pow(green, -0.0755148492);
  }
  // calculate blue value
  double blue;
  if (colorTemp >= 66) 
	{
    blueLevel = 255;
  } 
	else if (colorTemp <= 19)
	{
    blueLevel = 0;
  } 
	else 
	{
    blue = colorTemp - 10;
    blueLevel = 138.5177312231 * log(blue) - 305.0447927307;
  }
}

void updateLEDLevels()
{
  
	OCR0B = redLevel;
	OCR1A = blueLevel;
  OCR1B = greenLevel;
  /*
  analogWrite(RED, redLevel);
	//analogWrite(BLUE, blueLevel);
	analogWrite(GREEN, greenLevel);
  */
}

void adjustColor(uint8_t curBrightness)
{
	unsigned int colorTemp = map(analogRead(COLOR_POT), 0, MAX_POT, 
																MIN_COLOR_TEMP, MAX_COLOR_TEMP);
	calculateRGBFromTemp(colorTemp);
	redLevel = map(redLevel, 0, 255, 0, curBrightness);
	blueLevel = map(blueLevel, 0, 255, 0, curBrightness);
	greenLevel = map(greenLevel, 0, 255, 0, curBrightness);
	updateLEDLevels();
}

void fadeOutLED()
{
	uint8_t fadeBrightness = brightness;
	while (fadeBrightness > 0)
	{
		fadeBrightness -= FADE_INCREMENT;
		adjustColor(fadeBrightness);
		delay(FADE_DELAY);
	}
}

void fadeInLED()
{
	uint8_t fadeBrightness = 0;
	while (fadeBrightness < brightness)
	{
		fadeBrightness += FADE_INCREMENT;
		adjustColor(fadeBrightness);
		delay(FADE_DELAY);
	}
}

void wake()
{
	detachInterrupt(0);
}

void sleep()
{
	fadeOutLED();
	delay(2000); // ensure sleep button is no longer being pressed
	// attach interrupt to wake the controller
	attachInterrupt(0, wake, LOW);
	// put the controller to sleep
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_mode();
	// execution continues from here after waking
	sleep_disable();
  brightness = map(analogRead(BRIGHT_POT), 0, MAX_POT, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
	fadeInLED();
}

void candleFlame(uint8_t curBrightness)
{
	// set flicker brightness limits
	uint8_t maxRedLevel = curBrightness;
	uint8_t bottomRedLevel = curBrightness / 2;
	uint8_t midRedLevel = (maxRedLevel + bottomRedLevel) / 2;
	
	// set flicker time limits
	uint8_t maxChangeTime = curBrightness;
	uint8_t minChangeTime = curBrightness / 10;
	uint8_t maxBrightHoldTime = curBrightness / 2;
	uint8_t maxDimHoldTime = curBrightness / 4;
	
	unsigned long currentTime;
	currentTime = millis();
	
	switch(state)
	{
		case BRIGHT:
			flickerTime = random(maxChangeTime - minChangeTime)
													+ minChangeTime;
			flickerStart = currentTime;
			startRedLevel = endRedLevel;
			if ((startRedLevel > bottomRedLevel) &&
					(random(100) < RED_LEVEL_BOTTOM_PERCENT))
				endRedLevel = random(startRedLevel - bottomRedLevel) + bottomRedLevel;
			else
				endRedLevel = random(startRedLevel - midRedLevel) + midRedLevel;
			state = DOWN;
			break;
		case DIM:
			flickerTime = random(maxChangeTime - minChangeTime) + minChangeTime;
			flickerStart = currentTime;
			startRedLevel = endRedLevel;
			endRedLevel = random(maxRedLevel - startRedLevel) + midRedLevel;
			state = UP;
			break;
		case BRIGHT_HOLD:
		case DIM_HOLD:
			if (currentTime >= (flickerStart + flickerTime))
				state = (state == BRIGHT_HOLD) ? BRIGHT : DIM;
			break;
		case UP:
		case DOWN:
			if (currentTime < (flickerStart + flickerTime))
			{
				redLevel = startRedLevel + ((endRedLevel - startRedLevel) * 
									(((currentTime - flickerStart) * 1.0) / flickerTime));
				redLevel = MAXVAL(MINVAL(redLevel, maxRedLevel), bottomRedLevel);
				greenLevel = (redLevel < midRedLevel) ? redLevel * 3.25 / 8 : redLevel * 3 / 8;
				blueLevel = 0;
				updateLEDLevels();
			}
			else
			{
				redLevel = endRedLevel;
				redLevel = MAXVAL(MINVAL(redLevel, maxRedLevel), bottomRedLevel);
				greenLevel = (redLevel < midRedLevel) ? redLevel * 3.25 / 8 : redLevel * 3 / 8;
				blueLevel = 0;
				updateLEDLevels();
				if (state == DOWN)
				{
					if (random(100) < DIM_HOLD_PERCENT)
					{
						flickerStart = currentTime;
						flickerTime = random(maxDimHoldTime);
						state = DIM_HOLD;
					}
					else
					{
						state = DIM;
					}
				}
				else
				{
					if (random(100) < BRIGHT_HOLD_PERCENT)
					{
						flickerStart = currentTime;
						flickerTime = random(maxBrightHoldTime);
						state = BRIGHT_HOLD;
					}
					else
					{
						state = BRIGHT;
					}
				}
			}
			break;
	}
}

void setup()
{
	pinMode(RED, OUTPUT);
	pinMode(GREEN, OUTPUT);
	pinMode(BLUE, OUTPUT);
	pinMode(COLOR_SWITCH, INPUT);
	pinMode(SLEEP_BTN, INPUT);

  TCCR0A |= (1 << WGM01);
  TCCR0A |= (1 << WGM00);
  TCCR0A |= (1 << COM0B1);
  TCCR0B |= (1 << CS01);
  TCCR0B |= (1 << CS00);

  TIMSK0 |= (1 << TOIE0);

  TCCR1A |= (1 << COM1A1);
  TCCR1A |= (1 << COM1B1);
  TCCR1A |= (1 << WGM10);

  TCCR1B = 0;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11);
  TCCR1B |= (1 << CS10);
}

void loop()
{
	brightness = map(analogRead(BRIGHT_POT), 0, MAX_POT, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
	if (digitalRead(COLOR_SWITCH) == HIGH)
		adjustColor(brightness);
	else
		candleFlame(brightness);
	if (digitalRead(SLEEP_BTN) == LOW)
		sleep();
}
