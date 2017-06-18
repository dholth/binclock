/*
 * STM32 binary clock.
 *
 * Daniel Holth <dholth@fastmail.fm>, 2017
 */

#include "config/stm32plus.h"
#include "config/exti.h"
#include "config/power.h"
#include "config/stream.h"
#include "config/usart.h"

extern "C" {
#include "microscript.h"
}

using namespace stm32plus;

Rtc<RtcLseClockFeature> *_rtc;

class UsartRXInterrupts {

protected:
  /*
   * The USART1 peripheral, plain.
   */
  Usart1_Remap1<> _usart;

public:
  UsartPollingOutputStream outputStream;

  /*
   * Use the constructor base initializer to set up the USART at 57600
   */

  UsartRXInterrupts() : _usart(57600), outputStream(_usart) {}

  /*
   * Run the test
   */

  void run() {
    this->init();
    for (;;) {
      this->tick();
    }
  }

  void init() {
    init_microscript();

    // we're using interrupts, set up the NVIC
    Nvic::initialise();

    outputStream << "Hello from the clock\n";
    outputStream << "RTC "
                 << (_rtc->survived() ? "survived\n" : "did not survive\n");
    var_display("time");
  }

  void tick() {
    if (_usart.dataAvailable()) {
      char c[] = {_usart.receive()};
      _usart.send(c[0]); // echo
      parse_microscript(c, 1, c[0] == '\n');
      if (c[0] == '\n') {
        init_microscript(); // start over in case of error
      }
    }
  }
};

UsartRXInterrupts *usart;

void var_display(const char *name) {
  std::string time("time"), h("h"), m("m"), s("s");
  if (time.compare(name) == 0) {
    auto out = usart->outputStream;
    uint8_t time[4];

    _rtc->getTime(time[0], time[1], time[2], time[3]);
    out << name << "=";

    char buffer[4];
    for (int i = 0; i < 3; i++) {
      itoa(time[i], buffer, 10);
      if (time[i] < 10) {
        out << "0";
      }
      out << buffer;
    }
    out << "\n";
  }
}

void time_set(const char *variable, uint16_t time[]) {
  if (std::string("time").compare(variable) == 0) {
    _rtc->setTime(time[0], time[1], time[2]);
    usart->outputStream << "Time set\n";
  }
}

void var_set(const char *name, uint16_t value) {
  std::string h("h"), m("m"), s("s");
  bool set_time = false;
  uint8_t hours, minutes, seconds, am_pm;
  _rtc->getTime(hours, minutes, seconds, am_pm);
  if (h.compare(name) == 0) {
    hours = value;
    set_time = true;
  } else if (m.compare(name) == 0) {
    minutes = value;
    set_time = true;
  } else if (s.compare(name) == 0) {
    seconds = value;
    set_time = true;
  }
  if (set_time) {
    _rtc->setTime(hours, minutes, seconds, am_pm);
  }
}

/*
 * Main entry point
 */

int main() {

  MillisecondTimer::initialise();

  // Use low drive level for low capacitance LSE
  RCC_LSEDriveConfig(RCC_LSEDrive_Low);

  Rtc<RtcLseClockFeature> rtc(RTC_HourFormat_24, 0xabc123ab);

  _rtc = &rtc;

  UsartRXInterrupts commands;
  usart = &commands;

  usart->init();

  // drive individual LEDs
  GpioB<DigitalOutputFeature<GPIO_Speed_Level_3, Gpio::OPEN_DRAIN,
                             Gpio::PUPD_NONE, 10, 11, 12, 13, 14, 15>>
      sinks;

  // drive FETs to sink BJT to source current to LED groups
  // pin 9 has a hardware problem (probably with external FET) on my board
  GpioA<DigitalOutputFeature<GPIO_Speed_Level_3, Gpio::PUSH_PULL,
                             Gpio::PUPD_NONE, 8, 9, 10, 11>>
      sources;

  bool testpattern = false;

  while (true) {
    usart->tick();

    uint8_t hours, minutes, seconds, am_pm;
    _rtc->getTime(hours, minutes, seconds, am_pm);

    if (!testpattern || seconds % 3 == 0) {
      sources.resetAll();
      sinks.setAll();
      *sinks.getResetRegister() = hours << 10;
      sources[8].set();
      MillisecondTimer::delay(2);
    }

    if (!testpattern || seconds % 3 == 1) {
      sources.resetAll();
      sinks.setAll();
      *sinks.getResetRegister() = minutes << 10;
      sources[10].set();
      MillisecondTimer::delay(2);
    }

    if (!testpattern || seconds % 3 == 2) {
      sources.resetAll();
      sinks.setAll();
      *sinks.getResetRegister() = seconds << 10;
      sources[11].set();
      MillisecondTimer::delay(2);
    }
  }

  // not reached
  return 0;
}
