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

#include "rail.h"
#include "config.h"


Rail::Rail(const char *joystick_uuid, int32_t left_pin, int32_t right_pin)
{
    this->left_pin = left_pin;
    this->right_pin = right_pin;
    this->direction = 0.0;
    
    this->add_characteristic(joystick_uuid, std::bind(&Rail::set_direction, this, std::placeholders::_1), std::bind(&Rail::get_direction, this));
}

void Rail::start()
{
    pinMode(this->left_pin, OUTPUT);
    pinMode(this->right_pin, OUTPUT);
    this->set_direction(0.0);
}

void Rail::set_direction(float joystick_x)
{
    this->direction = joystick_x;
    if (joystick_x == 0.0) {
        digitalWrite(this->left_pin, HIGH);
        digitalWrite(this->right_pin, HIGH);
    } else if (joystick_x == -1.0) { //down
        digitalWrite(this->left_pin, HIGH);
        digitalWrite(this->right_pin, LOW);
    } else if (joystick_x == 1.0) {  //up
        digitalWrite(this->left_pin, LOW);
        digitalWrite(this->right_pin, HIGH);
    } else if (joystick_x == 2.0) {
        digitalWrite(this->left_pin, LOW);
        digitalWrite(this->right_pin, LOW);
    }
}

float Rail::get_direction()
{
    return this->direction;
}
