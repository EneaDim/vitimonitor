// Fixed-Point PID Algorithm
// Ported from: https://gist.github.com/bradley219/5373998
// Author: Li "oldrev" Wei <oldrev@gmail.com>

#include <stdio.h>             // Standard I/O (optional)
#include <stdint.h>            // Fixed-width integer types
#include <string.h>            // memset()

#include "fixedpoint-pid.h"    // Header file for FixedPid type and typedefs

// Define fixed-point Q format: Q16.16
#define FIXED32_Q        (16)                                   // Fractional bits
#define FIXED32_FMASK    (((Fixed32)1 << FIXED32_Q) - 1)        // Mask for fractional part

/**
 * @brief Convert integer to fixed-point representation
 */
Fixed32 Fixed32_FromInt(int32_t n) {
	return (Fixed32)((Fixed32)n << FIXED32_Q);
}

/**
 * @brief Extract fractional part of fixed-point number
 */
int32_t Fixed32_Frac(Fixed32 a){
	return a & FIXED32_FMASK;
}

/**
 * @brief Convert float to fixed-point with rounding
 */
Fixed32 Fixed32_FromFloat(float f) {
	return (Fixed32)((f) * (((Fixed64)1 << FIXED32_Q) + ((f) >= 0 ? 0.5 : -0.5)));
}

/**
 * @brief Convert fixed-point to float
 */
Fixed32 Fixed32_ToFloat(float T) {
	return (float)((T) * ((float)1 / (float)(1 << FIXED32_Q)));
}

/**
 * @brief Multiply two fixed-point numbers
 */
Fixed32 Fixed32_Mul(Fixed32 a, Fixed32 b) {
	return (Fixed32)(((Fixed64)a * (Fixed64)b) >> FIXED32_Q);
}

/**
 * @brief Divide two fixed-point numbers
 */
Fixed32 Fixed32_Div(Fixed32 a, Fixed32 b) {
	return (Fixed32)(((Fixed64)a << FIXED32_Q) / (Fixed64)b);
}

/**
 * @brief Initialize PID controller state to zero
 */
void FixedPid_Init(FixedPid* self) {
	memset(self, 0, sizeof(FixedPid));
}

/**
 * @brief Calculate PID output given setpoint and process variable
 *
 * @param self Pointer to PID controller state
 * @param setpoint Desired target value (fixed-point)
 * @param pv Measured process value (fixed-point)
 * @return PID output (fixed-point)
 */
Fixed32 FixedPid_Calculate(FixedPid* self, Fixed32 setpoint, Fixed32 pv) {
    // Compute error term: setpoint - process value
    Fixed32 error = setpoint - pv;

    // Proportional term: Kp * error
    Fixed32 Pout = Fixed32_Mul(self->Kp, error);

    // Integral term: accumulate error * dt
    self->Integral += Fixed32_Mul(error, self->Dt);
    Fixed32 Iout = Fixed32_Mul(self->Ki, self->Integral);

    // Derivative term: (error - previous error) / dt
    Fixed32 derivative = Fixed32_Div(error - self->PrevError, self->Dt);
    Fixed32 Dout = Fixed32_Mul(self->Kd, derivative);

    // Compute total output
    Fixed32 output = Pout + Iout + Dout;

    // Clamp output to min/max
    if (output > self->Max) {
        output = self->Max;
    } else if (output < self->Min) {
        output = self->Min;
    }

    // Save current error for next iteration
    self->PrevError = error;

    return output;
}

