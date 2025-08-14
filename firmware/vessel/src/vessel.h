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

#pragma once
#include <BLEDevice.h>
#include "peripheral.h"
#include "characteristic.h"

enum class VesselMode {
    IDLE,
    MANUAL_CONTROL,
    AUTOMATIC_CONTROL
};

class Vessel: public BLEServerCallbacks {
public:
    Vessel();

    void add_peripheral(Peripheral *peripheral);

    void start();

    void update();

    void onConnect(BLEServer *server);

    void onDisconnect(BLEServer *server);

private:
    VesselMode mode;
    BLEServer *server;
    BLEService *service;
    Peripheral *peripherals[MAX_PERIPHERALS];
    int peripheral_count;
    int64_t last_update_time;
};