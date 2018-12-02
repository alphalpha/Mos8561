/*
  Mos8561 - Library for interfacing with the Mos8561 Chip (SID - "Sound Interface Device")
  Created by Christian Dietz
  Released into the public domain
*/

#ifndef Morse_h
#define Morse_h

#include "Arduino.h"
#include <assert.h>

namespace sid {

const byte LATCH_PIN = 3;
const byte CLOCK_PIN = 4;
const byte DATA_PIN = 2;

const byte SID_RESET_PIN = 8;
const byte SID_CLOCK_PIN = 9;
const byte SID_SELECT_PIN = 13;

const int REF_A = 440;

enum class Waveform : byte {
  Noise = 0x80,
  Square = 0x40,
  Saw = 0x20,
  Triangle = 0x10
};

struct Adsr {
  byte att;
  byte dec;
  byte sus;
  byte rel;
};

namespace {
  int noteToFreq(byte note) {
    return REF_A * pow(2, (note - 69) / 12.f);
  }

  word freqAsWord(int freq) {
    return freq / 0.0596;
  }

  byte byteTo4Bits(int val) {
    byte rtv = map(val, 1, 127, 0, 15);
    return rtv;
  }

  void printBinary(int inByte) {
    for (int b = 7; b >= 0; b--)
    {
      Serial.print(bitRead(inByte, b));
    }
  }

  void printAddressData(byte address, byte data) {
    printBinary(address);
    Serial.print(F(" "));
    printBinary(data);
    Serial.println();
  }
}; // unnamed namespace

class Mos8561
{
public:
  Mos8561() {
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(SID_RESET_PIN, OUTPUT);
    pinMode(SID_SELECT_PIN, OUTPUT);
    pinMode(SID_CLOCK_PIN, OUTPUT );

    startClock();
    reset();
  }

  void setVolume(byte vol) {
    volume = vol;
    byte data = byteTo4Bits(volume);
    byte address = 0x18;
    writeRegister(address, data);
  }

  void setAdsr(byte voiceNum, Adsr adsr) {
    voices[voiceNum].adsr = adsr;
    // 1. Attack/Release
    byte address = 5 + (voiceNum * 7);
    byte data = voices[voiceNum].adsr.dec + (voices[voiceNum].adsr.att << 4);
    writeRegister(address, data);
    // 2. Release/Sustain
    ++address;
    data = voices[voiceNum].adsr.rel + (voices[voiceNum].adsr.sus << 4);
    writeRegister(address, data);
  }

  void setWaveform(byte voiceNum, Waveform waveform) {
    voices[voiceNum].waveform = waveform;
    writeControlByte(voiceNum);
  }

  void playNote(byte voiceNum, byte note, byte velocity) {
    assert(voiceNum < 3);
    voices[voiceNum].isPlaying = velocity > 0;
    writeNote(voiceNum, note);
    writeControlByte(voiceNum);
  }

private:
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

  void waitCycles(byte numCycles) {
    delayMicroseconds(numCycles);
  }

  void writeNote(byte voiceNum, byte note) {
    int hz = noteToFreq(note);
    word freq = freqAsWord(hz);
    // 1. Freq Lo
    byte address = voiceNum * 7;
    byte data = lowByte(freq);
    writeRegister(address, data);
    // 1. Freq Lo
    address += 1;
    data = highByte(freq);
    writeRegister(address, data);
  }

  void writeControlByte(byte voiceNum) {
    byte data = byte(voices[voiceNum].waveform) + voices[voiceNum].isPlaying;
    byte address = 4 + (voiceNum * 7);
    writeRegister(address, data);
  }

  void writeRegister(byte address, byte data) {
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

  struct Voice {
    byte num;
    Waveform waveform;
    Adsr adsr;
    bool isPlaying;
  };
  
  Voice voices[3];
  byte volume;
};

} // namespace sid

#endif

