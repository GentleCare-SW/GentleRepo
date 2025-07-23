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

static inline void write_message(BluetoothSerial *serial, int code, float value = 0.0)
{
    uint8_t data[MESSAGE_LENGTH];
    data[0] = code;
    memcpy(&data[1], &value, sizeof(float));
    serial->write(data, MESSAGE_LENGTH);
}

static inline bool read_message(BluetoothSerial *serial, int *code = nullptr, float *value = nullptr)
{
    if (serial->available() >= MESSAGE_LENGTH) {
        uint8_t data[MESSAGE_LENGTH];
        serial->readBytes(data, MESSAGE_LENGTH);

        if (code != nullptr)
            *code = data[0];
        if (value != nullptr)
            memcpy(value, &data[1], sizeof(float));

        return true;
    }

    return false;
}

static inline bool wait_for_response(BluetoothSerial *serial, int64_t timeout_ms, int *code = nullptr, float *value = nullptr)
{
    int64_t start_time = millis();

    while (millis() - start_time < timeout_ms) {
        if (read_message(serial, code, value))
            return true;
    }

    return false;
}

#endif