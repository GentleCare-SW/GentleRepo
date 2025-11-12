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

#include "servo.h"
#include "config.h"

static const float MAX_MICROSECONDS = 2500.0;
static const float MIN_MICROSECONDS = 500.0;
static const float SERVO_UPDATE_INTERVAL = 0.2;

Servo::Servo(const char *angle_uuid, const char *chamber_uuid, int32_t pwm_pin, int32_t ledc_channel)
{
    this->pwm_pin = pwm_pin;
    this->ledc_channel = ledc_channel;
    this->angle = SERVO_ANGLE1;
    this->chamber = 0.0;

    this->add_characteristic(angle_uuid, std::bind(&Servo::set_angle, this, std::placeholders::_1), std::bind(&Servo::get_angle, this));
    this->add_characteristic(chamber_uuid, std::bind(&Servo::set_chamber, this, std::placeholders::_1), std::bind(&Servo::get_chamber, this));
}

void Servo::start()
{
    pinMode(this->pwm_pin, OUTPUT);
    ledcSetup(this->ledc_channel, 400, 16);
    ledcAttachPin(this->pwm_pin, this->ledc_channel);
    this->set_chamber(0.0);
    this->last_update_time = micros();

}

void Servo::update(float dt)
{
    Peripheral::update(dt);

    uint32_t current_time = micros();
    if (current_time - this->last_update_time > (uint32_t)(SERVO_UPDATE_INTERVAL * 1e6)) {
        dt = (current_time - this->last_update_time) / 1e6;
        this->last_update_time = current_time;
        // if (this->goal_angle != this->angle){
        //     this->angle = (this->goal_angle-this->angle)>0 ? this->angle+1 : this->angle-1;
        //     this->set_angle(this->angle);
        // }
    }
}

void Servo::set_angle(float angle)
{
    this->angle = constrain(angle, 0.0, 180.0);
    uint32_t duty_cycle = (this->angle / 180.0 * (MAX_MICROSECONDS - MIN_MICROSECONDS) + MIN_MICROSECONDS) / MAX_MICROSECONDS * 0xffff;
    ledcWrite(this->ledc_channel, duty_cycle);
}

float Servo::get_angle()
{
    return this->angle;
}

void Servo::set_chamber(float chamber)
{
    this->angle = chamber == 0.0 ? SERVO_ANGLE1 : SERVO_ANGLE2;
    this->set_angle(this->angle);
}

float Servo::get_chamber()
{
    return this->chamber;
}