/*
 * Copyright (c) 2026 GentleCare Corporation. All rights reserved.
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

#include <math.h>
#include "can_motor_controller.h"
#include "config.h"
// #include <SPI.h>


static const float MOTOR_UPDATE_INTERVAL = 0.02;

// --- Board wiring -----------------------------------------------------------
// TODO: move to config.h 
static const byte CAN_CS_PIN = 5 ; //aka ncs or scn
// static const byte SCK_PIN  = 18 ; 
// static const byte MOSI_PIN = 23 ;
// static const byte MISO_PIN = 19 ; 

static const byte CAN_INT_PIN = 4;

#define CAN_ARBITRATION_RATE (1000UL * 1000UL)
#define CAN_DATA_RATE_FACTOR DataBitRateFactor::x5

// The driver and controller are file-scope objects, exactly like the reference
// sketch. `can` must be declared before `motor` because Moteus holds a reference
// to it. Keeping `can` at file scope also lets the ISR be a captureless lambda
// (`[]{ can.isr(); }`), which is the only form can.begin()/attachInterrupt()
// accepts.
ACAN2517FD can(CAN_CS_PIN, SPI, CAN_INT_PIN);
Moteus motor(can, []() {
    Moteus::Options options;
    options.id = 1;
    return options;
}());


CanMotorController::CanMotorController(const char *position_uuid, const char *velocity_uuid, const char *torque_uuid, const char *error_uuid, HardwareSerial *serial)
    : MotorController(position_uuid, velocity_uuid, torque_uuid, error_uuid, serial, -1, -1)
{

    this->cmd_position = NaN;
    this->cmd_velocity = 0.0;

    // MotorController's constructor bound the velocity write
    // characteristics to its own (UART) set_velocity. Re-point them
    // at the CAN overrides so BLE writes take the CAN path
    for (int i = 0; i < this->characteristic_count; i++) {
        const char *uuid = this->characteristics[i].uuid;
        if (uuid == velocity_uuid)
            this->characteristics[i].setter = std::bind(&CanMotorController::set_velocity, this, std::placeholders::_1);
    }
}

void CanMotorController::start()
{
    SPI.begin();

    //does it still work with data x5?
    ACAN2517FDSettings settings(ACAN2517FDSettings::OSC_40MHz, 1000ll * 1000ll, DataBitRateFactor::x1); 
    settings.mArbitrationSJW = 2;
    settings.mDriverTransmitFIFOSize = 1;
    settings.mDriverReceiveFIFOSize = 2;

    const uint32_t errorCode = can.begin(settings, [] { can.isr(); });
    Serial.println("Trying to start can");

    while (errorCode != 0) {
        Serial.print(F("CAN error 0x"));
        Serial.println(errorCode, HEX);
        delay(1000);
    }
    Serial.println("CAN init ok");

    motor.SetStop();

    this->set_velocity(0.0);
    this->last_update_time = micros();
}

void CanMotorController::update(float dt)
{
    Peripheral::update(dt);

    uint32_t current_time = micros();
    if (current_time - this->last_update_time > (uint32_t)(MOTOR_UPDATE_INTERVAL * 1e6)) {
        this->last_update_time = current_time;
        this->loopCount++;

        // Re-issue the current command every cycle. moteus faults into a stop if
        // it stops hearing position commands, so this must run continuously, not
        // just when set_velocity() is called (this is what the sketch's loop does).
        this->send_command();

        if (this->loopCount % 5 == 0) {
            moteus_query(motor.last_result().values);
            Serial.printf("Pos: %f  Vel: %f  Torque: %f \n", this->position, this->velocity, this->torque);
        }
    }
}

void CanMotorController::mode_changed(ServiceMode mode)
{
    this->set_velocity(0.0);
}

void CanMotorController::set_velocity(float velocity)
{
    // velocity is the desired output-shaft speed in rpm; convert to motor rev/s
    // and store it. The next update() cycle sends it over CAN, so this stays
    // cheap and non-blocking even when called from the BLE write callback.
    this->cmd_position = NaN;
    this->cmd_velocity = velocity * GEARBOX_RATIO / 60.0;
}

void CanMotorController::send_command()
{
    Moteus::PositionMode::Command cmd;
    cmd.position = this->cmd_position;
    cmd.velocity = this->cmd_velocity;
    bool ok = motor.SetPosition(cmd);
    if (!ok)
        this->set_error((float)MotorControllerError::NOT_RESPONDING);
    else
        this->set_error((float)MotorControllerError::NONE);
}

void CanMotorController::moteus_query(const Moteus::Query::Result& query)
{
    this->position = query.position / GEARBOX_RATIO;
    this->velocity = query.velocity * 60.0 / GEARBOX_RATIO;
    this->torque = query.torque * GEARBOX_RATIO;
}


// float CanMotorController::get_velocity()
// {
//     return this->velocity;
// }

// float CanMotorController::get_position()
// {
//     return this->position;
// }

// float CanMotorController::get_torque()
// {
//     return this->torque;
// }

// void CanMotorController::set_error(float error)
// {
//     this->error = MotorControllerError((int)error);
// }

// float CanMotorController::get_error()
// {
//     return (float)this->error;
// }

// bool CanMotorController::is_ok()
// {
//     return this->error == MotorControllerError::NONE;
// }


