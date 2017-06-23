/**
 * SDLbeerlight -
 *
 * Uses bicolor LED panel: https://www.adafruit.com/product/3054
 *
 * This code copies from https://gist.github.com/ladyada/dbb4e736967dd918f140
 *
 * Uses libraries:
 * - HT1632 - https://github.com/gauravmm/HT1632-for-Arduino
 * - FastLED 3.1 - https://github.com/FastLED/FastLED
 * - Bounce2 - https://github.com/thomasfredericks/Bounce2
 *
 * Stupid bluetooth implementation by @otrebla333
 *
 * @todbot
 *
 */

#include <HT1632.h>
#include <font_5x4.h>
#include <font_8x4.h>
#include <images.h>
#include <SoftwareSerial.h>

#define FASTLED_INTERNAL
#define FASTLED_ALLOW_INTERRUPTS 1
#include <FastLED.h>
FASTLED_USING_NAMESPACE

// #include <Adafruit_NeoPixel.h>

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


#define NUM_LEDS 16
#define LED_BRIGHTNESS 100
#define FRAMES_PER_SECOND  60
CRGB leds[NUM_LEDS];
uint8_t gHue;

const byte IMG_BEER [] PROGMEM = { 0b11111111, 0b01101011, 0b01010101, 0b01101011, 0b01010101, 0b11111111, 0b00100010, 0b00011100};
#define IMG_BEER_WIDTH    8
#define IMG_BEER_HEIGHT   8

Bounce debouncer0 = Bounce();
Bounce debouncer1 = Bounce();

int mode = 0;
const int msgMaxLen = 32;
//                       012345678901234567890123
char msg[ msgMaxLen ] = "hello there how are you";
int msgPos = 0;

#define BLUETOOTH_MAX_LENGTH 32   //Has to be 2 char more that expected max message length
                                  //@otrebla333: Should be the same than msgMaxLen?


// define this to use software serial, otherwise it will use hardware serial
#define BT_USE_SWSERIAL

#ifdef BT_USE_SWSERIAL
//Bluetooth software serial
SoftwareSerial bluetooth(2,3); // Arudino (RX,TX) - Bluetooth (TX,RX)
#endif

//String for storing data received from bluetooth
String rxDataBT;                //Received data
String messageDisplay;
//Char arrays for text processing
char messageDisplayChar[BLUETOOTH_MAX_LENGTH];

//BEER LIGHT GLOBAL STATUS VARIABLE; 0 = OFF
bool beerLight = 0;

// Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, pinLedStrip, NEO_GRB + NEO_KHZ800);


void setup ()
{
    Serial.begin(115200);
    Serial.println(F("Hello from SDL light!"));

#ifdef BT_USE_SWSERIAL
    bluetooth.begin(57600);
    bluetooth.println("Hello");
#else
    // make hardware serial act like bluetooth software serial
    #define bluetooth Serial
#endif
    delay(500);

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
    // strip.begin();
    // strip.show(); // Initialize all pixels to 'off'
}

void loop()
{
    doInputs();
    doPanel();
    doLedStrip();

    if(listenBluetooth())
    {
      analyzeMessage();
    }
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
    EVERY_N_MILLISECONDS( 1000 / FRAMES_PER_SECOND ) {

        if( mode ) {
            rainbow();
        }
        else {
            fadeToBlackBy( leds, NUM_LEDS, 20);
        }

        // do some periodic updates
        EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow

        // send the 'leds' array out to the actual LED strip
        FastLED.show();
    }

}

void doPanel()
{
    EVERY_N_MILLISECONDS( 200 ) { //200
        HT1632.renderTarget(0);
        HT1632.clear();          // Clear the previous image contents:

        if( mode ) {
            HT1632.drawText(msg, msgPos, 0, FONT_8X4, FONT_8X4_END, FONT_8X4_HEIGHT);
            //Serial.println(msgPos);
            msgPos--;
            //if( msgPos < -(msgMaxLen * 4) ) { msgPos = 0; } // 4 pixel wide font_end
            if( msgPos < -(messageLength() * 4) ) { msgPos = 32; } // 4 pixel wide font_end
            // if( msgPos < -msgMaxLen ) { msgPos = 0; }
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
    }

}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

bool listenBluetooth()
{
    if( bluetooth.available()>1 ) //Aqui hay num de bytes disponibles up to 64 bytes
    {
      rxDataBT= bluetooth.readStringUntil('\0');

      if (rxDataBT.length()>BLUETOOTH_MAX_LENGTH)
      {
          bluetooth.println("Message too long, muy mal");
      }
      else
      {
        Serial.println(rxDataBT);
      }
      return true;
    }
    else return false;
}

void analyzeMessage()
{
    bool beer=0;
    bool on=0;
    bool off=0;
    bool please=0;
    bool badWords=0;
    bool text=0;
    bool message=0;

    message = lookForWords("text:");

    if(message)
    {
      messageDisplay=rxDataBT.substring(5);
      messageDisplay.toCharArray(msg,messageDisplay.length());
    }
    else
    {
        badWords = lookForWords("fuck,fucking,motherfucker");
        beer=lookForWords("BEER,beer,Beer,cerveza,Cerveza,CERVEZA,light,LIGHT,Light");

        if(badWords)
            {
            bluetooth.println("Wow wow calm down, you are not ready for a beer");
            delay(1000);
            bluetooth.println("BEER LIGHT HAS BEEN DISABLED FOR 5 MINUTES");
            for (int i=0;i<20;i++) delay(1000); //delay 300 sec -> 5 min
            //THIS FOR COMPLETELY STOPS ALL OTHER FUNCTIONS
            bluetooth.println("TRY AGAIN");
            }

        else if(beer)
            {
                on = lookForWords("ON,on,On,encendida,go,GO,Go");
                off = lookForWords("OFF,off,Off,NO,no,No,down");

                if(on)
                {
                    please = lookForWords("PLEASE,please,Please,PORFAVOR,porfavor,Por favor,Por Favor, por favor, hijo de puta");
                    if(please)
                    {
                        bluetooth.println("Beerlight is ON!");
                        beerLight = 1;
                    }
                    else bluetooth.println("That's not how you ask for beer :[");
                }

                else if (off)
                {
                    bluetooth.println("Beerlight is OFF :(");
                    beerLight = 0;
                }

            }
    }
}


//Looks for a word or words (word1,word2,word3,wordN)in the received bluetooth message, returns true or false
bool lookForWords(String multipleWords)
{
  int currentIndex = 0;
  int commaIndex=0;
  String Word;

  //Looks for the first word given in the multipleWords String
  for (int i=0; i<=multipleWords.length();i++)
    {
    commaIndex=multipleWords.indexOf(",",currentIndex);
    if (commaIndex==0)
      {
        if(currentIndex==0)
          {
            Word=multipleWords;
            i=multipleWords.length();
          }
      }
    else
      {
        Word=multipleWords.substring(currentIndex,commaIndex); //If no comma, only 1 word
        currentIndex=commaIndex+1;
      }

    //Checks that BTlength is equal or longer than the word lenght (-2 added for compensation of real length)
    if(rxDataBT.length()-2>=Word.length())
      {
      //If so, looks for the word in the string
      for (int i=0; i<=rxDataBT.length() - Word.length();i++)
        {
          if(rxDataBT.substring(i,Word.length()+i) == Word) return true;
        }
      }
    }
  return false;


}

int messageLength()
{
  int i=0;
  while(msg[i]!='\0')
  {
    i++;
  }
  return i;
}
