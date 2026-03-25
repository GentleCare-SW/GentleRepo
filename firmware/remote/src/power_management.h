
#pragma once

#include <Arduino.h>
#include "config.h"
#include "Adafruit_MAX1704X.h"
#include "Adafruit_SSD1306.h"
#include "esp32-hal-timer.h"


class PowerManagement {
public:
    void begin();
    void update();
    void cutoff();
    int get_battery_percentage();
    
private:
    Adafruit_SSD1306 *display;
    Adafruit_MAX17048 fuelgauge;
    int battery_percentage;
};
