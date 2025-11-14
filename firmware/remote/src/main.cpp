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

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "control_panel.h"
#include "remote_platform.h"
#include "inputs.h"
#include "config.h"

static Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire);
static RemotePlatform platform(&display);
static ControlPanel panel(&platform, &display);
long start_millis;  

void setup()
{
    Serial.begin(BAUD_RATE);
    while (!Serial);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();
    
    static int32_t buttons[] = { 
        BUTTON_STOP_PIN, 
        BUTTON_PAUSE_PIN, 
        BUTTON_INVERT_PIN, 
        BUTTON_EVERT_PIN, 
        BUTTON_SERVO_PIN, 
        BUTTON_CHAMBER_PIN, 
        BUTTON_STOP_AIR1_PIN, 
        BUTTON_STOP_AIR2_PIN,
        BUTTON_STOP_MOTOR_PIN };

    #if PLATFORM_TYPE==0
        static uint32_t knob_dt_pins[] = { KNOB_MOTOR_DT_PIN, KNOB_AIR_DT_PIN, KNOB_SERVO_DT_PIN, KNOB_VALVE_DT_PIN };
        static uint32_t knob_clk_pins[] = { KNOB_MOTOR_CLK_PIN, KNOB_AIR_CLK_PIN, KNOB_SERVO_CLK_PIN, KNOB_VALVE_CLK_PIN };
        Knob MOTOR_KNOB = {MOTOR_VELOCITY_UUID, 1.0, -30.0, 30.0};
        Knob AIR_KNOB = {CENTRAL_DIMMER_UUID, 1.0, 0.0, 120.0};
        Knob SERVO_KNOB = {SERVO_ANGLE_UUID, 1.0, SERVO_ANGLE1, SERVO_ANGLE2};
        Knob VALVE_KNOB = {PROPORTIONAL_VALVE_UUID, 0.2, 0.0, 15.0};
        static Knob knob_params[] = { MOTOR_KNOB, AIR_KNOB, SERVO_KNOB, VALVE_KNOB };
    #else
        static uint32_t knob_dt_pins[] = { KNOB_MOTOR_DT_PIN, KNOB_AIR1_DT_PIN, KNOB_AIR2_DT_PIN };
        static uint32_t knob_clk_pins[] = { KNOB_MOTOR_CLK_PIN, KNOB_AIR1_CLK_PIN, KNOB_AIR2_CLK_PIN };
        Knob MOTOR_KNOB = {MOTOR_VELOCITY_UUID, 1.0, -30.0, 30.0};
        Knob AIR1_KNOB = {CENTRAL_DIMMER_UUID, 1.0, 0.0, 120.0};
        Knob AIR2_KNOB = {OUTER_DIMMER_UUID, 1.0, 0.0, 120.0};
        static Knob knob_params[] = { MOTOR_KNOB, AIR1_KNOB, AIR2_KNOB };
    #endif
    panel.start(buttons, knob_dt_pins, knob_clk_pins, knob_params);

    platform.start();
}

void loop()
{   
    platform.update();
    panel.update();

#if DEBUG_MODE
    Serial.printf(">Voltage: %f\n", platform.get(PROPORTIONAL_VALVE_UUID));
#endif
}