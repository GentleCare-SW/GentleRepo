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
#include <ESP32Encoder.h>
#include <Adafruit_SSD1306.h>
#include "remote_platform.h"

enum class ButtonType {
    STOP,
    INVERT,
    EVERT,
    PAUSE,
    CHAMBER,
    STOP_AIR,
    STOP_MOTOR,
    COUNT
};

enum class KnobType {
    AIR,
    MOTOR,
    COUNT
};

class ControlPanel {
public:
    ControlPanel(RemotePlatform *platform, Adafruit_SSD1306 *display);

    void start(uint32_t button_pins[(int)ButtonType::COUNT], uint32_t knob_dt_pins[(int)KnobType::COUNT], uint32_t knob_clk_pins[(int)KnobType::COUNT]);

    void update();

private:
    void update_buttons();

    void update_knobs();

    void update_display();
    
    RemotePlatform *platform;
    uint32_t button_pins[(int)ButtonType::COUNT];
    bool button_pressed[(int)ButtonType::COUNT];
    ESP32Encoder knobs[(int)KnobType::COUNT];
    int64_t last_knob_positions[(int)KnobType::COUNT]; 
    Adafruit_SSD1306 *display;
};