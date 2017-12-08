//
//  www.blinkenlight.net
//
//  Copyright 2016 Udo Klein
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see http://www.gnu.org/licenses/


// Resources:
//     http://www.obbl-net.de/WWVB.html
//     http://home.arcor.de/armin.diehl/WWVBusb/
//     http://www.meinbergglobal.com/english/info/ntp.htm#cfg
//     http://www.meinbergglobal.com/download/ntp/docs/ntp_cheat_sheet.pdf
//     http://www.meinberg.de/german/specs/timestr.htm
//     http://www.eecis.udel.edu/~mills/ntp/html/parsedata.html

#include <WWVB.h>

const uint8_t WWVB_analog_sample_pin = 5;
const uint8_t WWVB_sample_pin = A5;  // A5 == d19
const uint8_t WWVB_inverted_samples = 1;
const uint8_t WWVB_analog_samples = 1;
// const uint8_t WWVB_pin_mode = INPUT;  // disable internal pull up
const uint8_t WWVB_pin_mode = INPUT_PULLUP;  // enable internal pull up
const uint8_t WWVB_signal_good_indicator_pin = 13;

const uint8_t WWVB_monitor_pin = A4;  // A4 == d18

#if defined(__AVR__)
    #define print(...)   Serial.print(__VA_ARGS__)
    #define println(...) Serial.println(__VA_ARGS__)
#else
    #define print(...)   SerialUSB.print(__VA_ARGS__)
    #define println(...) SerialUSB.println(__VA_ARGS__)
#endif

uint8_t sample_input_pin() {
    const uint8_t sampled_data =
        WWVB_inverted_samples ^ (WWVB_analog_samples? (analogRead(WWVB_analog_sample_pin) > 200)
                                                      : digitalRead(WWVB_sample_pin));

    digitalWrite(WWVB_monitor_pin, sampled_data);
    return sampled_data;
}

void setup() {
    #if defined(__AVR__)
    Serial.begin(9600, SERIAL_7E2);
    #else
    SerialUSB.begin(9600, SERIAL_7E2);
    #endif
    println();
    println(F("Meinberg Emulator V3.1.1"));
    println(F("(c) Udo Klein 2016"));
    println(F("www.blinkenlight.net"));
    println();
    print(F("Sample Pin:      ")); println(WWVB_sample_pin);
    print(F("Sample Pin Mode: ")); println(WWVB_pin_mode);
    print(F("Inverted Mode:   ")); println(WWVB_inverted_samples);
    print(F("Analog Mode:     ")); println(WWVB_analog_samples);
    print(F("Monitor Pin:     ")); println(WWVB_monitor_pin);
    print(F("Signal Good Indicator Pin: ")); println(WWVB_signal_good_indicator_pin);
    println();
    println();
    println(F("Initializing..."));
    println();

    pinMode(WWVB_monitor_pin, OUTPUT);
    pinMode(WWVB_sample_pin, WWVB_pin_mode);
    pinMode(WWVB_signal_good_indicator_pin, OUTPUT);
    digitalWrite(WWVB_signal_good_indicator_pin, LOW);

    DCF77_Clock::setup();
    DCF77_Clock::set_input_provider(sample_input_pin);
}

void paddedPrint(BCD::bcd_t n) {
    print(n.digit.hi);
    print(n.digit.lo);
}

void loop() {
    const char STX = 2;
    const char ETX = 3;

    Clock::time_t now;

    DCF77_Clock::get_current_time(now);
    if (now.month.val > 0) {

        //println();
        print(STX);

        print("D:");
        paddedPrint(now.day);
        print('.');
        paddedPrint(now.month);
        print('.');
        paddedPrint(now.year);
        print(';');

        print("T:");
        print(now.weekday.digit.lo);
        print(';');

        print("U:");
        paddedPrint(now.hour);
        print('.');
        paddedPrint(now.minute);
        print('.');
        paddedPrint(now.second);
        print(';');

        uint8_t state = DCF77_Clock::get_clock_state();
        print(
            state == Clock::useless || state == Clock::dirty? '#'  // not synced
                                                            : ' '  // good
        );

        print(
            state == Clock::synced || state == Clock::locked? ' '  // DCF77
                                                            : '*'  // crystal clock
        );

        digitalWrite(WWVB_signal_good_indicator_pin, state >= Clock::locked);

        print(now.uses_summertime? 'S': ' ');
        print(
            now.timezone_change_scheduled? '!':
            now.leap_second_scheduled?     'A':
                                           ' '
        );

        print(ETX);
    }
}
