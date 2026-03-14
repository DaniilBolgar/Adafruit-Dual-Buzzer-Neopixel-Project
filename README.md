This project is to program an Adafruit Flora to light up an RGB Neopixel when a switch is activated
as well as the added feature of playing the Skyrim theme song on two buzzers. This was achieved
by rapidly alternating between which buzzer is playing its tone due to the limited nature of tone().

Users can mess around with the length of the notes and how fast the buzzers play and alternate to
see how different settings affect the pseudo-harmony of the dual buzzer setup.

Notes can also be altered to whatever the user desires, however will require not only editing/adding note
frequencies, but also the note lengths array ( noteDurations[] ). This is not really difficult, the tone()
is pretty limited in its capabilities, but requires some testing to figure out the melody and rhythm.
