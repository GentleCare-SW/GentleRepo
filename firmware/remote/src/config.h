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

#if PLATFORM_TYPE == 0

#define DEVICE_NAME "GentleWedge"
#define CONNECTION_NAME "Wedge"

#define BUTTON_STOP_PIN 14
#define BUTTON_PAUSE_PIN 27
#define BUTTON_INVERT_PIN 26
#define BUTTON_EVERT_PIN 25
#define BUTTON_CHAMBER_PIN 33
#define BUTTON_STOP_AIR_PIN 4
#define BUTTON_STOP_MOTOR_PIN 19
#define KNOB_AIR_DT_PIN 16
#define KNOB_AIR_CLK_PIN 17
#define KNOB_MOTOR_DT_PIN 18
#define KNOB_MOTOR_CLK_PIN 5


#elif PLATFORM_TYPE == 1

#define DEVICE_NAME "GentleGlide"
#define CONNECTION_NAME "Glide"

#define BUTTON_STOP_PIN 14
#define BUTTON_PAUSE_PIN 27
#define BUTTON_INVERT_PIN 26
#define BUTTON_EVERT_PIN 25
#define BUTTON_CHAMBER_PIN 33
#define BUTTON_STOP_AIR_PIN 4
#define BUTTON_STOP_MOTOR_PIN 19
#define KNOB_AIR_DT_PIN 16
#define KNOB_AIR_CLK_PIN 17
#define KNOB_MOTOR_DT_PIN 18
#define KNOB_MOTOR_CLK_PIN 5

#else

#error "Unknown platform type"

#endif