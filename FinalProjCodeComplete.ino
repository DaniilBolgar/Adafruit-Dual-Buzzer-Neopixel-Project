/*
    Skyrim main theme on dual buzzer player

    References and acknowledgements:
    Original inspiration for solution method to dual passive buzzer problem:
    https://www.reddit.com/r/arduino/comments/18dsiaz/how_do_i_reproduce_twoor_more_tones_at_the_same/

    millis() function documentation
    https://docs.arduino.cc/language-reference/en/functions/time/millis/

    tone() function documentation
    https://docs.arduino.cc/language-reference/en/functions/advanced-io/tone/

    Using millis() to time events such as playing tone()
    https://www.programmingelectronics.com/arduino-sketch-with-millis-instead-of-delay/

    The rights to the melody used in this program belongs to Bethesda's Elder Scrolls V: Skyrim and its
    original creator Jeremy Soule
    https://youtu.be/813-3iL5OsE

    Hardware:
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
    - Slide switch ON: NeoPixel should light up Red
    - Slide switch OFF: NeoPixel should turn off
    - Modifying SWITCH_MS changes length of buzzer play. Higher = slower alternation, Lower = faster
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

// Note durations: 2 = half, 4 = quarter, 8 = eighth
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

//Beats Per Minute - how many quarter notes played in 60 seconds:
const int BPM        = 80;
// 4 quarter notes per whole note divided by how many quarter notes play per minute
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
  /* 
    Because of the buzzers being used are passive, they require external input to tell them what tone to play (hence tone()), and can only be activated one at a time.
    The tradeoff is that they allow us to have only 2 buzzers as opposed to many more by each being able to play a range of notes.
    Active buzzers, on the other hand, would have a fixed frequency they play at, and be able to play at the same time as other buzzers.
    Since the program utilizes the passive version, we need to employ a rapid alternating technique. We can do so with millis()
    which, unlike delay, does not block/freeze the microcontroller so the other buzzer can play while that buzzer falls quiet.
    The intended result is that the alternation between buzzers is fast enough that both buzzers' melodies will be distinguishable. 

    SWITCH_MS controls how long a buzzer has to play its tone. I have found that the range of 105-160ms to be the best balance for audio quality
    and harmonization. 
    Lower = faster switching but grainer and lower quality
    Higher = slower switching, but runs the risk of skipping notes by blocking the other buzzer for too long 
  */

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
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
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