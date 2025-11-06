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
    
    static uint32_t buttons[] = { BUTTON_STOP_PIN, BUTTON_INVERT_PIN, BUTTON_EVERT_PIN, BUTTON_PAUSE_PIN, BUTTON_CHAMBER_PIN, BUTTON_STOP_AIR_PIN, BUTTON_STOP_MOTOR_PIN };
    static uint32_t knob_dt_pins[] = { KNOB_AIR_DT_PIN, KNOB_MOTOR_DT_PIN, KNOB_SERVO_DT_PIN };
    static uint32_t knob_clk_pins[] = { KNOB_AIR_CLK_PIN, KNOB_MOTOR_CLK_PIN, KNOB_SERVO_CLK_PIN };
    panel.start(buttons, knob_dt_pins, knob_clk_pins);

    platform.start();
}

void loop()
{   
    platform.update();
    panel.update();

#if DEBUG_MODE
    Serial.printf(">Servo angle: %f\n", platform.get(SERVO_ANGLE_UUID));
#endif
}