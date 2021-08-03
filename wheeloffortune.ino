#include <Tone.h>
#include <FastLED.h>

#define BUTTON_PIN      2
#define BUZZER_PIN      9
#define LED_STRIP_PIN   3
#define NUM_LEDS       24
#define DEF_BRIGHTNESS 255

#define STATE_IDLE    0
#define STATE_FINDING 1
#define STATE_FOUND   2

CRGB leds[NUM_LEDS];

byte APP_STATE = 0;
int findingSpeed = 10;
byte allowedCurrents[] = {1, 4, 7, 10, 13, 16, 19, 22};
byte winningCurrent = 0;

// Button variables
int buttonState;
int lastButtonState = LOW;
unsigned long lastBtnDebounceTime = 0;
unsigned long btnDebounceDelay = 50;

// Buzzer variables
Tone tone1;

// Effects variables
byte currentLed = 0;
byte foundToneNum = 0;
byte foundBlinkNum = 0;

byte colors[7][3] = {
  {255, 0, 0},
  {0, 255, 0},
  {0, 0, 255},
  {0, 255, 255},
  {255, 51, 255},
  {255, 255, 0},
  {255, 178, 102},
};
byte clrR = 0;
byte clrG = 0;
byte clrB = 0;
bool randomColor = false;

void setup() {
  FastLED.addLeds<WS2812, LED_STRIP_PIN, RGB>(leds, NUM_LEDS);
  
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  tone1.begin(BUZZER_PIN);

  clearLeds();

  randomSeed(analogRead(0));
}

long prevMillisGeneral = 0;
void loop() {
  switch (APP_STATE) {
    case STATE_FINDING:
      runFindingEffect();
      break;
    case STATE_FOUND:
      runFoundEffect();
      break;
    default:
      break;
  }

  // Handle the button press
  int currBtnState = digitalRead(BUTTON_PIN);
  if (currBtnState != lastButtonState) {
    lastBtnDebounceTime = millis();
  }
  if ((millis() - lastBtnDebounceTime) > btnDebounceDelay) {
    if (currBtnState != buttonState) {
      buttonState = currBtnState;
      if (buttonState == LOW) {
        triggerButtonPress();
      }
    }
  }
  lastButtonState = currBtnState;
}

// --- App States ---
long startMillisFinding = 0;
void runFindingEffect() {
  wheelEffect(clrR, clrG, clrB, findingSpeed);

  long timeDiff = (millis() - startMillisFinding);
  if (timeDiff > 1000 && timeDiff <= 5000) {
    if (findingSpeed < 1000 && (millis() - startMillisFinding) % 200 == 0) {
      findingSpeed += 1;
    }
  } else if (timeDiff > 5000 && currentLed == winningCurrent) {
    setAppState(STATE_FOUND);
  }
}

void runFoundEffect() {
  staticEffect(clrR, clrG, clrB);
}

// --- Button ---
void triggerButtonPress() {
  byte cIndex = random(0, 7);
  clrR = colors[cIndex][0];
  clrG = colors[cIndex][1];
  clrB = colors[cIndex][2];

  randomColor = random(0, 100) > 75 ? true : false;

  winningCurrent = allowedCurrents[random(0, 8)];
  setAppState(STATE_FINDING);
}

// --- Buzzer ---
void playBuzzerTone(uint16_t tone, int duration) 
{  
  tone1.play(tone, duration);
}

// --- Effects ---
long lastFindingBuzzerTone = 0;
long lastLedUpdate = 0;
void wheelEffect(byte r, byte g, byte b, int spd) {
  if (millis() - lastLedUpdate > spd) {
    clearLeds();
    if (currentLed == NUM_LEDS) currentLed = 0;
    byte prev = currentLed > 0 ? currentLed - 1 : NUM_LEDS - 1;
    byte next = currentLed < NUM_LEDS - 1 ? currentLed + 1 : 0;
    setLed(prev, r, g, b);
    setLed(currentLed, r, g, b);
    setLed(next, r, g, b);
    currentLed++;
    lastLedUpdate = millis();

    if (millis() - lastFindingBuzzerTone > 60) {
      playBuzzerTone(NOTE_B3, 50);
      lastFindingBuzzerTone = millis();

      if (randomColor) {
        byte cIndex = random(0, 7);
        clrR = colors[cIndex][0];
        clrG = colors[cIndex][1];
        clrB = colors[cIndex][2];
      }
    }
  }
}
long lastStaticBuzzerTone = 0;
long lastStaticBlink = 0;
bool lastStaticState = true;
void staticEffect(byte r, byte g, byte b) {
  if (millis() - lastStaticBlink > 100 && foundBlinkNum < 12) {
    lastStaticState = !lastStaticState;
    lastStaticBlink = millis();
    foundBlinkNum++;
  }

  if (foundBlinkNum >= 12) {
    lastStaticState = true;
  }

  if (lastStaticState) {
    if (currentLed == NUM_LEDS) currentLed = 0;
    byte prev = currentLed > 0 ? currentLed - 1 : NUM_LEDS - 1;
    byte next = currentLed < NUM_LEDS - 1 ? currentLed + 1 : 0;
    setLed(prev, r, g, b);
    setLed(currentLed, r, g, b);
    setLed(next, r, g, b);
  } else {
    clearLeds();
  }

  if (millis() - lastStaticBuzzerTone > 100 && foundToneNum < 12) {
    playBuzzerTone(NOTE_B3, 50);
    lastStaticBuzzerTone = millis();
    foundToneNum++;
  }
}

// --- State functions ---
void setAppState(byte state) {
  clearLeds();
  if (APP_STATE != state) {
    APP_STATE = state;
  }

  if (APP_STATE == STATE_FINDING) {
    findingSpeed = 10;
    startMillisFinding = millis();
  }
  if (APP_STATE == STATE_FOUND) {
    foundToneNum = 0;
    foundBlinkNum = 0;
  }
}

// --- Led utility functions ---
void clearLeds() {
  for (byte i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}
void setLed(byte led, byte r, byte g, byte b) {
  leds[led] = CRGB(r, g, b);
  leds[led].maximizeBrightness(DEF_BRIGHTNESS);
  FastLED.show();
}
void setLed(byte led, byte r, byte g, byte b, byte brightness) {
  leds[led] = CRGB(r, g, b);
  leds[led].maximizeBrightness(brightness);
  FastLED.show();
}
