# Experimentation

This document outlines the protocol and results for the experiments that have been conducted on GentleGlide.

## Experiment 1: Voltage as a Function of PWM

**Experiment**: Because the voltage supplied to the vacuum is not provided by a voltage regulator but instead comes from a triac voltage dimmer, the voltage cannot be controlled directly. Rather, a PWM signal is provided to the voltage dimmer (which is represented as a "percentage" between 0%-100%) which then determines how much voltage is output from the dimmer. The relationship between the PWM percentage and actual voltage is not necessicarily linear, therefore this experiment was conducted to determine the mapping between PWM and voltage through the use of a voltmeter. 

**Results**: 

https://www.desmos.com/calculator/cczbvugbbo

![](images/voltage_vs_pwm.png)

The black dots are the raw data points collected, and the red curve is a parametric fit for data points:

$$V(\text{PWM})=V_{\text{max}}\left(\frac{1-\cos(\pi(\text{PWM}+b))}{2}\right)\qquad0.1\leq\text{PWM}\leq 0.9,$$

$$\text{PWM}(V)=\frac{1}{\pi}\cos^{-1}\left(1-2\frac{V}{V_{\text{max}}}\right)-b\qquad 0.1\leq\text{PWM}\leq 0.9,$$

$$V_{\text{max}}\approx118.72,\quad b\approx0.07229.$$

**Conclusion**: The results show a strong non-linear relationship between PWM and voltage. For the following experiments, we use the curve fit provided above to determine what PWM percentage to use to achieve a desired voltage.

## Experiment 2: Pressure as a Function of Voltage

**Experiment**: This experiment determines the relationship between pressure inside the platform/sheet and voltage supplied to the air pump. The experiment was conducted at three different conditions: fully inverted, halfway inverted, and fully everted. For each condition, the experiment was conducted by recording a 20-30 second average of the pressure at each voltage level.

**Results**:

https://www.desmos.com/calculator/t076bu6jsy

![](images/pressure_vs_voltage.png)

Black is fully inverted, purple is halfway inverted and blue is fully everted. The lines of best fits are:

$$P=0.038V-0.34,$$
$$P=0.032V-0.40,$$
$$P=0.020V-0.22,$$

respectively.

**Conclusion**: The results show a linear relationship between voltage and pressure in each of the conditions, indicating that increasing the desired pressure requires a proportional increase in power towards the air pump.

## Experiment 3: Pressure Needed to Lift a Given Weight

**Experiment**: This experiment determines the amount of pressure needed to lift a given weight off the ground by exactly 3.5 inches. The experiment was conducted by placing a spine board on a fully everted GentleGlide and placing kettlebells ontop of the spine board to mimic a human weight. The weights were not distributed evenly, they were distributed such that the shoulder and hip areas had roughly double the weight as the lower-back and leg areas.

**Results**:

https://www.desmos.com/calculator/ozc46w6fde

![](images/pressure_vs_weight.png)

The line of best fit (red) is given by:

$$P=0.0046W+0.070.$$

**Conclusion**: The relationship between weight and the amount of pressure needed to lift the weight a given height shows to be linear. Lifting 300lbs required 1.42 PSI, indicating that the GentleGlide should achieve at minimum that much pressure while inverting underneath a 300lb human.

## Experiment 4: Torque Needed to Overcome a Given Pressure

**Experiment**: This experiment seeks to approximate the amount of torque required to invert with a given pressure inside the sheet. The experiment was conducted by first regulating the pressure to a constant setpoint, then winding the motor at a constant speed of 22.5 rpm until the cable was fully wrapped around the shaft. The average torque over time was recorded for each trial.

**Results**:

https://www.desmos.com/calculator/oniwtotm4w

![](images/torque_vs_pressure.png)

The line of best fit is given by:

$$\tau=1.82P+1.04.$$

**Conclusion**: The results show a linear relationship between pressure and torque, with each additional PSI of pressure requiring an additional 1.82 Nm of torque. Since lifting 300lbs requires 1.42 PSI, the motor should be able to achieve at least 3.62 Nm of torque for inversion.

## Experiment 5: Pressure Needed to Burrow Under a Given Weight

**Experiment**: This experiment determines how much pressure is needed to burrow under a given weight (concentrated roughly in a 2ftx2ft test-bed). The test-bed was set to a 45 degree angle and the pressure inside the GentleGlide was increased until it was able to burrow underneath the test-bed. There were two conditions tested: GentleGlide placed right next to the test-bed (representing burrowing under the shoulders) and GentleGlide placed ~2.5ft before the test-bed (representing burrowing under the hip).

**Results**:

https://www.desmos.com/calculator/6cjsuwd00v

![](images/pressure_vs_weight_burrowing.png)

The first condition (burrowing underneath shoulders) is shown in red, and the second condition (burrowing underneath hip) is shown in blue. The limit for how much weight GentleGlide was able to burrow under was 100lbs and 50lbs, respectively. Both showed a linear relationship with the following lines of best fits:

$$P=0.014W+0.45,$$
$$P=0.011W+0.52.$$

**Conclusion**: 

The difference between pressure needed to burrow under the shoulders vs hip shows to be negligible (at least until we can collect data on higher hip weights), indicating that the main challenge in burrowing underneath the hip is achieving high pressures despite leakage in the sheet. 

## Experiment 6: Pressure over Time as a Function of Leak

**Experiment**: TODO

**Results**:

**Conclusion**:

## Analysis

