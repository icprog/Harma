/*
 *
 */

#include <stdint.h>
#include "common.h"
#include "reports.h"
#include "LSM303DLHC.h"


uint8_t output_buffer[OUTPUT_BUFF_LEN];

/* ---------------- Define all reports that can be get / set ---------------- */

// Default report when we have an error
ret_t rpt_err(uint8_t * in_p, uint8_t in_len, uint8_t * out_p, uint8_t * out_len_ptr)
{
    out_p = 0;
    *out_len_ptr = 0;
    return RET_NORPT_ERR;
}

/* ------ LSM303DLHC REPORTS ------ */

// Report 0x20
ret_t rpt_LSM303DLHC_getTemp(uint8_t * in_p, uint8_t in_len, uint8_t * out_p, uint8_t * out_len_ptr)
{
    ret_t retval;
    int16_t temp_val;
    retval = LSM303DLHC_temp_getData(&temp_val);
    if (retval != RET_OK) { return retval; }

    *(int16_t *)out_p = temp_val;
    *out_len_ptr = sizeof(temp_val);
    return RET_OK;
}

// Report 0x21
ret_t rpt_LSM303DLHC_getAccel(uint8_t * in_p, uint8_t in_len, uint8_t * out_p, uint8_t * out_len_ptr)
{
    ret_t retval;
    accel_packet_t pkt;
    bool peak;

    // do some bounds checking
    if (in_len != 1) { return RET_INVALID_ARGS_ERR; }

    // check if we should peak or pop the packet
    if (in_p[0] == 0) {
        peak = false;
    } else {
        peak = true;
    }
    retval = LSM303DLHC_accel_getPacket(&pkt, peak);
    if (retval != RET_OK) { return retval; }

    // put the data onto the output buffer
    *(accel_packet_t *)out_p = pkt;
    *out_len_ptr = sizeof(accel_packet_t);
    return RET_OK;
}

// Report 0x22
ret_t rpt_LSM303DLHC_getMag(uint8_t * in_p, uint8_t in_len, uint8_t * out_p, uint8_t * out_len_ptr)
{
    ret_t retval;
    mag_packet_t pkt;
    bool peak;

    // do some bounds checking
    if (in_len != 1) { return RET_INVALID_ARGS_ERR; }

    // check if we should peak or pop the packet
    if (in_p[0] == 0) {
        peak = false;
    } else {
        peak = true;
    }
    retval = LSM303DLHC_mag_getPacket(&pkt, peak);
    if (retval != RET_OK) { return retval; }

    // put the data onto the output buffer
    *(mag_packet_t *)out_p = pkt;
    *out_len_ptr = sizeof(mag_packet_t);
    return RET_OK;
}


/* ------------------------- MASTER LOOKUP FUNCTION ------------------------- */


ret_t rpt_lookup(uint8_t rpt_type, uint8_t *input_buff_ptr, uint8_t input_buff_len,
                 uint8_t * output_buff_ptr, uint8_t * output_buff_len_ptr)
{
    // define the report lookup table
    static ret_t (*report_function_lookup[])(uint8_t *input_buff_ptr, uint8_t input_buff_len,
                                             uint8_t * output_buff_ptr, uint8_t * output_buff_len_ptr) = {
        /* Report 0x00 */ rpt_err,
        /* Report 0x01 */ rpt_err,
        /* Report 0x02 */ rpt_err,
        /* Report 0x03 */ rpt_err,
        /* Report 0x04 */ rpt_err,
        /* Report 0x05 */ rpt_err,
        /* Report 0x06 */ rpt_err,
        /* Report 0x07 */ rpt_err,
        /* Report 0x08 */ rpt_err,
        /* Report 0x09 */ rpt_err,
        /* Report 0x0a */ rpt_err,
        /* Report 0x0b */ rpt_err,
        /* Report 0x0c */ rpt_err,
        /* Report 0x0d */ rpt_err,
        /* Report 0x0e */ rpt_err,
        /* Report 0x0f */ rpt_err,
        /* Report 0x10 */ rpt_err,
        /* Report 0x11 */ rpt_err,
        /* Report 0x12 */ rpt_err,
        /* Report 0x13 */ rpt_err,
        /* Report 0x14 */ rpt_err,
        /* Report 0x15 */ rpt_err,
        /* Report 0x16 */ rpt_err,
        /* Report 0x17 */ rpt_err,
        /* Report 0x18 */ rpt_err,
        /* Report 0x19 */ rpt_err,
        /* Report 0x1a */ rpt_err,
        /* Report 0x1b */ rpt_err,
        /* Report 0x1c */ rpt_err,
        /* Report 0x1d */ rpt_err,
        /* Report 0x1e */ rpt_err,
        /* Report 0x1f */ rpt_err,
        /* Report 0x20 */ rpt_LSM303DLHC_getTemp,
        /* Report 0x21 */ rpt_LSM303DLHC_getAccel,
        /* Report 0x22 */ rpt_LSM303DLHC_getMag,
        /* Report 0x23 */ rpt_err,
        /* Report 0x24 */ rpt_err,
        /* Report 0x25 */ rpt_err,
        /* Report 0x26 */ rpt_err,
        /* Report 0x27 */ rpt_err,
        /* Report 0x28 */ rpt_err,
        /* Report 0x29 */ rpt_err,
        /* Report 0x2a */ rpt_err,
        /* Report 0x2b */ rpt_err,
        /* Report 0x2c */ rpt_err,
        /* Report 0x2d */ rpt_err,
        /* Report 0x2e */ rpt_err,
        /* Report 0x2f */ rpt_err,
        /* Report 0x30 */ rpt_err,
        /* Report 0x31 */ rpt_err,
        /* Report 0x32 */ rpt_err,
        /* Report 0x33 */ rpt_err,
        /* Report 0x34 */ rpt_err,
        /* Report 0x35 */ rpt_err,
        /* Report 0x36 */ rpt_err,
        /* Report 0x37 */ rpt_err,
        /* Report 0x38 */ rpt_err,
        /* Report 0x39 */ rpt_err,
        /* Report 0x3a */ rpt_err,
        /* Report 0x3b */ rpt_err,
        /* Report 0x3c */ rpt_err,
        /* Report 0x3d */ rpt_err,
        /* Report 0x3e */ rpt_err,
        /* Report 0x3f */ rpt_err,
        /* Report 0x40 */ rpt_err,
        /* Report 0x41 */ rpt_err,
        /* Report 0x42 */ rpt_err,
        /* Report 0x43 */ rpt_err,
        /* Report 0x44 */ rpt_err,
        /* Report 0x45 */ rpt_err,
        /* Report 0x46 */ rpt_err,
        /* Report 0x47 */ rpt_err,
        /* Report 0x48 */ rpt_err,
        /* Report 0x49 */ rpt_err,
        /* Report 0x4a */ rpt_err,
        /* Report 0x4b */ rpt_err,
        /* Report 0x4c */ rpt_err,
        /* Report 0x4d */ rpt_err,
        /* Report 0x4e */ rpt_err,
        /* Report 0x4f */ rpt_err,
        /* Report 0x50 */ rpt_err,
        /* Report 0x51 */ rpt_err,
        /* Report 0x52 */ rpt_err,
        /* Report 0x53 */ rpt_err,
        /* Report 0x54 */ rpt_err,
        /* Report 0x55 */ rpt_err,
        /* Report 0x56 */ rpt_err,
        /* Report 0x57 */ rpt_err,
        /* Report 0x58 */ rpt_err,
        /* Report 0x59 */ rpt_err,
        /* Report 0x5a */ rpt_err,
        /* Report 0x5b */ rpt_err,
        /* Report 0x5c */ rpt_err,
        /* Report 0x5d */ rpt_err,
        /* Report 0x5e */ rpt_err,
        /* Report 0x5f */ rpt_err,
        /* Report 0x60 */ rpt_err,
        /* Report 0x61 */ rpt_err,
        /* Report 0x62 */ rpt_err,
        /* Report 0x63 */ rpt_err,
        /* Report 0x64 */ rpt_err,
        /* Report 0x65 */ rpt_err,
        /* Report 0x66 */ rpt_err,
        /* Report 0x67 */ rpt_err,
        /* Report 0x68 */ rpt_err,
        /* Report 0x69 */ rpt_err,
        /* Report 0x6a */ rpt_err,
        /* Report 0x6b */ rpt_err,
        /* Report 0x6c */ rpt_err,
        /* Report 0x6d */ rpt_err,
        /* Report 0x6e */ rpt_err,
        /* Report 0x6f */ rpt_err,
        /* Report 0x70 */ rpt_err,
        /* Report 0x71 */ rpt_err,
        /* Report 0x72 */ rpt_err,
        /* Report 0x73 */ rpt_err,
        /* Report 0x74 */ rpt_err,
        /* Report 0x75 */ rpt_err,
        /* Report 0x76 */ rpt_err,
        /* Report 0x77 */ rpt_err,
        /* Report 0x78 */ rpt_err,
        /* Report 0x79 */ rpt_err,
        /* Report 0x7a */ rpt_err,
        /* Report 0x7b */ rpt_err,
        /* Report 0x7c */ rpt_err,
        /* Report 0x7d */ rpt_err,
        /* Report 0x7e */ rpt_err,
        /* Report 0x7f */ rpt_err,
        /* Report 0x80 */ rpt_err,
        /* Report 0x81 */ rpt_err,
        /* Report 0x82 */ rpt_err,
        /* Report 0x83 */ rpt_err,
        /* Report 0x84 */ rpt_err,
        /* Report 0x85 */ rpt_err,
        /* Report 0x86 */ rpt_err,
        /* Report 0x87 */ rpt_err,
        /* Report 0x88 */ rpt_err,
        /* Report 0x89 */ rpt_err,
        /* Report 0x8a */ rpt_err,
        /* Report 0x8b */ rpt_err,
        /* Report 0x8c */ rpt_err,
        /* Report 0x8d */ rpt_err,
        /* Report 0x8e */ rpt_err,
        /* Report 0x8f */ rpt_err,
        /* Report 0x90 */ rpt_err,
        /* Report 0x91 */ rpt_err,
        /* Report 0x92 */ rpt_err,
        /* Report 0x93 */ rpt_err,
        /* Report 0x94 */ rpt_err,
        /* Report 0x95 */ rpt_err,
        /* Report 0x96 */ rpt_err,
        /* Report 0x97 */ rpt_err,
        /* Report 0x98 */ rpt_err,
        /* Report 0x99 */ rpt_err,
        /* Report 0x9a */ rpt_err,
        /* Report 0x9b */ rpt_err,
        /* Report 0x9c */ rpt_err,
        /* Report 0x9d */ rpt_err,
        /* Report 0x9e */ rpt_err,
        /* Report 0x9f */ rpt_err,
        /* Report 0xa0 */ rpt_err,
        /* Report 0xa1 */ rpt_err,
        /* Report 0xa2 */ rpt_err,
        /* Report 0xa3 */ rpt_err,
        /* Report 0xa4 */ rpt_err,
        /* Report 0xa5 */ rpt_err,
        /* Report 0xa6 */ rpt_err,
        /* Report 0xa7 */ rpt_err,
        /* Report 0xa8 */ rpt_err,
        /* Report 0xa9 */ rpt_err,
        /* Report 0xaa */ rpt_err,
        /* Report 0xab */ rpt_err,
        /* Report 0xac */ rpt_err,
        /* Report 0xad */ rpt_err,
        /* Report 0xae */ rpt_err,
        /* Report 0xaf */ rpt_err,
        /* Report 0xb0 */ rpt_err,
        /* Report 0xb1 */ rpt_err,
        /* Report 0xb2 */ rpt_err,
        /* Report 0xb3 */ rpt_err,
        /* Report 0xb4 */ rpt_err,
        /* Report 0xb5 */ rpt_err,
        /* Report 0xb6 */ rpt_err,
        /* Report 0xb7 */ rpt_err,
        /* Report 0xb8 */ rpt_err,
        /* Report 0xb9 */ rpt_err,
        /* Report 0xba */ rpt_err,
        /* Report 0xbb */ rpt_err,
        /* Report 0xbc */ rpt_err,
        /* Report 0xbd */ rpt_err,
        /* Report 0xbe */ rpt_err,
        /* Report 0xbf */ rpt_err,
        /* Report 0xc0 */ rpt_err,
        /* Report 0xc1 */ rpt_err,
        /* Report 0xc2 */ rpt_err,
        /* Report 0xc3 */ rpt_err,
        /* Report 0xc4 */ rpt_err,
        /* Report 0xc5 */ rpt_err,
        /* Report 0xc6 */ rpt_err,
        /* Report 0xc7 */ rpt_err,
        /* Report 0xc8 */ rpt_err,
        /* Report 0xc9 */ rpt_err,
        /* Report 0xca */ rpt_err,
        /* Report 0xcb */ rpt_err,
        /* Report 0xcc */ rpt_err,
        /* Report 0xcd */ rpt_err,
        /* Report 0xce */ rpt_err,
        /* Report 0xcf */ rpt_err,
        /* Report 0xd0 */ rpt_err,
        /* Report 0xd1 */ rpt_err,
        /* Report 0xd2 */ rpt_err,
        /* Report 0xd3 */ rpt_err,
        /* Report 0xd4 */ rpt_err,
        /* Report 0xd5 */ rpt_err,
        /* Report 0xd6 */ rpt_err,
        /* Report 0xd7 */ rpt_err,
        /* Report 0xd8 */ rpt_err,
        /* Report 0xd9 */ rpt_err,
        /* Report 0xda */ rpt_err,
        /* Report 0xdb */ rpt_err,
        /* Report 0xdc */ rpt_err,
        /* Report 0xdd */ rpt_err,
        /* Report 0xde */ rpt_err,
        /* Report 0xdf */ rpt_err,
        /* Report 0xe0 */ rpt_err,
        /* Report 0xe1 */ rpt_err,
        /* Report 0xe2 */ rpt_err,
        /* Report 0xe3 */ rpt_err,
        /* Report 0xe4 */ rpt_err,
        /* Report 0xe5 */ rpt_err,
        /* Report 0xe6 */ rpt_err,
        /* Report 0xe7 */ rpt_err,
        /* Report 0xe8 */ rpt_err,
        /* Report 0xe9 */ rpt_err,
        /* Report 0xea */ rpt_err,
        /* Report 0xeb */ rpt_err,
        /* Report 0xec */ rpt_err,
        /* Report 0xed */ rpt_err,
        /* Report 0xee */ rpt_err,
        /* Report 0xef */ rpt_err,
        /* Report 0xf0 */ rpt_err,
        /* Report 0xf1 */ rpt_err,
        /* Report 0xf2 */ rpt_err,
        /* Report 0xf3 */ rpt_err,
        /* Report 0xf4 */ rpt_err,
        /* Report 0xf5 */ rpt_err,
        /* Report 0xf6 */ rpt_err,
        /* Report 0xf7 */ rpt_err,
        /* Report 0xf8 */ rpt_err,
        /* Report 0xf9 */ rpt_err,
        /* Report 0xfa */ rpt_err,
        /* Report 0xfb */ rpt_err,
        /* Report 0xfc */ rpt_err,
        /* Report 0xfd */ rpt_err,
        /* Report 0xfe */ rpt_err};

    // call the function specified
    output_buff_len_ptr = 0;
    output_buff_ptr = output_buffer;
    return report_function_lookup[rpt_type](input_buff_ptr, input_buff_len, output_buff_ptr, output_buff_len_ptr);
}
