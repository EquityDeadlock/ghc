#include "ghcontrol.h"

//Constants
const char alarmnames[NALARMS][ALARMNMSZ] = {"No Alarms","High Temperature","Low Temperature","High Humidity","Low Humidity","High Pressure","Low Pressure"};


// Setup #######################################################################

/** Initializes srand, GhDisplayHeaders
 * @version 2020-03-12
 * @author Braydon Giallombardo
 */
void GhControllerInit(void) {
	srand((unsigned) time(NULL));
	GhDisplayHeader("Braydon Giallombardo");
	#if SENSEHAT
        wiringPiSetup();
        ShInit();
    #endif
}

/** Random number generator from range
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @param upperBound integer for upper bound for the range
 * @param lowerBound integer for lower bound for the range
 * @return random integer from range
 */
int GhGetRandom(int upperBound, int lowerBound) {
	return rand() % (upperBound-lowerBound) + lowerBound;
}

/** Induces a delay in milliseconds
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @param milliseconds integer that represents the delay time in milliseconds
 */
void GhDelay(int milliseconds) {
	// Variables
	long wait;
	clock_t now, start;

	wait = milliseconds * (CLOCKS_PER_SEC/1000);
	start = clock();
	now = start;

	while( (now-start) < wait) {
		now = clock();
	}
}

// Displays #######################################################################

/** Prints Greenhouse Controller header
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @param sname string with operator's name
 */
void GhDisplayHeader(const char * sname) {
	fprintf(stdout,"\n%s's CENG153 Greenhouse Controller\n", sname);
}

/** Gets current sensor readings
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @param rdata object of the structure readings named rdata
 */
void GhDisplayReadings(reading_s rdata) {
	fprintf(stdout,"\n%sReadings	 T: %3.1lfC    H: %.0lf%%  P: %5.1lfmb", ctime(&rdata.rtime), rdata.temperature, rdata.humidity, rdata.pressure);
}

/** Displays current setpoints
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @param spts data object of the structure setpoints named spts
 */
void GhDisplaySetpoints(setpoint_s spts){
	fprintf(stdout, "\nSetpoints	sT: %.1lf C	H: %.0lf%%", spts.temperature, spts.humidity);
}

/** Displays environment control status
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @param ctrl object of the structure controls named ctrl
 */
void GhDisplayControls(control_s ctrl) {
	fprintf(stdout, "\nControls	Heater: %d Humidifier: %d\n", ctrl.heater, ctrl.humidifier);
}

/** Displays Readings and Setpoints to the Sense Hat 8x8 display
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @param rd object of the structure readings named rd
 * @param sd object of the structure setpoints named sd
 */
void GhDisplayAll(reading_s rd, setpoint_s sd) {
    int rv,sv,avh,avl;
    fbpixel_s pxc = {0};
    ShClearMatrix();

    pxc.red = 0x00; pxc.green = 0xFF; pxc.blue = 0x00;
    rv = (8.0 * ((rd.temperature / (USTEMP-LSTEMP))+0.05))-1.0;
    ShSetVerticalBar(TBAR, pxc, rv);

    pxc.red = 0x0F; pxc.green = 0x0F; pxc.blue = 0x00;
    sv = (8.0 * ((sd.temperature / (USTEMP-LSTEMP))+0.05))-1.0;
    ShSetPixel(TBAR, sv, pxc);

    pxc.red = 0x00; pxc.green = 0xFF; pxc.blue = 0x00;
    rv = (8.0 * ((rd.humidity / (USHUMID-LSHUMID))+0.05))-1.0;
    ShSetVerticalBar(HBAR, pxc, rv);

    pxc.red = 0x0F; pxc.green = 0x0F; pxc.blue = 0x00;
    sv = (8.0 * ((sd.humidity / (USHUMID-LSHUMID))+0.05))-1.0;
    ShSetPixel(HBAR, sv, pxc);

    pxc.red = 0x00; pxc.green = 0xFF; pxc.blue = 0x00;
    rv = (8.0 * ((rd.pressure / (USPRESS-LSPRESS))+0.05))-1.0;
    ShSetVerticalBar(PBAR, pxc, rv);
}

/* Display current state of alarms
 * @version 2020-03-06
 * @author Braydon Giallombardo
 * @param alarm Object of alarm_s
 */
void GhDisplayAlarms(alarm_s * head) {
	alarm_s * cur;
	cur = head;
	fprintf(stdout, "Alarms\n");
	while (cur = cur->next) {
        /*if (cur->code != NOALARM) {*/
           fprintf(stdout,"%s%s", cur->code, ctime(&cur->atime));

	}
}


// Sets ##########################################################################

/** Controls heater and humidifier operation by comparing setpoints and sensor readings
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @param target object of the structure setpoints named target
 * @param rdata object of the structure readings named rdata
 * @return cset - object of controls that determines the toggle of the heater and humififier
 */
control_s GhSetControls(setpoint_s target, reading_s rdata) {
	control_s cset;
	if (rdata.temperature < target.temperature) {
		cset.heater = ON;
	}
	else {
		cset.heater = OFF;
	}
	if (rdata.humidity < target.humidity) {
		cset.humidifier = ON;
	}
	else {
		cset.humidifier = OFF;
	}
	return cset;
}

/** Sets the setpoints using setpoints.dat or creates setpoints.dat if error
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @return cpoints - object of setpoints containing the target values for setpoints
 */
setpoint_s GhSetSetpoints(void) {
 	setpoint_s cpoints = {0};
 	cpoints = GhRetrieveSetpoints("setpoints.dat");

 	if (cpoints.temperature == 0) {
 		cpoints.temperature = STEMP;
 		cpoints.humidity = SHUMID;
 		GhSaveSetpoints("setpoints.dat", cpoints);
 	}
 	return cpoints;
}

/** Sets alarm limit values
 * @version 20-04-02
 * @author Braydon Giallombardo
 * @return alarmlimit_s - object of alarmlimit_s
 */
alarmlimit_s GhSetAlarmLimits(void) {
	alarmlimit_s calarm;

	calarm.hight = UPPERATEMP;
	calarm.lowt = LOWERATEMP;
	calarm.highh = UPPERAHUMID;
	calarm.lowh = LOWERAHUMID;
	calarm.highp = UPPERAPRESS;
	calarm.lowp = LOWERAPRESS;
	return calarm;
}

/** Sets alarms
 * @version 2020-04-06
 * @author Braydon Giallombardo
 * @param calarm[NALARM]
 * @param alarmpt
 * @param rdata
 */
alarm_s * GhSetAlarms(alarm_s * head, alarmlimit_s alarmpt, reading_s rdata) {
	for(int i=0; i<NALARMS;i++) {
        head->code = NOALARM;
	}
	if (rdata.temperature >= alarmpt.hight) {
		GhSetOneAlarm(HTEMP,rdata.rtime,rdata.temperature,head);
	}
    else {
		head = GhClearOneAlarm(HTEMP, head);
	}
	if (rdata.humidity >= alarmpt.highh) {
		GhSetOneAlarm(HHUMID,rdata.rtime,rdata.humidity,head);
	}
    else {
		head = GhClearOneAlarm(HHUMID, head);
	}
    if (rdata.pressure >= alarmpt.highp) {
		GhSetOneAlarm(HPRESS,rdata.rtime,rdata.pressure,head);
    }
    else {
		head = GhClearOneAlarm(HPRESS, head);
    }
    return head;
}

/** Gets current temperature
 * @version 2020-04-12
 * @author Braydon Giallombardo
 * @return GhGetRandom of LSTEMP to USTEMP, or 25.0
 */
int GhSetOneAlarm(alarm_e code, time_t atime, double value, alarm_s * head) {
	alarm_s * last;
	alarm_s * cur;
	cur = head;
	if(cur->code != 0) {
		while(cur !=  NULL)	{
            if(cur->code == code)	{
                return 0;
            }
            last = cur;
            cur = cur->next;
		}
		cur = (alarm_s *)calloc(1, sizeof(alarm_s));
		last->next = cur;
		if(cur == NULL) {
            return 0;
		}
	}
	cur->code = code;
	cur->atime = atime;
	cur->value = value;
	cur->next = NULL;
	return 1;
}

/** Gets current temperature
 * @version 2020-04-12
 * @author Braydon Giallombardo
 * @return GhGetRandom of LSTEMP to USTEMP, or 25.0
 */
alarm_s * GhClearOneAlarm(alarm_e code, alarm_s * head) {
    alarm_s * cur;
    alarm_s * last;
    cur = head;
    last = head;

  if(cur->code == code && cur->next == NULL) {
      cur->code = NOALARM;
      return head;
  }

  if(cur-> code == code && cur->next != NULL)
  {
      head = cur->next;
      free(cur);
      return head;
  }

  while(cur != NULL)
  {
      if(cur->code == code)
      {
          last->next = cur->next;
          free(cur);
      }
      last = cur;
      cur = cur->next;
  }
  return head;
}

// Gets ##########################################################################

/** Gets current temperature
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @return GhGetRandom of LSTEMP to USTEMP, or 25.0
 */
double GhGetTemperature(void) {
	#if SIMTEMPERATURE
		return GhGetRandom(USTEMP, LSTEMP);
	#else
        ht221sData_s ct = {0};
        ct = ShGetHT221SData();
		return ct.temperature;
	#endif
}

/** Gets current humidity
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @return GhGetRandom of LSHUMID to USHUMID, or 55.0
 */
double GhGetHumidity(void) {
	#if SIMHUMIDITY
		return GhGetRandom(USHUMID, LSHUMID);
	#else
        ht221sData_s ct = {0};
        ct = ShGetHT221SData();
		return ct.humidity;
	#endif
}

/** Gets current pressure
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @return GhGetRandom of LSPRESS to USPRESS, or 1013.0
 */
double GhGetPressure(void) {
	#if SIMPRESSURE
		return GhGetRandom(USPRESS, LSPRESS);
	#else
        lps25hData_s ct = {0};
        ct = ShGetLPS25HData();
		return ct.pressure;
	#endif
}

/** GhGetControls Placeholder
 * @version 2020-03-12
 * @author Braydon Giallombardo
 */
void GhGetControls(void) {}

/** GhGetSetpoints Placeholder
 * @version 2020-03-12
 * @author Braydon Giallombardo
 */
void GhGetSetpoints(void) {}

/** Displays current sensor readings
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @return now - object of readings containing the values of each reading
 */
reading_s GhGetReadings(void) {
	reading_s now = {0};
	now.rtime = time(NULL);
	now.temperature = GhGetTemperature();
	now.humidity = GhGetHumidity();
	now.pressure = GhGetPressure();
	return now;
}

// Data Logs ##########################################################################

/** Log of data from reading object "ghdata"
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @param *fname Name of the file
 * @param ghdata object of the structure readings named ghdata
 * @return if error opening: 0, else: 1
 */
int GhLogData(char * fname, reading_s ghdata) {
	FILE * fp;
	char ltime[25];
	fp = fopen(fname, "a");

	if (fp == NULL) {
		fprintf(stdout,"\nCan't open file, data not retrieved!\n");
		return 0;
	}
	else {
		// Set characters in array
		strcpy(ltime, ctime(&ghdata.rtime));
		ltime[3] = ',';
		ltime[7] = ',';
		ltime[10] = ',';
		ltime[19] = ',';
		// Write to fp stream
		fprintf(fp, "%.24s,%3.1lf,%3.1lf,%5.1lf\n", ltime, ghdata.temperature, ghdata.humidity, ghdata.pressure);
		fclose(fp);
		return 1;
	}
}

/** Save setpoint data as binary
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @param *fname Name of the file
 * @param spts object of the structure setpoints named spts
 * @return if error opening: 0, else: 1
 */
int GhSaveSetpoints(char * fname, setpoint_s spts){
	FILE * fp;
	fp = fopen(fname, "w");

  if (fp == NULL) {
  	fprintf(stdout,"\nCan't open file, data not retrieved!\n");
  	return 0;
  }
	else {
		fwrite(&spts, sizeof(setpoint_s), 1, fp);
		fclose(fp);
		return 1;
	}
}

/** Retrieves saved setpoints from file
 * @version 2020-03-12
 * @author Braydon Giallombardo
 * @param *fname Name of the file
 * @return if error opening: 0, else: 1
 */
setpoint_s GhRetrieveSetpoints(char * fname){
	setpoint_s spts = {0};
	FILE * fp;
	fp = fopen(fname, "r");

	if (fp == NULL) {
		fprintf(stdout, "\nCan't open file, data not retrieved!\n");
		return spts;
  }
	else {
		fread(&spts, sizeof(setpoint_s), 1, fp);
		fclose(fp);
		return spts;
	}
}
