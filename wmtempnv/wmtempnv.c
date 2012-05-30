/*
 * wmtempnv: a sensor monitor for WindowMaker. this little app is mainly based
 * on wmsensormon and other simple dockapps.
 *
 * version = 0.4
 *
 * requirements: configured lm_sensors, sensor program and
 *               nvidia-settings package
 * licence: gpl
 */

#include <sys/param.h>
#include <sys/types.h>
#include "standards.h"
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <string.h>
#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"
#include "../wmgeneral/misc.h"
#include "wmtempnv_master2.xpm"
#include "wmtempnv_mask.xbm"

#include <NVCtrl/NVCtrl.h>
#include <NVCtrl/NVCtrlLib.h>

#define MAXSTRLEN 8
#define TEMP 40
#define TEMP_OVER 47
#define GPU_T 70
#define GPU_T_OVER 85
#define MAXFNAME 50

void display_values(int, int, int);
int get_temp(int core_number, Display*);
int get_offset(int temp, int cpu);
Display *display;

int main(int argc, char **argv){
    int temp1=0, temp2=0, temp3=0;
    /* offset is one of 0 (normal), 7 (alert), 14 (warning) */
    int offset1=0, offset2=0, offset3=0;
    int counter = 0;
    display = XOpenDisplay(NULL);

    openXwindow(argc, argv, wmtempnv_master2_xpm, wmtempnv_mask_bits, 
            wmtempnv_mask_width, wmtempnv_mask_height);

    while(TRUE){
        temp1 = get_temp(0, display);
        offset1 = get_offset(temp1, 1);
        temp2 = get_temp(1, display);
        offset2 = get_offset(temp2, 1);
        temp3 = get_temp(2, display);
        offset3 = get_offset(temp3, 0);

        // core 1
        copyXPMArea(0, 87 + offset1, 23, 7, 4, 7); // LCD: "CPU"
        copyXPMArea(69, 87 + offset1, 5, 7, 22, 7); // LCD: number of cpu
        copyXPMArea(66, 65 + offset1, 9, 7, 51, 7); // LCD: "°C"
        display_values(temp1, 0, offset1);

        // core 2
        copyXPMArea(0, 87 + offset2, 23, 7, 4, 21);
        copyXPMArea(75, 87 + offset2, 5, 7, 22, 21);
        copyXPMArea(66, 65 + offset2, 9, 7, 51, 21);
        display_values(temp2, 14, offset2);

        // gpu
        copyXPMArea(23, 87 + offset3, 23, 7, 4, 35);
        copyXPMArea(66, 65 + offset3, 9, 7, 51, 35);
        display_values(temp3, 28, offset3);
        RedrawWindow();
        counter--;
        sleep(1);
    }
}

int get_offset(int temp, int cpu){
    int alt, wrn;
    if(cpu == 1){
        wrn = TEMP;
        alt = TEMP_OVER;
    }else{
        wrn = GPU_T;
        alt = GPU_T_OVER;
    }
    if(temp >= alt){
        return 7;  // Alert
    }else if(temp >= wrn){
        return 14; // Warning
    }else{
        return 0;  // Normal
    }
}

void display_values(int temp, int offset, int offset2){
    char text[5], num1, num2, num3, num4;

    sprintf(text, "%03d", temp);
    num1 = (text[0] - '0');
    num2 = (text[1] - '0');
    num3 = (text[2] - '0');
    num4 = (text[3] - '0');
    if(num1)
        copyXPMArea(5 * num1, 65 + offset2, 5, 7, 31, 7 + offset);
    else
        copyXPMArea(60, 65 + offset2, 5, 7, 31, 7 + offset);
    copyXPMArea(5 * num2, 65 + offset2, 5, 7, 38, 7 + offset);
    copyXPMArea(5 * num3, 65 + offset2, 5, 7, 45, 7 + offset);
    copyXPMArea(5 * num4, 65 + offset2, 5, 7, 51, 7 + offset);
}

int get_temp(int core_number, Display *disp){
    // Core temperature. argument is core number. core no.2 is GPU
    FILE *file;
    char filename[MAXFNAME];
    int core_temp = 0;
	Bool res;

    if(core_number == 2){

		res = XNVCTRLQueryTargetAttribute(disp,
				NV_CTRL_TARGET_TYPE_GPU, 0, 0,
				NV_CTRL_GPU_CORE_TEMPERATURE, &core_temp);

		if (res == False) core_temp = 0;
    }else{
        snprintf(filename, MAXFNAME,
                "/sys/bus/platform/devices/coretemp.0/temp%d_input",
                core_number + 2);
        if((file = fopen(filename, "r")) != NULL){
            if(fscanf(file, "%d", &core_temp) != EOF){
                core_temp = core_temp / 1000;
                fclose(file);
            }
        }
    }
    return core_temp;
}

