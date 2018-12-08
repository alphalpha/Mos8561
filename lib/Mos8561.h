/*
  Mos8561 - Library for interfacing with the Mos8561 Chip (SID - "Sound Interface Device")
  Created by Christian Dietz
  Released into the public domain
*/

#ifndef Mos8561_h
#define Mos8561_h

#include <assert.h>
#include <math.h>


namespace sid {

const int REF_A = 440;

enum class Waveform : uint8_t {
  Noise = 0x80,
  Square = 0x40,
  Saw = 0x20,
  Triangle = 0x10
};

struct Adsr {
  uint8_t att;
  uint8_t dec;
  uint8_t sus;
  uint8_t rel;
};

// This type must be implemented and injected into Mos8561
// It defines how the Mos8561 chip is connected to the Arduino
struct Controller {
  Controller();
  void startClock(); 
  void reset();
  void writeRegister(const uint8_t address, const uint8_t data);
};

namespace {
  float noteToFreq(const float note) {
    return REF_A * pow(2, (note - 69) / 12.f);
  }

  uint16_t freqAsWord(const float freq) {
    return (uint16_t)round(freq / 0.0596);
  }

  uint8_t byteTo4Bits(const int val) {
    return (uint8_t)(val * 15 / 127.f);
  }
}; // unnamed namespace

template<typename Controller>
class Mos8561
{
public:
  Mos8561(Controller ctr):
    controller(ctr) {
  }

  void start() {
    controller.startClock();
    controller.reset();
  }

  void setVolume(const uint8_t vol) {
    volume = vol;
    uint8_t data = byteTo4Bits(volume);
    uint8_t address = 0x18;
    controller.writeRegister(address, data);
  }

  void setAdsr(const uint8_t voiceNum, const Adsr adsr) {
    voices[voiceNum].adsr = adsr;
    // 1. Attack/Release
    uint8_t address = 5 + (voiceNum * 7);
    uint8_t data = voices[voiceNum].adsr.dec + (voices[voiceNum].adsr.att << 4);
    controller.writeRegister(address, data);
    // 2. Release/Sustain
    ++address;
    data = voices[voiceNum].adsr.rel + (voices[voiceNum].adsr.sus << 4);
    controller.writeRegister(address, data);
  }

  void setWaveform(const uint8_t voiceNum, const Waveform waveform) {
    voices[voiceNum].waveform = waveform;
    writeControlByte(voiceNum);
  }

  void setPulseWidth(const uint8_t voiceNum, const uint16_t width) {
    assert(width < 4096);
    voices[voiceNum].pulseWidth = width;
    // Lower byte of 12-bit value
    uint8_t data = (uint8_t)(voices[voiceNum].pulseWidth & 0xff);
    uint8_t address = 2 + (voiceNum * 7);
    controller.writeRegister(address, data);
    // Upper four bits of 12-bit value
    data = (uint8_t)(voices[voiceNum].pulseWidth >> 8);
    ++address;
    controller.writeRegister(address, data);
  }

  void setFilterIsEnabled(const uint8_t voiceNum, const bool isEnabled) {
    voices[voiceNum].filterIsEnabled = isEnabled;
    const uint8_t address = 23;
    const uint8_t data = voices[0].filterIsEnabled + (voices[1].filterIsEnabled << 1)
      + (voices[2].filterIsEnabled << 2);
    controller.writeRegister(address, data);
  }

  void playNote(const uint8_t voiceNum, const double note, const uint8_t velocity) {
    assert(voiceNum < 3);
    voices[voiceNum].isPlaying = velocity > 0;
    const auto hz = noteToFreq(note);
    writeFrequency(voiceNum, hz);
    writeControlByte(voiceNum);
  }

private:
  void writeFrequency(const uint8_t voiceNum, const float hz) {
    uint16_t freq = freqAsWord(hz);
    // 1. Freq Lo
    uint8_t address = voiceNum * 7;
    uint8_t data = (uint8_t)(freq & 0xff);
    controller.writeRegister(address, data);
    // 1. Freq Lo
    address += 1;
    data = (uint8_t)(freq >> 8);
    controller.writeRegister(address, data);
  }

  void writeControlByte(const uint8_t voiceNum) {
    uint8_t data = uint8_t(voices[voiceNum].waveform) + voices[voiceNum].isPlaying;
    uint8_t address = 4 + (voiceNum * 7);
    controller.writeRegister(address, data);
  }

  void writeRegister(const uint8_t address, const uint8_t data) {
    controller.writeRegister(address, data);
  }

  struct Voice {
    uint8_t num;
    Waveform waveform;
    uint16_t pulseWidth;
    Adsr adsr;
    bool filterIsEnabled = false;
    bool isPlaying = false;
  };
  
  Controller controller;
  Voice voices[3];
  uint8_t volume;
};

} // namespace sid

#endif

