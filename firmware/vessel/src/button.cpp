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

#include "button.h"

Button::Button()
{
    this->pin = -1;
    this->pressed = false;
    this->state_changed = false;
}

Button::Button(uint32_t pin)
{
    this->pin = pin;
    this->pressed = false;
    this->state_changed = false;
}

void Button::start()
{
    pinMode(this->pin, INPUT_PULLUP);
}

void Button::update(float dt)
{
    bool pressed = digitalRead(this->pin) == LOW;
    this->state_changed = pressed != this->pressed;
    this->pressed = pressed;
}

bool Button::on_release()
{
    return this->state_changed && !this->pressed;
}

bool Button::on_press()
{
    return this->state_changed && this->pressed;
}