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

#include "steering.h"
#include "config.h"


Steering::Steering(const char *joystick_uuid, int32_t left_valve_pin, int32_t right_valve_pin)
{
    this->left_valve_pin = left_valve_pin;
    this->right_valve_pin = right_valve_pin;
    this->direction = 0.0;

    this->add_characteristic(joystick_uuid, std::bind(&Steering::set_direction, this, std::placeholders::_1), std::bind(&Steering::get_direction, this));
    
}

void Steering::start()
{
    pinMode(this->left_valve_pin, OUTPUT);
    pinMode(this->right_valve_pin, OUTPUT);
    this->set_direction(0.0);
}

void Steering::set_direction(float joystick_x)
{
    this->direction = joystick_x;
    if (joystick_x == 0.0) {
        digitalWrite(this->left_valve_pin, LOW);
        digitalWrite(this->right_valve_pin, LOW);
    } else if (joystick_x == 1.0) {
        digitalWrite(this->left_valve_pin, HIGH);
        digitalWrite(this->right_valve_pin, LOW);
        Serial.println("turning right");
    } else if (joystick_x == -1.0) {
        digitalWrite(this->left_valve_pin, LOW);
        digitalWrite(this->right_valve_pin, HIGH);
        Serial.println("turning left");
    }
}

float Steering::get_direction()
{
    return this->direction;
}
