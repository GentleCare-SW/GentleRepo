# Control Strategy

This document outlines the control strategy employed in the GentleGlide/Wedge.

**Legend**:

| Symbol | Description |
|-|-|
| $\theta$ | Motor angle (referred to as "position"). |
| $\dot{\theta}$ | Motor angular velocity (referred to as "velocity") |
| $\dot{\theta}_{\text{ref}}$ | Reference or desired motor velocity. |
| $\tau$ | Measured torque that the motor is applying. |
| $\tau_{\text{ref}}$ | Reference or desired torque. |
| $v$ | The "percentage" of voltage supplied to the vacuum. The relationship to the actual voltage is non-linear. |
| $P$ | The measured air pressure inside the vessel. |
| $P_{\text{ref}}$ | The reference or desired pressure. |

## Eversion

For eversion, the goal of the control loop is to regulate the tension in the cable by commanding the air supply to increase when tension is too low, and decrease when the tension is too high. In cases where the desired tension cannot be achieved, the motor should slow down to a halt, which prevents the motor from unwinding the cable too far when the sheet is jammed.

This control strategy has the effect of automatically adjusting pressure depending on the load that the sheet is under (e.g. the upper back area is a heavier load so the pressure would automatically increase when burrowing underneath, then decrease back to a lower level once getting past that hurdle).

To achieve this control strategy, we employ the following control loop:

![](images/eversion_control_loop.png)

The measured torque $\tau$ is subtracted from the hand-tuned reference torque $\tau_{\text{ref}}$ which produces an error $e$ that is supplied as input to two separate proportional-derivative (PD) blocks.

The first PD block modulates the desired motor velocity $\dot{\theta}_{\text{ref}}$ such that the velocity decreases when the torque is too far below the setpoint, and increases when the torque is at or near the setpoint. Similarly, the second PD block modulates the vacuum's voltage supply $v$ such that the air supply is increased when the torque is below the setpoint and decreases otherwise. Both $\dot{\theta}_{\text{ref}}$ and $v$ are constrained to minimum and maximum values to prevent excessive speed or air supply.

The reference velocity and voltage supply are then supplied into the velocity controller and vacuum, respectively, which produces an actual motor velocity $\dot{\theta}$ and pressure $P$ inside the vessel. A new torque $\tau$ is then produced and measured, closing the feedback loop.

Because the shaft's radius $r$ is not constant, but depends on the motor's position $\theta$, this controller does *not* actually achieve constant cable tension throughout the eversion. To achieve constant tension, it's possible to model $r$ as having a linear relationship to $\theta$, which would in turn make $\tau_{\text{ref}}$ dependent on $\theta$. However, we have found that achieving constant tension is not strictly necessary. By tuning $\tau_{\text{ref}}$ to the smallest possible shaft radius $r_{\text{min}}$, the control loop can continue to maintain sufficient tension in the cable for larger radii without introducing new issues. We therefore opt to the simpler solution of tuning $\tau_{\text{ref}}$ to be large enough to work under the minimum shaft radius rather than modeling it as a function of $\theta$.

## Inversion

For inversion, the goal is simply to maintain a defined reference velocity $\dot{\theta}_{\text{ref}}$ and pressure $P_{\text{ref}}$ by commanding the motor current and vacuum's voltage supply.

We define $\dot{\theta}_{\text{ref}}$ as a piecewise-linear function of $\theta$ in the following form:

![](images/inversion_theta_dot.png)

The velocity has a ramp down to $0$ to counteract the effect of the sheet inverting faster as the radius of the shaft increases. Since the on-board ODrive controller encapsulates the velocity control loop, the velocity controller's implementation details are not provided here.

$P_{\text{ref}}$ is defined to be a constant value tuned such that it is low enough to prevent the motor from requiring excessive torque, but high enough that the sheet is inflated to not cause excess friction. The control loop for the pressure is as follows:

![](images/pressure_control_loop.png)

Note that the pressure inside the vessel $P$ would typically increase as the sheet inverts. Therefore, by regulating $P$ to be constant, it has the effect of decreasing the vacuum's voltage supply as the sheet gets closer to fully inverted.

Throughout the inversion process, the motor's torque is monitored to ensure that it remains within a typical operating range. If the torque exceeds a tuned threshold, it is a sign that the system is not inverting properly (e.g. buckling or something getting caught in between) and the system stops itself to prevent possible damage.