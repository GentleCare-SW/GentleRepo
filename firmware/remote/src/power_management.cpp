

#include <Arduino.h>
#include "config.h"
#include "power_management.h"
#include "esp32-hal-timer.h"



void PowerManagement::begin() {
    pinMode(POWER_ENABLE_PIN, OUTPUT);
    digitalWrite(POWER_ENABLE_PIN, HIGH);           // Enable power to the system
    pinMode(CHARGE_DETECT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_POWER_PIN, INPUT);
    fuelgauge.begin();
    this->battery_percentage = 0;
}


void PowerManagement::update() { 
    // Check if the power button is pressed
    if ((digitalRead(BUTTON_POWER_PIN))) {         //if power button is pressed
        delay(1000);                                // Debounce delay
        if ((digitalRead(BUTTON_POWER_PIN))) {     //if still pressed after debounce delay
            PowerManagement::cutoff();              // Cut power to the system
        }
    }
    this->battery_percentage = (int)fuelgauge.cellPercent();
    /*
    this->display->setCursor(103, 0);
    this->display->printf("BAT:");
    this->display->fillRect(102, 8, 32, 8, BLACK); 
    this->display->setCursor(102, 8);
    BatteryPercentage = fuelgauge.cellPercent();
    if (BatteryPercentage > 100) {
        BatteryPercentage = 100;
    }   
    
    this->display->printf("%d%%", (int)BatteryPercentage);

    if ((digitalRead(CHARGE_DETECT_PIN) == LOW)) { //if charging
        this->display->drawBitmap(119, 56, lightning_icon, 8, 8, WHITE); 
    }
    else {
        this->display->fillRect(119, 56, 8, 8, BLACK); 
    }
    */

}

    
void PowerManagement::cutoff() {
    digitalWrite(POWER_ENABLE_PIN, LOW);            // Cut power to the system
    //this->display->clearDisplay();
    //this->display->display();
}   

int PowerManagement::get_battery_percentage() {
    return this->battery_percentage;
}

