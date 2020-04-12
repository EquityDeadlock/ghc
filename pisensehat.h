/** RPi Sensehat constants, structures, function prototypes
 * @version pisensehat.h 2020-01-15
 */
#ifndef PISENSEHAT_H
#define PISENSEHAT_H

// Includes
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <poll.h>
#include <dirent.h>
#include <linux/input.h>
#include <time.h>

// LPS25H Constants
#define LPS25HI2CADDRESS 0x5c
#define PRESS_OUT_XL 0x28
#define PRESS_OUT_L 0x29
#define PRESS_OUT_H 0x2A
//#define TEMP_OUT_L 0x2B
//#define TEMP_OUT_H 0x2C

// HTS221 Constants
#define HTS221I2CADDRESS 0x5F
#define HTS221DELAY 25000
#define WHO_AM_I 0x0F

#define CTRL_REG1 0x20
#define CTRL_REG2 0x21

#define T0_OUT_L 0x3C
#define T0_OUT_H 0x3D
#define T1_OUT_L 0x3E
#define T1_OUT_H 0x3F
#define T0_degC_x8 0x32
#define T1_degC_x8 0x33
#define T1_T0_MSB 0x35

#define TEMP_OUT_L 0x2A
#define TEMP_OUT_H 0x2B

#define H0_T0_OUT_L 0x36
#define H0_T0_OUT_H 0x37
#define H1_T0_OUT_L 0x3A
#define H1_T0_OUT_H 0x3B
#define H0_rH_x2 0x30
#define H1_rH_x2 0x31

#define H_T_OUT_L 0x28
#define H_T_OUT_H 0x29

// Sense Hat Frame Buffer Constants
#define FILEPATH "/dev/fb1"
#define NUM_WORDS 64
#define FILESIZE (NUM_WORDS * sizeof(uint16_t))

// RGB565 Color Masks
#define RGB565_RED      0xF800
#define RGB565_GREEN    0x07E0
#define RGB565_BLUE     0x001F

// Structures
typedef struct fbpixel
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} fbpixel_s;

typedef struct lps25hData
{
    double temperature;
    double pressure;
} lps25hData_s;

typedef struct ht221sData
{
    double temperature;
    double humidity;
} ht221sData_s;

// Function Prototypes
/// @cond INTERNAL
int ShInit(void);
int ShExit(void);
void ShClearMatrix(void);
uint8_t ShSetPixel(int x,int y,fbpixel_s px);
fbpixel_s ShGetPixel(int x,int y);
int ShSetVerticalBar(int bar,fbpixel_s px, uint8_t value);
double ShLPS25HGetPressure(void);
lps25hData_s ShGetLPS25HData(void);
ht221sData_s ShGetHT221SData(void);
/// @endcond
#endif
