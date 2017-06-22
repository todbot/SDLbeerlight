/**
 * SDLbeerlight -
 *
 * Uses bicolor LED panel: https://www.adafruit.com/product/3054
 *
 * This code copies from https://gist.github.com/ladyada/dbb4e736967dd918f140
 *
 * Uses library https://github.com/gauravmm/HT1632-for-Arduino
 *
 * @todbot
 *
 */

#include <HT1632.h>
#include <font_5x4.h>
#include <font_8x4.h>
#include <images.h>

#define FASTLED_INTERNAL
#include <FastLED.h>
FASTLED_USING_NAMESPACE

#include <Bounce2.h>

int i = 0;

const int pinDisplayCLK = 13;
const int pinDisplayCS1 = 12;
const int pinDisplayCS2 = 12; // unused
const int pinDisplayWR  = 10;
const int pinDisplayDATA = 9;
const int pinLedStrip    = 7;
const int pinIn0 = A0;
const int pinIn1 = A1;

// #define LED_PIN 7
// #define DISPLAY_CLK  13
// #define DISPLAY_CS1  12     // first panel's CS pin
// #define DISPLAY_CS2  7      // second panel's CS pin
// #define DISPLAY_WR   10
// #define DISPLAY_DATA 9

#define NUM_LEDS 16
#define LED_BRIGHTNESS 100
#define FRAMES_PER_SECOND  120
CRGB leds[NUM_LEDS];
uint8_t gHue;

const byte IMG_BEER [] PROGMEM = { 0b11111111, 0b01101011, 0b01010101, 0b01101011, 0b01010101, 0b11111111, 0b00100010, 0b00011100};
#define IMG_BEER_WIDTH    8
#define IMG_BEER_HEIGHT   8

Bounce debouncer0 = Bounce();
Bounce debouncer1 = Bounce();

int mode = 0;
const int msgMaxLen = 32;
char msg[ msgMaxLen ] = "hello there how are you";
int msgPos = 0;

void setup ()
{
    Serial.begin(115200);
    Serial.println(F("Hello from SDL light!"));

    pinMode(pinIn0, INPUT_PULLUP);
    pinMode(pinIn1, INPUT_PULLUP);
    debouncer0.attach(pinIn0);
    debouncer0.interval(50); // interval in ms
    debouncer1.attach(pinIn1);
    debouncer1.interval(50); // interval in ms

    HT1632.setCLK(pinDisplayCLK);
    HT1632.begin(pinDisplayCS1, pinDisplayCS2, pinDisplayWR, pinDisplayDATA);
    // HT1632.begin(DISPLAY_CS1, DISPLAY_CS2, DISPLAY_WR, DISPLAY_DATA);

    FastLED.addLeds<WS2812, pinLedStrip,GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(LED_BRIGHTNESS);     // set master brightness control
}

void loop()
{
    doInputs();
    doPanel();
    doLedStrip();
}

void doInputs()
{
    debouncer0.update();
    debouncer1.update();

    if( debouncer0.fell() ) {
        Serial.print(F("Button0: push! mode:")); Serial.println(mode);
        mode = !mode;
    }

    if( debouncer1.fell() ) {
        Serial.print(F("Button1: push! mode:")); Serial.println(mode);
    }

}

void doLedStrip()
{
    if( mode ) {
        // Call the current pattern function once, updating the 'leds' array
        rainbow();
    }
    else {
        fadeToBlackBy( leds, NUM_LEDS, 10);
    }

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000/FRAMES_PER_SECOND);

    // do some periodic updates
    EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

void doPanel()
{
    EVERY_N_MILLISECONDS( 300 ) {
        HT1632.renderTarget(0);
        HT1632.clear();          // Clear the previous image contents:

        if( mode ) {
            // char msgout[9];
            // strncpy(msgout, msg+msgPos,8); // display is 8 characters across
            // msgPos++;
            // if( msgPos == msgMaxLen || msgout[0] == '\0' ) { msgPos = 0; }

            // HT1632.drawText(msgout, 0,0, FONT_5X4, FONT_5X4_END, FONT_5X4_HEIGHT);
            // HT1632.drawText(msgout, 0,0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
            HT1632.drawText(msg, msgPos, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
            msgPos--; if( msgPos < -msgMaxLen ) { msgPos = 0; }

            // Draw a different image based on the frame number:
            if(i++ % 2 == 0) {
                HT1632.selectChannel(0); // Select the first channel (green)
                HT1632.drawImage(IMG_SPEAKER_A, IMG_SPEAKER_WIDTH,  IMG_SPEAKER_HEIGHT, 0, 8);
                HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 8, 8);
                // HT1632.drawImage(IMG_MUSIC, IMG_MUSIC_WIDTH,  IMG_MUSIC_HEIGHT, 13, 5);
                HT1632.drawImage(IMG_BEER, IMG_BEER_WIDTH,  IMG_BEER_HEIGHT, 13, 8);
                HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 23, 8);
                HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 28, 9);

                HT1632.selectChannel(1); // Select the first channel (green)
                HT1632.drawImage(IMG_BEER, IMG_BEER_WIDTH,  IMG_BEER_HEIGHT, 13, 8);

            } else {
                HT1632.selectChannel(1); // Select the second channel (red)
                HT1632.drawImage(IMG_SPEAKER_B, IMG_SPEAKER_WIDTH,  IMG_SPEAKER_HEIGHT, 0, 8);
                HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 8, 9);
                HT1632.drawImage(IMG_BEER, IMG_BEER_WIDTH,  IMG_BEER_HEIGHT, 13, 8);
                // HT1632.drawImage(IMG_MUSIC, IMG_MUSIC_WIDTH,  IMG_MUSIC_HEIGHT, 13, 4);
                HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 23, 9);
                HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 28, 8);
            }
        }
        HT1632.render();

    /*
       delay(2000);

       HT1632.renderTarget(0);
       HT1632.clear();
       if (~i & 0b01) { // On frames 1 and 3:
        HT1632.selectChannel(0); // Select the first channel
        // Draw a heart:
        HT1632.drawImage(IMG_HEART, IMG_HEART_WIDTH,  IMG_HEART_HEIGHT, 19, 1);
       }
       if (~i & 0b10) { // On frames 2 and 3:
        HT1632.selectChannel(1); // Select the second channel
        // Draw a heart:
        HT1632.drawImage(IMG_HEART, IMG_HEART_WIDTH,  IMG_HEART_HEIGHT, 19, 1);
       }
       HT1632.drawText("Ada", 2, 2, FONT_5X4, FONT_5X4_END, FONT_5X4_HEIGHT);
       HT1632.drawText("fruit !", 2, 9, FONT_5X4, FONT_5X4_END, FONT_5X4_HEIGHT);

       // Perform the drawing:
       HT1632.render();
     */
 }

}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}
