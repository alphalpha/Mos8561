#define CATCH_CONFIG_MAIN

#include <Mos8561.h>

#include <catch2/catch.hpp>
#include <functional>
#include <utility>
#include <vector>


struct Callback {
  void operator()() {
    ++numCalls;
  }
  size_t numCalls = 0;
};

struct RegisterCallback {
  void operator()(uint8_t address, uint8_t data) {
    vec.push_back(std::make_pair(address, data));
  }
  std::vector<std::pair<uint8_t, uint8_t>> vec;
};

struct MockController {
  MockController(Callback& clockFn, Callback& resetFn, RegisterCallback& registerFn)
    : startClockCallback(clockFn)
    , resetCallback(resetFn)
    , writeRegisterCallback(registerFn) {
  }

  void startClock() {
    startClockCallback();
  }

  void reset() {
    resetCallback();
  }

  void writeRegister(uint8_t address, uint8_t data) {
    writeRegisterCallback(address, data);
  }

  Callback& startClockCallback;
  Callback& resetCallback;
  RegisterCallback& writeRegisterCallback;
};

TEST_CASE("Mos8561") {
  Callback startClockCallback;
  Callback resetCallback;
  RegisterCallback writeRegisterCallback;
  MockController ctl(startClockCallback, resetCallback, writeRegisterCallback);
  sid::Mos8561<MockController> mos(ctl);

  REQUIRE(startClockCallback.numCalls == 0);
  REQUIRE(resetCallback.numCalls == 0);
  REQUIRE(writeRegisterCallback.vec.size() == 0);

  SECTION("Start") {
    mos.start();
    REQUIRE(startClockCallback.numCalls == 1);
    REQUIRE(resetCallback.numCalls == 1);
    REQUIRE(writeRegisterCallback.vec.size() == 0);
  }

  SECTION("Set Volume") {
    mos.setVolume(127);
    CHECK(startClockCallback.numCalls == 0);
    CHECK(resetCallback.numCalls == 0);
    REQUIRE(writeRegisterCallback.vec.size() == 1);
    REQUIRE(writeRegisterCallback.vec.front().first == 24);
    REQUIRE(writeRegisterCallback.vec.front().second == 0b00001111);
  }

  SECTION("Set Square Waveform for first Voice") {
    mos.setWaveform(0, sid::Waveform::Square);
    CHECK(startClockCallback.numCalls == 0);
    CHECK(resetCallback.numCalls == 0);
    REQUIRE(writeRegisterCallback.vec.size() == 1);
    REQUIRE(writeRegisterCallback.vec.front().first == 4);
    REQUIRE(writeRegisterCallback.vec.front().second == 0b01000000);
  }

  SECTION("Set Noise Waveform for second Voice") {
    mos.setWaveform(1, sid::Waveform::Noise);
    CHECK(startClockCallback.numCalls == 0);
    CHECK(resetCallback.numCalls == 0);
    REQUIRE(writeRegisterCallback.vec.size() == 1);
    REQUIRE(writeRegisterCallback.vec.front().first == 11);
    REQUIRE(writeRegisterCallback.vec.front().second == 0b10000000);
  }

  SECTION("Set Triangle Waveform for third Voice") {
    mos.setWaveform(2, sid::Waveform::Triangle);
    CHECK(startClockCallback.numCalls == 0);
    CHECK(resetCallback.numCalls == 0);
    REQUIRE(writeRegisterCallback.vec.size() == 1);
    REQUIRE(writeRegisterCallback.vec.front().first == 18);
    REQUIRE(writeRegisterCallback.vec.front().second == 0b00010000);
  }

  SECTION("Set ADSR for first Voice") {
    sid::Adsr adsr = {0x8, 0x3, 0xF, 0x4};
    mos.setAdsr(0, adsr);
    CHECK(startClockCallback.numCalls == 0);
    CHECK(resetCallback.numCalls == 0);
    REQUIRE(writeRegisterCallback.vec.size() == 2);
    REQUIRE(writeRegisterCallback.vec.front().first == 5);
    REQUIRE(writeRegisterCallback.vec.front().second == 0b10000011);
    REQUIRE(writeRegisterCallback.vec.back().first == 6);
    REQUIRE(writeRegisterCallback.vec.back().second == 0b11110100);
  }

  SECTION("Set ADSR for second Voice") {
    sid::Adsr adsr = {0xA, 0x4, 0x0, 0x5};
    mos.setAdsr(1, adsr);
    CHECK(startClockCallback.numCalls == 0);
    CHECK(resetCallback.numCalls == 0);
    REQUIRE(writeRegisterCallback.vec.size() == 2);
    REQUIRE(writeRegisterCallback.vec.front().first == 12);
    REQUIRE(writeRegisterCallback.vec.front().second == 0b10100100);
    REQUIRE(writeRegisterCallback.vec.back().first == 13);
    REQUIRE(writeRegisterCallback.vec.back().second == 0b00000101);
  }

  SECTION("Set ADSR for third Voice") {
    sid::Adsr adsr = {0x2, 0x0, 0x1, 0xB};
    mos.setAdsr(2, adsr);
    CHECK(startClockCallback.numCalls == 0);
    CHECK(resetCallback.numCalls == 0);
    REQUIRE(writeRegisterCallback.vec.size() == 2);
    REQUIRE(writeRegisterCallback.vec.front().first == 19);
    REQUIRE(writeRegisterCallback.vec.front().second == 0b00100000);
    REQUIRE(writeRegisterCallback.vec.back().first == 20);
    REQUIRE(writeRegisterCallback.vec.back().second == 0b00011011);
  }

  SECTION("Set Pulse Width for first Voice (lower byte only)") {
    const auto width = 42;
    mos.setPulseWidth(0, width);
    CHECK(startClockCallback.numCalls == 0);
    CHECK(resetCallback.numCalls == 0);
    CHECK(writeRegisterCallback.vec.size() == 2);
    REQUIRE(writeRegisterCallback.vec.front().first == 2);
    REQUIRE(writeRegisterCallback.vec.front().second == 42);
    REQUIRE(writeRegisterCallback.vec.back().first == 3);
    REQUIRE(writeRegisterCallback.vec.back().second == 0);
  }

  SECTION("Set Pulse Width for first Voice (upper four bits only)") {
    const auto width = 256;
    mos.setPulseWidth(0, width);
    CHECK(startClockCallback.numCalls == 0);
    CHECK(resetCallback.numCalls == 0);
    CHECK(writeRegisterCallback.vec.size() == 2);
    REQUIRE(writeRegisterCallback.vec.front().first == 2);
    REQUIRE(writeRegisterCallback.vec.front().second == 0);
    REQUIRE(writeRegisterCallback.vec.back().first == 3);
    REQUIRE(writeRegisterCallback.vec.back().second == 1);
  }

  SECTION("Set Pulse Width for second Voice") {
    const auto width = 1234;
    mos.setPulseWidth(1, width);
    CHECK(startClockCallback.numCalls == 0);
    CHECK(resetCallback.numCalls == 0);
    CHECK(writeRegisterCallback.vec.size() == 2);
    REQUIRE(writeRegisterCallback.vec.front().first == 9);
    REQUIRE(writeRegisterCallback.vec.front().second == 0b11010010);
    REQUIRE(writeRegisterCallback.vec.back().first == 10);
    REQUIRE(writeRegisterCallback.vec.back().second == 0b100);
  }

  SECTION("Set Pulse Width for third Voice") {
    const auto width = 3456;
    mos.setPulseWidth(2, width);
    CHECK(startClockCallback.numCalls == 0);
    CHECK(resetCallback.numCalls == 0);
    CHECK(writeRegisterCallback.vec.size() == 2);
    REQUIRE(writeRegisterCallback.vec.front().first == 16);
    REQUIRE(writeRegisterCallback.vec.front().second == 0b10000000);
    REQUIRE(writeRegisterCallback.vec.back().first == 17);
    REQUIRE(writeRegisterCallback.vec.back().second == 0b1101);
  }

  SECTION("Play Note On and Off with first Voice") {
    const auto voiceNum = 0;
    const auto note = 57;
    mos.setWaveform(voiceNum, sid::Waveform::Square);
    mos.playNote(voiceNum, note, 127);
    CHECK(writeRegisterCallback.vec.size() == 4);
    REQUIRE(writeRegisterCallback.vec.at(0).first == 4);
    REQUIRE(writeRegisterCallback.vec.at(0).second == 0b01000000);
    REQUIRE(writeRegisterCallback.vec.at(1).first == 0);
    REQUIRE(writeRegisterCallback.vec.at(1).second == 0x6B);
    REQUIRE(writeRegisterCallback.vec.at(2).first == 1);
    REQUIRE(writeRegisterCallback.vec.at(2).second == 0x0E);
    REQUIRE(writeRegisterCallback.vec.at(3).first == 4);
    REQUIRE(writeRegisterCallback.vec.at(3).second == 0b01000001);

    mos.playNote(voiceNum, note, 0);
    REQUIRE(writeRegisterCallback.vec.at(4).first == 0);
    REQUIRE(writeRegisterCallback.vec.at(4).second == 0x6B);
    REQUIRE(writeRegisterCallback.vec.at(5).first == 1);
    REQUIRE(writeRegisterCallback.vec.at(5).second == 0x0E);
    REQUIRE(writeRegisterCallback.vec.at(6).first == 4);
    REQUIRE(writeRegisterCallback.vec.at(6).second == 0b01000000);
  }

  SECTION("Play Note On and Off with second Voice") {
    const auto voiceNum = 1;
    const auto note = 48;
    mos.setWaveform(voiceNum, sid::Waveform::Triangle);
    mos.playNote(voiceNum, note, 127);
    CHECK(writeRegisterCallback.vec.size() == 4);
    REQUIRE(writeRegisterCallback.vec.at(0).first == 11);
    REQUIRE(writeRegisterCallback.vec.at(0).second == 0b00010000);
    REQUIRE(writeRegisterCallback.vec.at(1).first == 7);
    REQUIRE(writeRegisterCallback.vec.at(1).second == 0x93);
    REQUIRE(writeRegisterCallback.vec.at(2).first == 8);
    REQUIRE(writeRegisterCallback.vec.at(2).second == 0x08);
    REQUIRE(writeRegisterCallback.vec.at(3).first == 11);
    REQUIRE(writeRegisterCallback.vec.at(3).second == 0b00010001);

    mos.playNote(voiceNum, note, 0);
    REQUIRE(writeRegisterCallback.vec.at(4).first == 7);
    REQUIRE(writeRegisterCallback.vec.at(4).second == 0x93);
    REQUIRE(writeRegisterCallback.vec.at(5).first == 8);
    REQUIRE(writeRegisterCallback.vec.at(5).second == 0x08);
    REQUIRE(writeRegisterCallback.vec.at(6).first == 11);
    REQUIRE(writeRegisterCallback.vec.at(6).second == 0b00010000);
  }

  SECTION("Play Note On and Off with third Voice") {
    const auto voiceNum = 2;
    const auto note = 14;
    mos.setWaveform(voiceNum, sid::Waveform::Saw);
    mos.playNote(voiceNum, note, 127);
    CHECK(writeRegisterCallback.vec.size() == 4);
    REQUIRE(writeRegisterCallback.vec.at(0).first == 18);
    REQUIRE(writeRegisterCallback.vec.at(0).second == 0b00100000);
    REQUIRE(writeRegisterCallback.vec.at(1).first == 14);
    REQUIRE(writeRegisterCallback.vec.at(1).second == 0x34);
    REQUIRE(writeRegisterCallback.vec.at(2).first == 15);
    REQUIRE(writeRegisterCallback.vec.at(2).second == 0x01);
    REQUIRE(writeRegisterCallback.vec.at(3).first == 18);
    REQUIRE(writeRegisterCallback.vec.at(3).second == 0b00100001);

    mos.playNote(voiceNum, note, 0);
    REQUIRE(writeRegisterCallback.vec.at(4).first == 14);
    REQUIRE(writeRegisterCallback.vec.at(4).second == 0x34);
    REQUIRE(writeRegisterCallback.vec.at(5).first == 15);
    REQUIRE(writeRegisterCallback.vec.at(5).second == 0x01);
    REQUIRE(writeRegisterCallback.vec.at(6).first == 18);
    REQUIRE(writeRegisterCallback.vec.at(6).second == 0b00100000);
  }
}
