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

#include <WWVB.h>

#if defined(__AVR__)
const uint8_t WWVB_analog_sample_pin = 5;
const uint8_t WWVB_sample_pin = A5;       // A5 == d19
const uint8_t WWVB_inverted_samples = 1;
const uint8_t WWVB_analog_samples = 1;
// const uint8_t WWVB_pin_mode = INPUT;  // disable internal pull up
const uint8_t WWVB_pin_mode = INPUT_PULLUP;  // enable internal pull up

const uint8_t WWVB_monitor_led = 18;  // A4 == d18

uint8_t ledpin(const uint8_t led) {
    return led;
}
#else
const uint8_t WWVB_sample_pin = 53;
const uint8_t WWVB_inverted_samples = 0;

// const uint8_t WWVB_pin_mode = INPUT;  // disable internal pull up
const uint8_t WWVB_pin_mode = INPUT_PULLUP;  // enable internal pull up

const uint8_t WWVB_monitor_led = 19;

uint8_t ledpin(const uint8_t led) {
    return led<14? led: led+(54-14);
}
#endif

uint8_t sample_input_pin() {
    const uint8_t sampled_data =
        #if defined(__AVR__)
        WWVB_inverted_samples ^ (WWVB_analog_samples? (analogRead(WWVB_analog_sample_pin) > 200)
                                                      : digitalRead(WWVB_sample_pin));
        #else
        WWVB_inverted_samples ^ digitalRead(WWVB_sample_pin);
        #endif

    digitalWrite(ledpin(WWVB_monitor_led), sampled_data);
    return sampled_data;
}

void setup() {
    using namespace Clock;

    Serial.begin(9600);
    Serial.println();
    Serial.println(F("Simple DCF77 Clock V3.1.1"));
    Serial.println(F("(c) Udo Klein 2016"));
    Serial.println(F("www.blinkenlight.net"));
    Serial.println();
    Serial.print(F("Sample Pin:      ")); Serial.println(WWVB_sample_pin);
    Serial.print(F("Sample Pin Mode: ")); Serial.println(WWVB_pin_mode);
    Serial.print(F("Inverted Mode:   ")); Serial.println(WWVB_inverted_samples);
    #if defined(__AVR__)
    Serial.print(F("Analog Mode:     ")); Serial.println(WWVB_analog_samples);
    #endif
    Serial.print(F("Monitor Pin:     ")); Serial.println(ledpin(WWVB_monitor_led));
    Serial.println();
    Serial.println();
    Serial.println(F("Initializing..."));

    pinMode(ledpin(WWVB_monitor_led), OUTPUT);
    pinMode(WWVB_sample_pin, WWVB_pin_mode);

    DCF77_Clock::setup();
    DCF77_Clock::set_input_provider(sample_input_pin);


    // Wait till clock is synced, depending on the signal quality this may take
    // rather long. About 5 minutes with a good signal, 30 minutes or longer
    // with a bad signal
    for (uint8_t state = Clock::useless;
        state == Clock::useless || state == Clock::dirty;
        state = DCF77_Clock::get_clock_state()) {

        // wait for next sec
        Clock::time_t now;
        DCF77_Clock::get_current_time(now);

        // render one dot per second while initializing
        static uint8_t count = 0;
        Serial.print('.');
        ++count;
        if (count == 60) {
            count = 0;
            Serial.println();
        }
    }
}

void paddedPrint(BCD::bcd_t n) {
    Serial.print(n.digit.hi);
    Serial.print(n.digit.lo);
}

void loop() {
    Clock::time_t now;

    DCF77_Clock::get_current_time(now);
    if (now.month.val > 0) {
        switch (DCF77_Clock::get_clock_state()) {
            case Clock::useless: Serial.print(F("useless ")); break;
            case Clock::dirty:   Serial.print(F("dirty:  ")); break;
            case Clock::synced:  Serial.print(F("synced: ")); break;
            case Clock::locked:  Serial.print(F("locked: ")); break;
        }
        Serial.print(' ');

        Serial.print(F("20"));
        paddedPrint(now.year);
        Serial.print('-');
        paddedPrint(now.month);
        Serial.print('-');
        paddedPrint(now.day);
        Serial.print(' ');

        paddedPrint(now.hour);
        Serial.print(':');
        paddedPrint(now.minute);
        Serial.print(':');
        paddedPrint(now.second);

        Serial.print("+0");
        Serial.print(now.uses_summertime? '2': '1');
        Serial.println();
    }
}
