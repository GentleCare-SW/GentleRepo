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
#include "servo.h"

static int32_t pwm_pin = -1;
static int64_t pulse_length = 500;
static int64_t pulse_start_time = 0;

static const int64_t DUTY_CYCLE = 3000;

void servo_initialize(int32_t pwm)
{
    pwm_pin = pwm;
    pinMode(pwm_pin, OUTPUT);
    pulse_start_time = micros();
}

void servo_set_angle(float angle)
{
    if (angle < 0.0)
        angle = 0.0;
    
    if (angle > 180.0)
        angle = 180.0;

    pulse_length = (int64_t)(angle / 180.0 * 2000.0 + 500.0);
}

void servo_update()
{
    int64_t current_time = micros();

    if (current_time >= pulse_start_time + DUTY_CYCLE)
        pulse_start_time = current_time;
    else if (current_time >= pulse_start_time + pulse_length)
        digitalWrite(pwm_pin, LOW);
    else
        digitalWrite(pwm_pin, HIGH);
}