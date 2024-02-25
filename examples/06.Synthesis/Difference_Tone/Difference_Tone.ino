/*  Example using clipping to modify the spectrum of an audio signal
    and emphasise a tone generated by the difference in frequency of 2 waves,
    using Mozzi sonification library.

    Demonstrates the use of EventDelay(), rand() and fixed-point numbers.
    
    Note that an example using the newer FixMath paradigm is also available: Difference_Tone_FixMath.

    Circuit: Audio output on digital pin 9 on a Uno or similar, or
    DAC/A14 on Teensy 3.1, or
    check the README or http://sensorium.github.io/Mozzi/

		Mozzi documentation/API
		https://sensorium.github.io/Mozzi/doc/html/index.html

		Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users

    Tim Barrass 2012, CC by-nc-sa.
*/

#include <Mozzi.h>
#include <Oscil.h>
#include <EventDelay.h>
#include <mozzi_rand.h>
#include <mozzi_midi.h>
#include <tables/sin2048_int8.h>

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
Oscil <SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> aSin1(SIN2048_DATA); // sine wave sound source
Oscil <SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> aSin2(SIN2048_DATA); // sine wave sound source
Oscil <SIN2048_NUM_CELLS, MOZZI_AUDIO_RATE> aGain(SIN2048_DATA); // to fade audio signal in and out before waveshaping

// for scheduling note changes
EventDelay kChangeNoteDelay;

// audio frequency as Q16n16 fractional number
Q16n16 freq1 = Q8n0_to_Q16n16(440);


void setup(){
  startMozzi(); // :)
  aSin1.setFreq_Q16n16(freq1); // set the frequency with a Q16n16 fractional number
  aGain.setFreq(0.2f); // use a float for low frequencies, in setup it doesn't need to be fast
  kChangeNoteDelay.set(2000); // note duration ms, within resolution of MOZZI_CONTROL_RATE
}


void updateControl(){
  if(kChangeNoteDelay.ready()){
    // change proportional frequency of second tone
    byte harmonic = rand((byte)12);
    byte shimmer = rand((byte)255);
    Q16n16 harmonic_step = freq1/12;
    Q16n16 freq2difference = harmonic*harmonic_step;
    freq2difference += (harmonic_step*shimmer)>>11;
    Q16n16 freq2 = freq1-freq2difference;
    aSin2.setFreq_Q16n16(freq2); // set the frequency with a Q16n16 fractional number
    kChangeNoteDelay.start();
  }
}


AudioOutput updateAudio(){
  int asig = (int)((((uint32_t)aSin1.next()+ aSin2.next())*(200u+aGain.next()))>>3);
  return MonoOutput::fromAlmostNBit(9, asig).clip();
}


void loop(){
  audioHook(); // required here
}
