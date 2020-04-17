/** Defines the entry point for the console application.
 * @version ghc.c 2020-03-12
 * @author Braydon Giallombardo
 */
#include "ghcontrol.h"
int main(void) {

	// Variables
	time_t now;
	int logged;
	reading_s creadings = {0};
	control_s ctrl = {0};
	setpoint_s sets = {0};
	setpoint_s spts = {0};
	alarmlimit_s alimits = {0};
	alarm_s * arecord;
	arecord = (alarm_s *) calloc(1,sizeof(alarm_s));
	if(arecord == NULL) {
		printf("\nCannot allocate memory\n");
		return 0;
	}



	// Initialization
	GhControllerInit();
	spts=GhSetSetpoints();
	alimits=GhSetAlarmLimits();
	// Loop
	while(1) {
		now = time(NULL);
		GhGetSetpoints();
		creadings=GhGetReadings();
		logged = GhLogData("ghdata.txt", creadings);
		ctrl=GhSetControls(spts, creadings);
		arecord=GhSetAlarms(arecord, alimits, creadings);
		GhDisplayAll(creadings, spts);
		GhDisplayReadings(creadings);
		GhDisplaySetpoints(spts);
		GhDisplayControls(ctrl);
		GhDisplayAlarms(arecord);
		GhDelay(GHUPDATE);
	}

	// Exit
	fprintf(stdout,"\n\nPress ENTER to continue...");
	getchar();
	return 1;
}
