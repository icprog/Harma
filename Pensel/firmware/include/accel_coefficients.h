
#ifndef _ACCEL_COEFFICIENTS_H_
#define _ACCEL_COEFFICIENTS_H_

#define FIR_ACCEL_GRAVITY_ORDER (16)
#define FIR_ACCEL_MOVEMENT_ORDER (15)

// Low pass filter for gravity vector detection
const float accel_coefficients_LPF[FIR_ACCEL_GRAVITY_ORDER] = {-0.00240944875944f,
    -0.00416217525112f, 0.009536485428f, 0.0199709259953f,
    -0.0379541806908f, -0.0695728329288f, 0.137360839193f, 0.447230387014f,
    0.447230387014f, 0.137360839193f, -0.0695728329288f, -0.0379541806908f,
    0.0199709259953f, 0.009536485428f, -0.00416217525112f, -0.00240944875944f};

// band pass filter for movement detection
const float accel_coefficients_BPF[FIR_ACCEL_MOVEMENT_ORDER] = {-0.0047846698549f,
    4.54680087369e-19f, 0.0168247546099f, 0.0427336721913f,
    0.0456754563004f, 0.0f, -0.0701556742885f, 0.939412922083f,
    -0.0701556742885f, 0.0f, 0.0456754563004f, 0.0427336721913f,
    0.0168247546099f, 4.54680087369e-19f, -0.0047846698549f};

#endif /* _ACCEL_COEFFICIENTS_H_ */
