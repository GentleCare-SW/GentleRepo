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
#include "config.h"
#include "remote_platform.h"
#include "common/uuids.h"


// Replace with your network credentials
const char* ssid     = "GentleCare";
const char* password = "WeLoveRobots";

RemotePlatform::RemotePlatform(Adafruit_SSD1306 *display, PowerManagement *power)
{
    this->found_device = false;
    this->display = display;
    this->power = power;
}

void RemotePlatform::start()
{
    NimBLEDevice::init("GentleBombDetonator");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    this->scanner = NimBLEDevice::getScan();
    this->scanner->setScanCallbacks(this);
    this->scanner->setActiveScan(true);
    this->scanner->setMaxResults(0);

    this->client = NimBLEDevice::createClient();

    this->display->clearDisplay();
    this->display->drawBitmap(0, 0, logo_bitmap, 128, 64, WHITE);
    this->display->display();
    
    WiFi.begin(ssid, password);
    delay(2000);
    this->internet_connection = WiFi.status() == WL_CONNECTED;

    if (this->internet_connection) {
        // TZ string carries the DST rules; SNTP syncs in the background.
        Serial.println("Wifi Connected");
        configTzTime("EST5EDT,M3.2.0,M11.1.0", "pool.ntp.org", "time.nist.gov");
    }

    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(500);
    //     Serial.print(".");
    // }
    // // Print local IP address and start web server
    // Serial.println("");
    // Serial.println("WiFi connected.");
    // Serial.println("IP address: ");
    // Serial.println(WiFi.localIP());

}

void RemotePlatform::update()
{
    if (!this->client->isConnected()) {
        this->display->clearDisplay();
        this->display->drawBitmap(0, 0, logo_bitmap, 128, 64, WHITE);
        this->display->drawBitmap(120, 0, battery_icon, 8, 13, WHITE);
        this->display->display();
    }

    while (!this->client->isConnected()) {
        if (!this->scanner->isScanning())
            this->scanner->start(0);

        if ((digitalRead(BUTTON_POWER_PIN))) {         //if power button is pressed
            delay(1000);                                // Debounce delay
            if ((digitalRead(BUTTON_POWER_PIN))) {     //if still pressed after debounce delay
                this->power->cutoff();              // Cut power to the system
            }
        }
        
        if (millis() - this->last_display_update >= 500) {
            this->last_display_update = millis();
            this->power->update();      
            this->display->fillRect(122, 2, 5, 10, BLACK); //black out previous percentage
            int bat = (int)(this->power->get_battery_percentage()*0.1);
            this->display->fillRect(122, 2+(10-bat), 5, bat, WHITE);
            //this->display->setCursor(102, 8);
            //this->display->printf("%d%%", this->power->get_battery_percentage());

            if ((digitalRead(CHARGE_DETECT_PIN) == LOW))    // if charging
                this->display->drawBitmap(119, 56, lightning_icon, 8, 8, WHITE);
            else
                this->display->fillRect(119, 56, 8, 8, BLACK);

            this->display->display();   
        }

        if (this->found_device) {
            Serial.println("found device");
            this->found_device = false;
            this->scanner->stop();

            //delay(1000); //I forgot why this is here
            this->display->clearDisplay();
            this->display->setCursor(0, 0);
            if (this->device->getName() == "Glide")
                this->display->println("Connecting to \nGentleGlide");
            else if (this->device->getName() == "Wedge")
                this->display->println("Connecting to \nGentleWedge");

            this->display->display();
            
            // Set desired connection params before connecting so they apply to
            // this link. 12-24 (15-30ms) interval, 0 latency, 400 (4s) supervision
            // timeout - a longer timeout tolerates brief RF interference without
            // dropping the link.
            this->client->setConnectionParams(6, 20, 0, 200);

            if (!this->client->connect(this->device)) {
                Serial.println("connect failed");
                this->found_device = false;     // fall back to scanning
                continue;
            }

            this->service = this->client->getService(SERVICE_UUID);
            if (this->service == nullptr) {
                Serial.println("service not found");
                this->client->disconnect();
                this->found_device = false;     // fall back to scanning
                continue;
            }

            for (int i = 0; i < CHARACTERISTIC_UUID_COUNT; i++) {
                this->characteristics[i] = this->service->getCharacteristic(CHARACTERISTIC_UUIDS[i]);
                if (this->characteristics[i] != nullptr && this->characteristics[i]->canNotify())
                    this->characteristics[i]->subscribe(true, std::bind(&RemotePlatform::on_notification, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
                this->values[i] = 0.0;
            }
            Serial.println("Bluetooth successfully initialized");
        }
    }
}

void RemotePlatform::onResult(const NimBLEAdvertisedDevice *device)
{
#if DEBUG_MODE
    Serial.printf("Device: %s\n", device->toString().c_str());
#endif
    if (device->getServiceUUID().toString() != SERVICE_UUID)
        return;
    if (device->getName() != CONNECTION_NAME)
        return;
    this->device = (NimBLEAdvertisedDevice *)device;
    this->found_device = true;
}

void RemotePlatform::on_notification(NimBLERemoteCharacteristic *characteristic, uint8_t *data, size_t length, bool isNotify)
{
    if (length != sizeof(float))
        return;

    for (int i = 0; i < CHARACTERISTIC_UUID_COUNT; i++) {
        if (strcmp(CHARACTERISTIC_UUIDS[i], characteristic->getUUID().toString().c_str()) == 0)
            this->values[i] = *(float *)data;
    }
}

NimBLERemoteCharacteristic *RemotePlatform::get_characteristic(const char *uuid)
{
    for (int i = 0; i < CHARACTERISTIC_UUID_COUNT; i++) {
        if (strcmp(CHARACTERISTIC_UUIDS[i], uuid) == 0)
            return this->characteristics[i];
    }

    return nullptr;
}

float RemotePlatform::get(const char *uuid)
{
    for (int i = 0; i < CHARACTERISTIC_UUID_COUNT; i++) {
        if (strcmp(CHARACTERISTIC_UUIDS[i], uuid) == 0)
            return this->values[i];
    }
    return 0.0;
}

void RemotePlatform::set(const char *uuid, float value, bool with_response)
{
    if (!this->client->isConnected())
        return;
    
    NimBLERemoteCharacteristic *characteristic = this->get_characteristic(uuid);
    if (characteristic == nullptr)
        return;

    characteristic->writeValue((uint8_t *)&value, sizeof(value), with_response);
}
