#include "Mos8561.h"


void printBinary(int inByte) {
  for (int b = 7; b >= 0; b--)
  {
    Serial.print(bitRead(inByte, b));
  }
}

void printAddressData(uint8_t address, uint8_t data) {
  printBinary(address);
  Serial.print(F(" "));
  printBinary(data);
  Serial.println();
}

struct Controller {
  const byte LATCH_PIN = 3;
  const byte CLOCK_PIN = 4;
  const byte DATA_PIN = 2;

  const byte SID_RESET_PIN = 8;
  const byte SID_CLOCK_PIN = 9;
  const byte SID_SELECT_PIN = 13;

  Controller() {
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(SID_RESET_PIN, OUTPUT);
    pinMode(SID_SELECT_PIN, OUTPUT);
    pinMode(SID_CLOCK_PIN, OUTPUT );
  }

  void startClock() {
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 7;
    TCCR1A |= (1 << COM1A0);
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS10);
    waitCycles(1);
  }

  void reset() {
    digitalWrite(SID_RESET_PIN, HIGH);
    waitCycles(2);
    digitalWrite(SID_RESET_PIN, LOW);
    waitCycles(10);
    digitalWrite(SID_RESET_PIN, HIGH);
  }

  void writeRegister(const byte address, const byte data) {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, data);
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, address);
    digitalWrite(LATCH_PIN, HIGH);

    // What is the deal with the SELECT PIN?
    // Also works without toggling it
    digitalWrite(SID_SELECT_PIN, HIGH);
    waitCycles(2);
    digitalWrite(SID_SELECT_PIN, LOW);
  }
private:
  void waitCycles(uint8_t numCycles) {
    delayMicroseconds(numCycles);
  }
};

Controller controller;
sid::Mos8561<Controller> synth(controller);

void setup() {
  Serial.begin(38400);
  synth.start();
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
