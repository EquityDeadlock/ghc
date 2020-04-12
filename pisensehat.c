/** RPi Sensehat functions
 * @version pisensehat.c 2020-01-15
 */

#include "pisensehat.h"

static int fbfd;        // Frame buffer file handle;
static uint16_t *map;   // Frame buffer memory map pointer;
static int HTS221fd;    // HTS221 Sensor file handle;
static int LPS25Hfd;    // LPS25Hfd Sensor file handle;

/** Initialize Sensehat
 * @author Paul Moggach
 * @author Kristian Medri
 * @version 2020-01-14
 * @param void
 * @return status an int, 1 if successful
 */
int ShInit(void)
{
    int status = 1;
    struct fb_fix_screeninfo fix_info;

    // Frame Buffer Initialization for 8X8 LED Matrix
    /* open the led frame buffer device */
    fbfd = open(FILEPATH, O_RDWR);
    if (fbfd == -1)
    {
        perror("Error (call to 'open')");
        exit(EXIT_FAILURE);
    }

    /* read fixed screen info for the open device */
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fix_info) == -1)
    {
        perror("Error (call to 'ioctl')");
        close(fbfd);
        exit(EXIT_FAILURE);
    }

    /* now check the correct device has been found */
    if (strcmp(fix_info.id, "RPi-Sense FB") != 0)
    {
        printf("%s\n", "Error: RPi-Sense FB not found");
        close(fbfd);
        exit(EXIT_FAILURE);
    }

    /* map the led frame buffer device into memory */
    map = mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
    if (map == MAP_FAILED)
    {
        close(fbfd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

    // Sensor Initialization
	HTS221fd = wiringPiI2CSetup(HTS221I2CADDRESS);
	LPS25Hfd = wiringPiI2CSetup(LPS25HI2CADDRESS);

    // Power down the device (clean start)
    wiringPiI2CWriteReg8(HTS221fd, CTRL_REG1, 0x00);
    wiringPiI2CWriteReg8(LPS25Hfd, CTRL_REG1, 0x00);
    return status;
}

/** Closes Down the Sensehat
 * @author Paul Moggach
 * @author Kristian Medri
 * @version 2020-01-14
 * @param void
 * @return status an int, 1 if successful
 */
int ShExit(void)
{
    int status = 1;
    ShClearMatrix();
    /* un-map and close */
    if (munmap(map, FILESIZE) == -1)
    {
        perror("Error un-mmapping the file");
	status=0;
        return status;
    }
    close(fbfd);
    close(HTS221fd);
    close(LPS25Hfd);
    return status;
}

/** Clears Sensehat 8X8 RGB LED display
 * @author Paul Moggach
 * @author Kristian Medri
 * @version 2020-01-14
 * @param void
 * @return void
 */
void ShClearMatrix(void)
{
    memset(map, 0, FILESIZE);
}

/** Sets a pixel on the Sensehat display
 * @author Paul Moggach
 * @author Kristian Medri
 * @version 2020-01-14
 * @param x an integer position value
 * @param y an integer position value
 * @param fbpixel_s pixel colour data
 * @return uint8_t 1 if successful
 */
uint8_t ShSetPixel(int x,int y,fbpixel_s px)
{
    int i;

	if (x >= 0 && x < 8 && y >= 0 && y < 8)
	{
        i = (y*8)+x; // offset into array
        map[i] = (px.red << 11) | (px.green << 5) | (px.blue);
		return 1;
	}
	return 0;
}

/** Gets pixel information from the Sensehat display
 * @author Paul Moggach
 * @author Kristian Medri
 * @version 2020-01-14
 * @param x an integer position value
 * @param y an integer position value
 * @return cpx fbpixel_s pixel colour data
 */
fbpixel_s ShGetPixel(int x,int y)
{
    fbpixel_s cpx = {0};
    int i;

	if (x >= 0 && x < 8 && y >= 0 && y < 8)
	{
        i = (y*8)+x; // offset into array
        cpx.red = (map[i] & RGB565_RED) >> 11;
        cpx.green = (map[i] & RGB565_GREEN) >> 5;
        cpx.blue = (map[i] & RGB565_BLUE);
	}
	return cpx;
}

/** Sets a vertical bar on the Sensehat display
 * @author Paul Moggach
 * @author Kristian Medri
 * @version 2020-01-14
 * @param int bar to light
 * @param fbpixel_s pixel colour data
 * @param uint8_t value how many pixels to light in bar
 * @return int 1 if successful
 */
int ShSetVerticalBar(int bar,fbpixel_s px, uint8_t value)
{
    int i;

	if (bar >= 0 && bar < 8 && value >= 0 && value < 8)
	{
        for(i=0; i< value; i++)
        {
            ShSetPixel(bar,i,px);
        }
        px.red = 0x00;
        px.green = 0x00;
        px.blue = 0x00;
        for(i=value; i< 8;i++)
        {
            ShSetPixel(bar,i,px);
        }
		return 1;
	}
	return 0;
}

/** Gets LPS25H Sensehat sensor information
 * @author Paul Moggach
 * @author Kristian Medri
 * @version 2020-01-14
 * @param void
 * @return lps25hData_s pressure and temperature data
 */
lps25hData_s ShGetLPS25HData(void)
{
    lps25hData_s rd = {0};

    uint8_t temp_out_l = 0, temp_out_h = 0;
    int16_t temp_out = 0;
    uint8_t press_out_xl = 0;
    uint8_t press_out_l = 0;
    uint8_t press_out_h = 0;
    int32_t press_out = 0;
    uint8_t status = 0;

	// Power down the device (clean start)
    wiringPiI2CWriteReg8(LPS25Hfd, CTRL_REG1, 0x00);

    // Turn on the humidity sensor analog front end in single shot mode
    wiringPiI2CWriteReg8(LPS25Hfd, CTRL_REG1, 0x84);

    // Run one-shot measurement (temperature and humidity). The set bit will be reset by the
    // sensor itself after execution (self-clearing bit)
    wiringPiI2CWriteReg8(LPS25Hfd, CTRL_REG2, 0x01);

    // Wait until the measurement is completed
    do
	{
		usleep(HTS221DELAY);	// 25 ms
		status = wiringPiI2CReadReg8(LPS25Hfd, CTRL_REG2);
    }
    while (status != 0);

    /* Read the temperature measurement (2 bytes to read) */
    temp_out_l = wiringPiI2CReadReg8(LPS25Hfd, TEMP_OUT_L);
    temp_out_h = wiringPiI2CReadReg8(LPS25Hfd, TEMP_OUT_H);

    /* Read the pressure measurement (3 bytes to read) */
    press_out_xl = wiringPiI2CReadReg8(LPS25Hfd, PRESS_OUT_XL);
    press_out_l = wiringPiI2CReadReg8(LPS25Hfd, PRESS_OUT_L);
    press_out_h = wiringPiI2CReadReg8(LPS25Hfd, PRESS_OUT_H);

    /* make 16 and 24 bit values (using bit shift) */
    temp_out = temp_out_h << 8 | temp_out_l;
    press_out = press_out_h << 16 | press_out_l << 8 | press_out_xl;

    /* calculate output values */
    rd.temperature = 42.5 + (temp_out / 480.0);
    rd.pressure = press_out / 4096.0;

	// Power down the device
    wiringPiI2CWriteReg8(LPS25Hfd, CTRL_REG1, 0x00);

    return rd;
}

/** Gets HT221S Sensehat sensor data
 * @author Paul Moggach
 * @author Kristian Medri
 * @version 2020-01-14
 * @param void
 * @return ht221sData_s temperature and humidity data
 */
ht221sData_s ShGetHT221SData(void)
{
    ht221sData_s rd = {0};
    int status;
	uint8_t t0_out_l,t0_out_h,t1_out_l,t1_out_h;
	uint8_t t0_degC_x8,t1_degC_x8,t1_t0_msb;
	int16_t T0_OUT,T1_OUT;
	uint16_t T0_DegC_x8,T1_DegC_x8;
	double T0_DegC,T1_DegC;
	double t_gradient_m,t_intercept_c;
	uint8_t t_out_l,t_out_h;
	int16_t T_OUT;
	uint8_t h0_out_l,h0_out_h,h1_out_l,h1_out_h,h0_rh_x2,h1_rh_x2,h_t_out_l,h_t_out_h;
	int16_t H0_T0_OUT,H1_T0_OUT,H_T_OUT;
	double H0_rH,H1_rH,h_gradient_m,h_intercept_c;

	// Power down the device (clean start)
    wiringPiI2CWriteReg8(HTS221fd, CTRL_REG1, 0x00);
    // Turn on the humidity sensor analog front end in single shot mode
    wiringPiI2CWriteReg8(HTS221fd, CTRL_REG1, 0x84);
    // Run one-shot measurement (temperature and humidity). The set bit will be reset by the
    // sensor itself after execution (self-clearing bit)
    wiringPiI2CWriteReg8(HTS221fd, CTRL_REG2, 0x01);

    // Wait until the measurement is completed
    do
	{
		usleep(HTS221DELAY);	// 25 ms
		status = wiringPiI2CReadReg8(HTS221fd, CTRL_REG2);
    }
    while (status != 0);

    // Read calibration temperature LSB (ADC) data
    // (temperature calibration x-data for two points)
    t0_out_l = wiringPiI2CReadReg8(HTS221fd, T0_OUT_L);
    t0_out_h = wiringPiI2CReadReg8(HTS221fd, T0_OUT_H);
    t1_out_l = wiringPiI2CReadReg8(HTS221fd, T1_OUT_L);
    t1_out_h = wiringPiI2CReadReg8(HTS221fd, T1_OUT_H);

   // Read calibration relative humidity LSB (ADC) data
    // (humidity calibration x-data for two points)
    h0_out_l = wiringPiI2CReadReg8(HTS221fd, H0_T0_OUT_L);
    h0_out_h = wiringPiI2CReadReg8(HTS221fd, H0_T0_OUT_H);
    h1_out_l = wiringPiI2CReadReg8(HTS221fd, H1_T0_OUT_L);
    h1_out_h = wiringPiI2CReadReg8(HTS221fd, H1_T0_OUT_H);

    // Read calibration temperature (°C) data
    // (temperature calibration y-data for two points)
    t0_degC_x8 = wiringPiI2CReadReg8(HTS221fd, T0_degC_x8);
    t1_degC_x8 = wiringPiI2CReadReg8(HTS221fd, T1_degC_x8);
    t1_t0_msb = wiringPiI2CReadReg8(HTS221fd, T1_T0_MSB);

   // Read relative humidity (% rH) data
    // (humidity calibration y-data for two points)
    h0_rh_x2 = wiringPiI2CReadReg8(HTS221fd, H0_rH_x2);
    h1_rh_x2 = wiringPiI2CReadReg8(HTS221fd, H1_rH_x2);

    // make 16 bit values (bit shift)
    // (temperature calibration x-values)
    T0_OUT = t0_out_h << 8 | t0_out_l;
    T1_OUT = t1_out_h << 8 | t1_out_l;

    // make 16 and 10 bit values (bit mask and bit shift)
    T0_DegC_x8 = (t1_t0_msb & 3) << 8 | t0_degC_x8;
    T1_DegC_x8 = ((t1_t0_msb & 12) >> 2) << 8 | t1_degC_x8;

    // Calculate calibration values
    // (temperature calibration y-values)
    T0_DegC = T0_DegC_x8 / 8.0;
    T1_DegC = T1_DegC_x8 / 8.0;

	// Solve the linear equasions 'y = mx + c' to give the
    // calibration straight line graphs for temperature and humidity
    t_gradient_m = (T1_DegC - T0_DegC) / (T1_OUT - T0_OUT);
    t_intercept_c = T1_DegC - (t_gradient_m * T1_OUT);

	// Read the ambient temperature measurement (2 bytes to read)
    t_out_l = wiringPiI2CReadReg8(HTS221fd, TEMP_OUT_L);
    t_out_h = wiringPiI2CReadReg8(HTS221fd, TEMP_OUT_H);

    // make 16 bit value
    T_OUT = t_out_h << 8 | t_out_l;

    // make 16 bit values (bit shift)
    // (humidity calibration x-values)
    H0_T0_OUT = h0_out_h << 8 | h0_out_l;
    H1_T0_OUT = h1_out_h << 8 | h1_out_l;

    // Humidity calibration values
    // (humidity calibration y-values)
    H0_rH = h0_rh_x2 / 2.0;
    H1_rH = h1_rh_x2 / 2.0;
    h_gradient_m = (H1_rH - H0_rH) / (H1_T0_OUT - H0_T0_OUT);
    h_intercept_c = H1_rH - (h_gradient_m * H1_T0_OUT);

    // Read the ambient humidity measurement (2 bytes to read)
    h_t_out_l = wiringPiI2CReadReg8(HTS221fd, H_T_OUT_L);
    h_t_out_h = wiringPiI2CReadReg8(HTS221fd, H_T_OUT_H);

    // make 16 bit value
    H_T_OUT = h_t_out_h << 8 | h_t_out_l;

	// Power down the device
    wiringPiI2CWriteReg8(HTS221fd, CTRL_REG1, 0x00);

	// Calculate and return ambient temperature
    rd.temperature = (t_gradient_m * T_OUT) + t_intercept_c;
    rd.humidity = (h_gradient_m * H_T_OUT) + h_intercept_c;
    return rd;
}
