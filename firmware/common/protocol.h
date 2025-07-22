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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

static const int32_t MESSAGE_LENGTH = 5;

typedef enum command_code {
    COMMAND_CODE_SET_DIMMER_LEVEL = 'd',
    COMMAND_CODE_SET_CHAMBER = 'c',
    COMMAND_CODE_SET_SERVO_ANGLE = 's',
    COMMAND_CODE_SET_PRESSURE_REFERENCE = 'a',
    COMMAND_CODE_SET_MOTOR_VELOCITY = 'v',
    COMMAND_CODE_GET_DIMMER_LEVEL = 'D',
    COMMAND_CODE_GET_SERVO_ANGLE = 'S',
    COMMAND_CODE_GET_PRESSURE = 'R',
    COMMAND_CODE_GET_PRESSURE_REFERENCE = 'A',
    COMMAND_CODE_GET_MOTOR_VELOCITY = 'V',
    COMMAND_CODE_GET_MOTOR_POSITION = 'P',
    COMMAND_CODE_GET_MOTOR_CURRENT = 'M',
} command_code_t;

typedef enum response_code {
    RESPONSE_CODE_OK = 'o',
    RESPONSE_CODE_ERROR = 'e',
} response_code_t;

typedef union message_content {
    int32_t int_value;
    float float_value;
} message_content_t;

#endif