
#ifndef _MAG_COEFFICIENTS_H_
#define _MAG_COEFFICIENTS_H_


#define FIR_MAG_NORTH_ORDER (16)

// Low pass filter for North vector detection
const float mag_coefficients_LPF[FIR_MAG_NORTH_ORDER] = {-0.00240944875944f,
    -0.00416217525112f, 0.009536485428f, 0.0199709259953f,
    -0.0379541806908f, -0.0695728329288f, 0.137360839193f, 0.447230387014f,
    0.447230387014f, 0.137360839193f, -0.0695728329288f, -0.0379541806908f,
    0.0199709259953f, 0.009536485428f, -0.00416217525112f, -0.00240944875944f};

#endif /* _MAG_COEFFICIENTS_H_ */
