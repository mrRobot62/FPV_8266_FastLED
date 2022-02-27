//#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
//#define FASTLED_ESP8266_D1_PIN_ORDER



#include <Arduino.h>
#include <FastLED.h>
#include <AceButton.h>
using namespace ace_button;

//  Debug and Test options
#define _DEBUG_
//#define _TEST_


#ifdef _DEBUG_
  #define _PP(a) Serial.print(a);
  #define _PL(a) Serial.println(a);
#else
  #define _PP(a)
  #define _PL(a)
#endif

#define BTN_PIN     9   
#define POT_PIN     A0
#define LED_PIN     2
#define NUM_LEDS    57
#define BRIGHTNESS  50
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_PATTERNS 4

#define POT_MIN 5
#define POT_MAX 40

#include <FastLED.h>

CRGB source1[NUM_LEDS];
CRGB source2[NUM_LEDS];
CRGB output[NUM_LEDS];

uint8_t blendAmount = 0;
uint8_t patternCounter = 0;
uint8_t source1Pattern = 0;
uint8_t source2Pattern = 1;
bool useSource1 = false;

uint8_t pot_val, pot_val_last;

void movingDots(CRGB *LEDarray);
void nextPattern();
void runPattern(uint8_t pattern, CRGB *LEDArray);
void rainbowBeat(CRGB *LEDarray);
void redWhiteBlue(CRGB *LEDarray);
void runFire(CRGB *LEDarray, uint8_t side);

void handleEvent(AceButton*, uint8_t, uint8_t);

CRGBPalette16 firePalette = HeatColors_p;

AceButton button(BTN_PIN);

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(output, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(115200);
  pot_val = pot_val_last = map(analogRead(POT_PIN), 0,1023, POT_MIN, POT_MAX);
  pinMode(BTN_PIN, INPUT_PULLUP); 
  button.init(BTN_PIN, LOW);

  ButtonConfig* buttonConfig = button.getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  // Check if the button was pressed while booting
  if (button.isPressedRaw()) {
    _PL(F("setup(): button was pressed while booting"));
  }
  _PL("FPV 8266 - GateLEDs");
  _PL("LunaX V0.1, (c) 2022")
}

void loop() {
  button.check();

  pot_val = map(analogRead(POT_PIN),0,1023, POT_MIN, POT_MAX);
  
  if (pot_val != pot_val_last) {
    _PP("New PotValue:"); _PL(pot_val);
    pot_val_last = pot_val;
  }

  EVERY_N_MILLISECONDS(10) {
    blend(source1, source2, output, NUM_LEDS, blendAmount);   // Blend between the two sources

    if (useSource1) {
      if (blendAmount < 255) {
        blendAmount++;                   // Blend 'up' to source 2
        //_PL("Blend up");
      }
    } else {
      if (blendAmount > 0) {
        blendAmount--;                     // Blend 'down' to source 1
        //_PL("Blend down");
      }
    }
  }

  EVERY_N_SECONDS(5) {
    //nextPattern();
  }

  runPattern(source1Pattern, source1);                  // Run both patterns simultaneously
  runPattern(source2Pattern, source2);
  
  FastLED.show();
}

void handleEvent(AceButton* /* button */, uint8_t eventType,
    uint8_t buttonState) {
    _PP("handleEvent(): eventType "); _PP(eventType);
    _PP("; ButtonState: "); _PL(buttonState);

    switch(eventType) {
      case AceButton::kEventPressed:
        nextPattern();
        break;
      //case AceButton::kFeatureLongPress:
      //   _PL("Long pressed");
      //  break;
    }
}

void nextPattern() {
  patternCounter = (patternCounter + 1) % NUM_PATTERNS;          // Change the number after the % to the number of patterns you have
  _PP("NextPattern #"); _PL(patternCounter);

  if (useSource1) source1Pattern = patternCounter;    // Determine which source array for new pattern
  else source2Pattern = patternCounter;

  useSource1 = !useSource1;                           // Swap source array for next time around
}

void runPattern(uint8_t pattern, CRGB *LEDArray) {
  switch (pattern) {
    case 0:
      movingDots(LEDArray);
      break;
    case 1:
      rainbowBeat(LEDArray);
      break;
    case 2:
      redWhiteBlue(LEDArray);
      break;
    case 3:
      runFire(LEDArray, 0);
      break;
/*
    case 4:
      //runFire(LEDArray, 1);
      break;
    case 5:
      //runFire(LEDArray, 2);
      break;
*/
    }

}

//------- Put your patterns below -------//

void movingDots(CRGB *LEDarray) {
  uint8_t bpm1 = pot_val;
  uint8_t bpm2 = pot_val * 2;
  uint8_t bpm3 = pot_val * 2;

  uint16_t posBeat  = beatsin16(bpm1, 0, NUM_LEDS - 1, 0, 0);
  uint16_t posBeat2 = beatsin16(bpm2, 0, NUM_LEDS - 1, 0, 0);

  uint16_t posBeat3 = beatsin16(bpm1, 0, NUM_LEDS - 1, 0, 32767);
  uint16_t posBeat4 = beatsin16(bpm2, 0, NUM_LEDS - 1, 0, 32767);


  // Wave for LED color
  uint8_t colBeat  = beatsin8(10, 0, 255, 0, 0);

  LEDarray[(posBeat + posBeat2) / 2]  = CHSV(colBeat, 255, 255);
  LEDarray[(posBeat3 + posBeat4) / 2]  = CHSV(colBeat, 255, 255);

  fadeToBlackBy(LEDarray, NUM_LEDS, 3);
}


void rainbowBeat(CRGB *LEDarray) {
  uint8_t bpm1 = pot_val;
  uint8_t bpm2 = pot_val - (pot_val*50/100);  // 20% from PotValue
  uint16_t beatA = beatsin16(bpm1, 0, 255);
  uint16_t beatB = beatsin16(bpm2, 0, 255);
  fill_rainbow(LEDarray, NUM_LEDS, (beatA+beatB)/2, 8);
}


void redWhiteBlue(CRGB *LEDarray) {
  uint8_t bpm1 = pot_val;

  uint16_t sinBeat   = beatsin16(bpm1, 0, NUM_LEDS - 1, 0, 0);
  uint16_t sinBeat2  = beatsin16(bpm1, 0, NUM_LEDS - 1, 0, 21845);
  uint16_t sinBeat3  = beatsin16(bpm1, 0, NUM_LEDS - 1, 0, 43690);

  LEDarray[sinBeat]   = CRGB::Blue;
  LEDarray[sinBeat2]  = CRGB::Red;
  LEDarray[sinBeat3]  = CRGB::White;
  
  fadeToBlackBy(LEDarray, NUM_LEDS, 10);
}


void runFire(CRGB *LEDarray, uint8_t side) {
  // side
  // 0 = left side, 1 = right side, 2=both sides

  int  a = millis();
  
  int start, end, mid;
  start = 0;
  mid = NUM_LEDS / 2;
  end = NUM_LEDS ;

  for (int i = start ; i < end; i++) {

    // 3D noise, x is constant, we move through time on y and z axis
    // The 60 here will change the scale of the effect, lower is smoother
    // higher is more flickery. The time value for z was in the original code
    // as that was a 2D matrix version. I've left it in here as it looks 
    // nice in 1D too!
    uint8_t noise = inoise8 (0 , i * 60 + a , a / 3);

    // Divides 255 by (NUM_LEDS - 1) and subtracts that amount away from 255 to return 
    // a decreasing number each time e.g. for NUM_LEDS = 18, difference between
    // each point is 15, so:
    // i = 0, math = 255
    // i = 1, math = 240
    // i = 2, math = 225
    // ...
    // i = NUM_LEDS, math =  0
    uint8_t math = abs8(i - (NUM_LEDS-1)) * 255 / (NUM_LEDS-1);
    // Take the noise function from earlier and subtract math from it,
    // so the further along the strip you are, the higher palette index you
    // are likely to get. This results in the base of the fire (white, yellow)
    // being at the far end of the strip
    uint8_t index = qsub8 (noise, math);
    LEDarray[i] = ColorFromPalette (firePalette, index, 255);    

/*
    uint8_t idx_l, idx_r;
    uint8_t math, noise;
    switch (side) {
      case 0:
        noise = inoise8 (0 , i * 60 + a , a / 3);
        math = abs8(i - (NUM_LEDS-1)) * 255 / (NUM_LEDS-1);
        idx_l = qsub8 (noise, math);
        LEDarray[idx_l] = ColorFromPalette (firePalette, idx_l, 255);    
        break;
      case 1:
        idx_r = qsub8 (noise, math);
        idx_r = NUM_LEDS - idx_r;
        LEDarray[idx_r] = ColorFromPalette (firePalette, idx_r, 255);    
      case 2:
        idx_l = qsub8 (noise, math);
        idx_r = qsub8 (noise, math);
        idx_r = NUM_LEDS - idx_r;
        // Set the LED color from the palette
        LEDarray[idx_l] = ColorFromPalette (firePalette, idx_l, 255);    
        LEDarray[idx_r] = ColorFromPalette (firePalette, idx_r, 255);    
    }
*/
  }  
} 

