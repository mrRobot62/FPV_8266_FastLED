//#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
//#define FASTLED_ESP8266_D1_PIN_ORDER



#include <Arduino.h>
#include <FastLED.h>


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



#define LED_PIN     0
#define NUM_LEDS    57
#define BRIGHTNESS  50
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_PATTERNS 4

#include <FastLED.h>

CRGB source1[NUM_LEDS];
CRGB source2[NUM_LEDS];
CRGB output[NUM_LEDS];

uint8_t blendAmount = 0;
uint8_t patternCounter = 0;
uint8_t source1Pattern = 0;
uint8_t source2Pattern = 1;
bool useSource1 = false;

void movingDots(CRGB *LEDarray);
void nextPattern();
void runPattern(uint8_t pattern, CRGB *LEDArray);
void rainbowBeat(CRGB *LEDarray);
void redWhiteBlue(CRGB *LEDarray);
void runFire(CRGB *LEDarray, uint8_t side);


CRGBPalette16 firePalette = HeatColors_p;


void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(output, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  Serial.begin(115200);

  _PL("FPV 8266 - GateLEDs - RING 2 Pattern");
  _PL("LundaX V0.1, (c) 2022")
}

void loop() {

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
    nextPattern();
  }

  runPattern(source1Pattern, source1);                  // Run both patterns simultaneously
  runPattern(source2Pattern, source2);
  
  FastLED.show();
}
