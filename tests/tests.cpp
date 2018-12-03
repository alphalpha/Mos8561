#define CATCH_CONFIG_MAIN

#include <functional>
#include <catch2/catch.hpp>
#include <Mos8561.h>


struct Callback {
  void operator()() {
    ++numCalls;
  }
  size_t numCalls = 0;
};

struct MockController {
  MockController(Callback& clockFn, Callback& resetFn, Callback& registerFn)
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

  void writeRegister(uint8_t, uint8_t) {
    writeRegisterCallback();
  }

  Callback& startClockCallback;
  Callback& resetCallback;
  Callback& writeRegisterCallback;
};

TEST_CASE("Mos8561") {
  Callback startClockCallback;
  Callback resetCallback;
  Callback writeRegisterCallback;
  MockController ctl(startClockCallback, resetCallback, writeRegisterCallback);
  sid::Mos8561<MockController> mos(ctl);

  REQUIRE(startClockCallback.numCalls == 0 );
  REQUIRE(resetCallback.numCalls == 0 );
  REQUIRE(writeRegisterCallback.numCalls == 0 );

  SECTION("Start") {
    mos.start();
    REQUIRE(startClockCallback.numCalls == 1 );
    REQUIRE(resetCallback.numCalls == 1 );
    REQUIRE(writeRegisterCallback.numCalls == 0 );
  }
}
