// Uses library https://github.com/gauravmm/HT1632-for-Arduino

#include <HT1632.h>
#include <font_5x4.h>
#include <images.h>

int i = 0;

#define DISPLAY_CLK  13
#define DISPLAY_CS1  12     // first panel's CS pin
#define DISPLAY_CS2  7      // second panel's CS pin
#define DISPLAY_WR   10
#define DISPLAY_DATA 9

const byte IMG_BEER [] PROGMEM = { 0b11111111, 0b01101011, 0b01010101, 0b01101011, 0b01010101, 0b11111111, 0b00100010, 0b00011100};
#define IMG_BEER_WIDTH 	 8
#define IMG_BEER_HEIGHT 	 8

void setup () {
  HT1632.setCLK(DISPLAY_CLK);
  HT1632.begin(DISPLAY_CS1, DISPLAY_CS2, DISPLAY_WR, DISPLAY_DATA);
}

void loop () {
  // Clear the previous image contents:
  HT1632.renderTarget(0);
  HT1632.clear();

  // Draw a different image based on the frame number:
  if(i++ % 2 == 0) {
    HT1632.selectChannel(0); // Select the first channel
    HT1632.drawImage(IMG_SPEAKER_A, IMG_SPEAKER_WIDTH,  IMG_SPEAKER_HEIGHT, 0, 4);
    HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 8, 4);
    // HT1632.drawImage(IMG_MUSIC, IMG_MUSIC_WIDTH,  IMG_MUSIC_HEIGHT, 13, 5);
    HT1632.drawImage(IMG_BEER, IMG_BEER_WIDTH,  IMG_BEER_HEIGHT, 13, 5);
    HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 23, 4);
    HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 28, 5);
  } else {
    HT1632.selectChannel(1); // Select the second channel
    HT1632.drawImage(IMG_SPEAKER_B, IMG_SPEAKER_WIDTH,  IMG_SPEAKER_HEIGHT, 0, 4);
    HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 8, 5);
    HT1632.drawImage(IMG_BEER, IMG_BEER_WIDTH,  IMG_BEER_HEIGHT, 13, 4);
    // HT1632.drawImage(IMG_MUSIC, IMG_MUSIC_WIDTH,  IMG_MUSIC_HEIGHT, 13, 4);
    HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 23, 5);
    HT1632.drawImage(IMG_MUSICNOTE, IMG_MUSICNOTE_WIDTH,  IMG_MUSICNOTE_HEIGHT, 28, 4);
  }
  HT1632.render();

/*
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
  delay(200);
}
