#include "Mos8561.h"


sid::Mos8561 synth;

void setup() {
  Serial.begin(38400);
  synth = sid::Mos8561();
  sid::Adsr adsr = {0x4, 0x3, 0xF, 0x4};
  synth.setVolume(127);
  for (int i = 0; i < 3; ++i) {
    synth.setAdsr(i, adsr);
    synth.setWaveform(i, sid::Waveform::Triangle);
  }
}

// Test Sequencer
byte note_seq[] = {45, 57, 69};
long interval = 250;
long prev = 0;
byte vel = 127;
byte index = 0;

void loop() {
  auto current = millis();
  if (current - prev > interval) {
    prev = current;
    if (vel > 0) {
      index = index == 2 ? 0 : index + 1;
    }
    synth.playNote(0, note_seq[index], vel);
    synth.playNote(1, note_seq[index] + 3, vel);
//    synth.playNote(2, note_seq[index], vel);
    vel = vel > 0 ? 0 : 127;
  }
}
