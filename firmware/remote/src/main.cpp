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
#include <BluetoothSerial.h>

static BluetoothSerial SerialBT;

void setup()
{
    Serial.begin(BAUD_RATE);
    while (!Serial);

    SerialBT.begin("GentleRemote", true);
    while (!SerialBT);
}

void loop()
{
    while (!SerialBT.connected()) {
        Serial.printf("Trying to connect to device...\n");
        if (!SerialBT.connect("GentleWedge"))
            Serial.printf("Could not find device, retrying...\n");
        else
            Serial.printf("Connected to device!\n");
    }

    while (SerialBT.available())
        Serial.printf("%c", SerialBT.read());
}