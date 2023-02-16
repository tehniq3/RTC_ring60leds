// http://arduino-kid.ru/blog/chasy-na-svetodiodakh-ws2812
// https://youtu.be/1oqfN3A-YfY
// v.1 - added clock adjustment by Nicu FLORICA (niq_ro)
// v.2 - added outside second line and moved numbers (regenerate others)
// v.2.a - changed to "classical" RTC library from https://github.com/adafruit/RTClib
// v.2.b - line seconds changed in moving point
// RTC - v.1 - replaced ESP8266 with RTC + Arduino (Uno, Nano)

#include "FastLED.h"
#include "RTClib.h"                      //  https://github.com/adafruit/RTClib
RTC_DS3231 rtc;

#define DEBUG_ON
const int NUM_LEDS = 60;
const int DATA_PIN = 6;
//const int photoPin = A0;             // Аналоговый вход с фоторезистором
CRGB LEDs[NUM_LEDS];

#define hplus 2    // pin for increment the hour
#define mplus 3    // pin for increment the minute
#define pinzi 13   // pin for day  
byte ora, minut, secunda;

unsigned long tpschimbare;
unsigned long tpsecunda = 1000;

// Change the colors here if you want.
// Check for reference: https://github.com/FastLED/FastLED/wiki/Pixel-reference#predefined-colors-list
// You can also set the colors with RGB values, for example red:
// CRGB colorHour = CRGB(255, 0, 0);
byte ics = 64;
CRGB fundal = CRGB(ics, ics, ics);
//CRGB fundal = CRGB::PaleTurquoise;
CRGB colorHour = CRGB::Red;
CRGB colorMinute = CRGB::Green;
CRGB colorSecond = CRGB::Blue;
CRGB colorHourMinute = CRGB::Yellow;
CRGB colorHourSecond = CRGB::Magenta;
CRGB colorMinuteSecond = CRGB::Cyan;
CRGB colorAll = CRGB::White;

// Set this to true if you want the hour LED to move between hours (if set to false the hour LED will only move every hour)
#define USE_LED_MOVE_BETWEEN_HOURS true

// Cutoff times for day / night brightness.
#define USE_NIGHTCUTOFF true   // Enable/Disable night brightness
#define USE_AUTODST false       // Enable/Disable automatic chnge of summer time (DST) or manualy (D5 = HIGH -> summer time)
#define MORNINGCUTOFF 8         // When does daybrightness begin?   8am
#define NIGHTCUTOFF 19          // When does nightbrightness begin? 10pm
#define NIGHTBRIGHTNESS 4       // Brightness level from 0 (off) to 255 (full brightness)
#define DAYBRIGHTNESS 40        // Brightness level from 0 (off) to 255 (full brightness)

unsigned long testtime = 10000;
byte contor = 10;   // if delay = 100 (delay x contor = 1000ms = 1s)

void setup() {
  Serial.begin(9600);
  Serial.println("Clock");
/*
pinMode(hplus, INPUT_PULLUP);
pinMode(hplus, INPUT_PULLUP);
*/
  pinMode (hplus, INPUT);
  pinMode (mplus, INPUT);
  digitalWrite (hplus, HIGH);
  digitalWrite (mplus, HIGH);

  pinMode(pinzi, OUTPUT);
  digitalWrite(pinzi, 0);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
                          
  FastLED.delay(2000);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(LEDs, NUM_LEDS); 
  FastLED.setBrightness(2); //Number 0-255
  FastLED.clear();
}

void loop() {
if (contor == 0)
{ 
DateTime now = rtc.now();
ora = now.hour();  // hour
minut = now.minute();   // minute
secunda = now.second(); // second
#ifdef DEBUG_ON
  Serial.print(ora);
  Serial.print(":");
  Serial.print(minut);
  Serial.print(":");
  Serial.print(secunda);
  Serial.print(" -> ");
  Serial.print(" night time: ");
  Serial.print(night());  
  Serial.println();
  Serial.print(ora);
  Serial.print(" > ");
  Serial.print(NIGHTCUTOFF);
  Serial.print(" / < ");
  Serial.print(MORNINGCUTOFF);
  Serial.println(" ? ");
#endif  
}
citirebutoane();
afisare();

contor++;
if (contor > 10) contor = 0;
delay(100);
}  // end main loop

void citirebutoane() {
if (digitalRead(hplus) == LOW)
{
  Serial.print(ora);
  Serial.print("-> ");
  ora  = ora + 1;
  if (ora > 23) ora = 0;
  Serial.println(ora);
  //rtc.setTime(ora, minut, 0);
  rtc.adjust(DateTime(2023, 1, 21, ora, minut, secunda));
  Serial.println("H+");
  delay(500);
  FastLED.clear();
  afisare();
}

if (digitalRead(mplus) == LOW)
{
  Serial.print(minut);
  Serial.print("-> ");
  minut = minut + 1;
  if (minut > 59) minut = 0;
  Serial.println(minut);
  //rtc.setTime(ora, minut, 0);
  rtc.adjust(DateTime(2023, 1, 21, ora, minut, 0));
  Serial.println("M+");
  delay(500);
  FastLED.clear();
  afisare();
}
}

void afisare()
{
  // --------- display ring clock part ------------------------------------
    for (int i=0; i<NUM_LEDS; i++)  // clear all leds
      LEDs[i] = CRGB::Black;
      
for (int i=0; i<12; i++)  // show important points
      {
        LEDs[i*5] = fundal;  
        LEDs[i*5].fadeToBlackBy(25);
      }

    int secunda1 = getLEDMinuteOrSecond(secunda);
    int minut1 = getLEDMinuteOrSecond(minut);
    int ora1 = getLEDHour(ora, minut);
    
    // Set "Hands"
    LEDs[secunda1] = colorSecond;
    LEDs[minut1] = colorMinute;  
    LEDs[ora1] = colorHour;  

    // Hour and min are on same spot
    if (ora1 == minut1)
      LEDs[ora1] = colorHourMinute;

    // Hour and sec are on same spot
    if (ora1 == secunda1)
      LEDs[ora1] = colorHourSecond;

    // Min and sec are on same spot
    if (minut1 == secunda1)
      LEDs[minut1] = colorMinuteSecond;

    // All are on same spot
    if (minut1 == secunda1 && minut1 == ora1)
      LEDs[minut1] = colorAll;

    if (night() && USE_NIGHTCUTOFF == true )
    {
      FastLED.setBrightness (NIGHTBRIGHTNESS); 
      digitalWrite(pinzi, 0);
    }
      else
     {
      FastLED.setBrightness (DAYBRIGHTNESS);  
      digitalWrite(pinzi, 1);
     } 
    FastLED.show();
// ---------end of display ring clock part ------------------------------------
}

byte getLEDHour(byte hours, byte minutes) {
  if (hours > 12)
    hours = hours - 12;

  byte hourLED;
  if (hours <= 5) 
    hourLED = (hours * 5) + 30;
  else
    hourLED = (hours * 5) - 30;

  if (USE_LED_MOVE_BETWEEN_HOURS == true) {
    if        (minutes >= 12 && minutes < 24) {
      hourLED += 1;
    } else if (minutes >= 24 && minutes < 36) {
      hourLED += 2;
    } else if (minutes >= 36 && minutes < 48) {
      hourLED += 3;
    } else if (minutes >= 48) {
      hourLED += 4;
    }
  }

  return hourLED;  
}

byte getLEDMinuteOrSecond(byte minuteOrSecond) {
  if (minuteOrSecond < 30) 
    return minuteOrSecond + 30;
  else 
    return minuteOrSecond - 30;
}

boolean night() {
  if (ora >= NIGHTCUTOFF || ora <= MORNINGCUTOFF) 
    return true;    
    else
    return false;    
}
