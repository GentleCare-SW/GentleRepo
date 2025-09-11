/*
 * Copyright (c) 2025 GentleCare Corporation. All rights reserved.
 *
 * This source code and the accompanying materials are the confidential and
 * proprietary information of GentleCare Corporation. Unauthorized copying or
 * distribution of this file, via any medium, is strictly prohibited without
 * the prior written permission of GentleCare Corporation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "peripheral.h"
#include "button.h"
#include "auto_controller.h"
#include <Adafruit_SSD1306.h>

enum class ButtonType {
    STOP,
    PAUSE,
    INVERT,
    EVERT,
    CHAMBER,
    COUNT,
};

class ControlPanel : public Peripheral {
public:
    ControlPanel(uint32_t stop_pin, uint32_t pause_pin, uint32_t invert_pin, uint32_t evert_pin, uint32_t chamber_pin, AutoController *auto_controller, MotorController *motor_controller, PressureSensor *pressure_sensor, VoltageDimmer *dimmer);

    void start() override;

    void update(float dt) override;

private:
    void update_display();

    Button buttons[(uint32_t)ButtonType::COUNT];
    Adafruit_SSD1306 display;
    uint64_t last_display_update_time;
    AutoController *auto_controller;
    MotorController *motor_controller;
    PressureSensor *pressure_sensor;
    VoltageDimmer *dimmer;
};