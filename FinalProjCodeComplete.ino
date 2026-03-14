/*
    Skyrim main theme on dual buzzer player

    Hardware required:
    1x Adafruit flora
    2x sewable Buzzers (I used the ones from Lilypad Protosnap Pluses)
      -> one goes on pin 6 (the melody)
      -> one goes on pin 3 (accompaniment)
    1x sewable button (I also took this from the protosnap)
      -> goes on pin 9
    1x slide switch
      -> goes on pin 10
    1x Neopixel (I used the Flora RGB V3)
      -> pin12

    Testing:
    - Press button: both buzzers should alternate, producing melody + accompaniment
    - Disconnect pin 6 buzzer: only accompaniment plays on its turns, silence on melody turns
    - Disconnect pin 3 buzzer: only melody plays on its turns, silence on accompaniment turns
    - Slide switch ON: NeoPixel should light up
    - Slide switch OFF: NeoPixel should turn off
*/


#include <Adafruit_NeoPixel.h>

// C minor scale notes, octaves 3-6
// Octave 3
#define NOTE_C3  131
#define NOTE_D3  147
#define NOTE_Eb3 156
#define NOTE_F3  175
#define NOTE_G3  196
#define NOTE_Ab3 208
#define NOTE_Bb3 233

// Octave 4
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_Eb4 311
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_Ab4 415
#define NOTE_Bb4 466

// Octave 5
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_Eb5 622
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_Ab5 831
#define NOTE_Bb5 932

// Octave 6
#define NOTE_C6  1047
#define NOTE_D6  1175
#define NOTE_Eb6 1245
#define NOTE_F6  1397
#define NOTE_G6  1568
#define NOTE_Ab6 1661
#define NOTE_Bb6 1865

// Skyrim main theme melody (octaves 4-5)
int melody[] = {
  NOTE_C4, NOTE_D4, NOTE_Eb4, NOTE_Eb4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_Bb4, 
  NOTE_F4, NOTE_Eb4, NOTE_D4, NOTE_C4, NOTE_C4, NOTE_D4, NOTE_Eb4, NOTE_Eb4, 
  NOTE_F4, NOTE_G4, NOTE_G4, NOTE_Bb4, NOTE_C5, NOTE_Bb4, NOTE_D5, NOTE_C5, 
  NOTE_C5, NOTE_D5, NOTE_Eb5, NOTE_D5, NOTE_C5, NOTE_Bb4, NOTE_Ab4, NOTE_G4, 
  NOTE_F4, NOTE_Eb4, NOTE_G4, NOTE_F4, NOTE_Eb4, NOTE_D4, NOTE_C4
};

// Accompaniment — melody shifted down an octave (octaves 3-4)
int accompaniment[] = {
  NOTE_C3, NOTE_D3, NOTE_Eb3, NOTE_Eb3, NOTE_F3, NOTE_G3, NOTE_G3, NOTE_Bb3, 
  NOTE_F3, NOTE_Eb3, NOTE_D3, NOTE_C3, NOTE_C3, NOTE_D3, NOTE_Eb3, NOTE_Eb3, 
  NOTE_F3, NOTE_G3, NOTE_G3, NOTE_Bb3, NOTE_C4, NOTE_Bb3, NOTE_D4, NOTE_C4, 
  NOTE_C4, NOTE_D4, NOTE_Eb4, NOTE_D4, NOTE_C4, NOTE_Bb3, NOTE_Ab3, NOTE_G3, 
  NOTE_F3, NOTE_Eb3, NOTE_G3, NOTE_F3, NOTE_Eb3, NOTE_D3, NOTE_C3
};

// Note durations: 4 = quarter, 8 = eighth, 2 = half
int noteDurations[] = {
  8,8,4,8,8,4,8,8,
  4,8,8,4,8,8,4,8,
  8,4,8,8,4,8,8,4,
  8,8,4,4,4,4,4,4,
  4,8,8,4,8,8,2
};

const int buzzerPin  = 6;
const int buzzerPin2 = 3;
const int buttonPin  = 9;
const int pixelPin   = 12;
const int switchPin  = 10;

const int BPM        = 80;
const int WHOLE_NOTE = (60000 * 4) / BPM;
const int melodyLen  = sizeof(melody) / sizeof(melody[0]);

const int NUMPIXELS = 1;
Adafruit_NeoPixel pixels(NUMPIXELS, pixelPin, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buzzerPin2, OUTPUT);
  pinMode(switchPin, INPUT_PULLUP);

  pixels.begin();
  pixels.setBrightness(50);
  pixels.clear();
  pixels.show();
}

void playSong() {
  // Arduino's tone() function can only be used one pin at a time, so we rapidly alternate
  // between the two buzzers. Each plays its own note fast enough that both
  // seem to sound somewhat simultaneously. SWITCH_MS controls how long each buzzer plays
  // per turn — higher sounds more distinct, lower sounds more blended but grainier.
  // In my testing, I found frequencies of 105-150 to sound the best as anything below 100 gains
  // a lot of clicking noises. Values that are too high (above say 175) result in shorter notes being
  // skipped altogether whereas the 105-150 range sounds almost like an old videogame theme (Which is kind of is)

  // handles length of buzzer play
  const int SWITCH_MS = 112;

  // cycle through the melody array and play the notes
  for (int i = 0; i < melodyLen; i++) {
    int noteDuration      = WHOLE_NOTE / noteDurations[i];
    int pauseBetweenNotes = (int)(noteDuration * 1.2);

    unsigned long startTime = millis();

    while ((long)(millis() - startTime) < pauseBetweenNotes) {
      unsigned long elapsed = millis() - startTime;

      if ((elapsed / SWITCH_MS) % 2 == 0) {
        tone(buzzerPin, melody[i]);
        noTone(buzzerPin2);
      } else {
        noTone(buzzerPin);
        tone(buzzerPin2, accompaniment[i]);
      }
    }

    noTone(buzzerPin);
    noTone(buzzerPin2);
  }
}

void loop() {
  // Slide switch controls NeoPixel light
  int switchVal = digitalRead(switchPin);
  if (switchVal == LOW) {
    pixels.setPixelColor(0, pixels.Color(150, 150, 150));
  } else {
    pixels.clear();
  }
  pixels.show();

  delay(20);

  // Button triggers song playback
  int buttonVal = digitalRead(buttonPin);
  if (buttonVal == LOW) {
    playSong();
  }

  delay(100);
}