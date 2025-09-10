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

ControlPanel::ControlPanel(uint32_t stop_pin, uint32_t pause_pin, uint32_t invert_pin, uint32_t evert_pin, uint32_t chamber_pin, AutoController *auto_controller)
{
    this->buttons[(uint32_t)ButtonType::STOP] = Button(stop_pin);
    this->buttons[(uint32_t)ButtonType::PAUSE] = Button(pause_pin);
    this->buttons[(uint32_t)ButtonType::INVERT] = Button(invert_pin);
    this->buttons[(uint32_t)ButtonType::EVERT] = Button(evert_pin);
    this->buttons[(uint32_t)ButtonType::CHAMBER] = Button(chamber_pin);
    this->auto_controller = auto_controller;
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
    this->display.setCursor(0, 0);
    this->display.println("Control Panel");
    this->display.display();
}

void ControlPanel::update(float dt)
{
    for (uint32_t i = 0; i < (uint32_t)ButtonType::COUNT; i++)
        this->buttons[i].update(dt);
    
    if (this->buttons[(uint32_t)ButtonType::STOP].on_release())
        this->auto_controller->set_mode((float)AutoControlMode::IDLE);
    else if (this->buttons[(uint32_t)ButtonType::PAUSE].on_release())
        this->auto_controller->toggle_paused();
    else if (this->buttons[(uint32_t)ButtonType::INVERT].on_release())
        this->auto_controller->set_mode((float)AutoControlMode::INVERSION);
    else if (this->buttons[(uint32_t)ButtonType::EVERT].on_release())
        this->auto_controller->set_mode((float)AutoControlMode::EVERSION);
}