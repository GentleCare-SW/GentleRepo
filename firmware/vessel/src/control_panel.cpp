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

#include "control_panel.h"
#include "config.h"

static const float DISPLAY_UPDATE_INTERVAL = 0.1;

ControlPanel::ControlPanel(uint32_t stop_pin, uint32_t pause_pin, uint32_t invert_pin, uint32_t evert_pin, uint32_t chamber_pin, AutoController *auto_controller, MotorController *motor_controller, PressureSensor *pressure_sensor, VoltageDimmer *dimmer, Servo *servo)
{
    this->buttons[(uint32_t)ButtonType::STOP] = Button(stop_pin);
    this->buttons[(uint32_t)ButtonType::PAUSE] = Button(pause_pin);
    this->buttons[(uint32_t)ButtonType::INVERT] = Button(invert_pin);
    this->buttons[(uint32_t)ButtonType::EVERT] = Button(evert_pin);
    this->buttons[(uint32_t)ButtonType::CHAMBER] = Button(chamber_pin);
    this->auto_controller = auto_controller;
    this->motor_controller = motor_controller;
    this->pressure_sensor = pressure_sensor;
    this->dimmer = dimmer;
    this->servo = servo;
    this->last_display_update_time = 0;
}

void ControlPanel::start()
{
    for (uint32_t i = 0; i < (uint32_t)ButtonType::COUNT; i++)
        this->buttons[i].start();
    
    this->display = Adafruit_SSD1306(CONTROL_PANEL_DISPLAY_WIDTH, CONTROL_PANEL_DISPLAY_HEIGHT, &Wire);
    this->display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    this->display.clearDisplay();
    this->display.setTextSize(1);
    this->display.setTextColor(SSD1306_WHITE);
    this->display.display();
    this->last_display_update_time = millis();
}

void ControlPanel::update_display()
{
    this->display.clearDisplay();

    this->display.setCursor(0, 0);
    this->display.printf("Status: ");
    switch ((AutoControlMode)this->auto_controller->get_mode()) {
        case AutoControlMode::IDLE:
            this->display.printf("Idle\n");
            break;
        case AutoControlMode::INVERSION:
            this->display.printf("Inverting\n");
            break;
        case AutoControlMode::EVERSION:
            this->display.printf("Everting\n");
            break;
        case AutoControlMode::INVERSION_PAUSED:
        case AutoControlMode::EVERSION_PAUSED:
            this->display.printf("Paused\n");
            break;
        default:
            this->display.printf("Unknown\n");
            break;
    }
    if ((MotorControllerError)this->motor_controller->get_error() != MotorControllerError::NONE)
        this->display.printf("MOTOR ERROR: %i\n", (int)this->motor_controller->get_error());
    else if ((PressureSensorError)this->pressure_sensor->get_error() != PressureSensorError::NONE)
        this->display.printf("PRESSURE ERROR: %i\n", (int)this->pressure_sensor->get_error());

    this->display.setCursor(0, 16);
    float progress = this->auto_controller->get_progress();
    this->display.printf("Progress: %.1f%%\n", progress * 100.0);
    this->display.fillRect(0, 24 + 1, (int16_t)(CONTROL_PANEL_DISPLAY_WIDTH * progress), 8 - 2, SSD1306_WHITE);
    this->display.drawRect(0, 24 + 1, CONTROL_PANEL_DISPLAY_WIDTH, 8 - 2, SSD1306_WHITE);

    this->display.setCursor(0, 40);
    this->display.printf("Voltage: %.1f%%\n", this->dimmer->get_percentage() * 100.0);
    this->display.printf("Pressure: %.1f PSI\n", this->pressure_sensor->get_pressure());
    this->display.printf("Torque: %.2f Nm\n", this->motor_controller->get_torque());

    this->display.display();
}

void ControlPanel::update(float dt)
{
    for (uint32_t i = 0; i < (uint32_t)ButtonType::COUNT; i++)
        this->buttons[i].update(dt);
    
    if (this->buttons[(uint32_t)ButtonType::STOP].on_release())
        this->auto_controller->set_mode((float)AutoControlMode::IDLE);
    if (this->buttons[(uint32_t)ButtonType::PAUSE].on_release())
        this->auto_controller->toggle_paused();
    if (this->buttons[(uint32_t)ButtonType::INVERT].on_release())
        this->auto_controller->set_mode((float)AutoControlMode::INVERSION);
    if (this->buttons[(uint32_t)ButtonType::EVERT].on_release())
        this->auto_controller->set_mode((float)AutoControlMode::EVERSION);

#if ENABLE_SERVO
    if (this->buttons[(uint32_t)ButtonType::CHAMBER].on_release())
        this->servo->set_chamber(1 - this->servo->get_chamber());
#endif
    
    if (millis() - this->last_display_update_time > DISPLAY_UPDATE_INTERVAL * 1000) {
        this->last_display_update_time = millis();
        this->update_display();
    }
}