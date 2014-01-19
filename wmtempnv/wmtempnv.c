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
#include "wmtempnv_master.xpm"
#include "wmtempnv_mask.xbm"

#include <NVCtrl/NVCtrl.h>
#include <NVCtrl/NVCtrlLib.h>

#define MAXSTRLEN 8
#define WARN_TEMP 65
#define CRIT_TEMP 80
#define WARN_TEMP_GPU 70
#define CRIT_TEMP_GPU 85
#define MAXFNAME 50


void display_values(int, int, int);
int get_temp(int core_number, Display*);
int get_offset(int temp, short cpu);
void display_help(char* progname);
int get_gpu_temp(char* path, Display *disp);
void read_file_into(char *filepath, int *output);
Display *display;

int main(int argc, char **argv){
    int core1_temp=0, core2_temp=0, core3_temp=0, core4_temp=0, gpu_temp=0;
    /* offset is one of 0 (normal), 7 (alert), 14 (warning) */
    int core1_offset=0, core2_offset=0, core3_offset=0, core4_offset=0,
        gpu_offset=0;
    int counter = 0;
    char* path = "";
    display = XOpenDisplay(NULL);

    if (argc > 1) {
        if (argc > 2) {
            display_help(argv[0]);
            exit(2);
        }
        
        if (argc == 2 &&
                (strcmp(argv[1], "--help") == 0 ||
                 strcmp(argv[1], "-h") == 0)) {
            display_help(argv[0]);
            exit(0);
        }

        path = argv[1];
    }

    openXwindow(argc, argv, wmtempnv_master, wmtempnv_mask_bits,
            wmtempnv_mask_width, wmtempnv_mask_height);

    while(TRUE){
        if (counter < 1){
            counter = 5;

            // cpu
            core1_temp = get_temp(0, display);
            core1_offset = get_offset(core1_temp, 1);
            core2_temp = get_temp(1, display);
            core2_offset = get_offset(core2_temp, 1);
            core3_temp = get_temp(2, display);
            core3_offset = get_offset(core3_temp, 1);
            core4_temp = get_temp(3, display);
            core4_offset = get_offset(core4_temp, 1);

            // gpu
            gpu_temp = get_gpu_temp(path, display);
            gpu_offset = get_offset(gpu_temp, 0);
        }

        // core 1
        copyXPMArea(0, 87 + core1_offset, 23, 7, 4, 7); // LCD: "CPU"
        copyXPMArea(5, 65 + core1_offset, 5, 7, 22, 7); // LCD: number of cpu
        copyXPMArea(66, 65 + core1_offset, 9, 7, 51, 7); // LCD: "°C"
        display_values(core1_temp, 0, core1_offset);

        // core 2
        copyXPMArea(0, 87 + core2_offset, 23, 7, 4, 16);
        copyXPMArea(10, 65 + core2_offset, 5, 7, 22, 16);
        copyXPMArea(66, 65 + core2_offset, 9, 7, 51, 16);
        display_values(core2_temp, 9, core2_offset);

        // core 3
        copyXPMArea(0, 87 + core3_offset, 23, 7, 4, 25);
        copyXPMArea(15, 65 + core3_offset, 5, 7, 22, 25);
        copyXPMArea(66, 65 + core3_offset, 9, 7, 51, 25);
        display_values(core3_temp, 18, core3_offset);

        // core 4
        copyXPMArea(0, 87 + core4_offset, 23, 7, 4, 34);
        copyXPMArea(20, 65 + core4_offset, 5, 7, 22, 34);
        copyXPMArea(66, 65 + core4_offset, 9, 7, 51, 34);
        display_values(core4_temp, 27, core4_offset);

        // gpu
        copyXPMArea(23, 87 + gpu_offset, 23, 7, 4, 49);
        copyXPMArea(66, 65 + gpu_offset , 9, 7, 51, 49);
        display_values(gpu_temp, 42, gpu_offset);
        RedrawWindow();
        counter--;
        usleep(100000);
    }
}

int get_offset(int temp, short cpu){
    int alt, wrn;
    if(cpu == 1){
        wrn = WARN_TEMP;
        alt = CRIT_TEMP;
    }else{
        wrn = WARN_TEMP_GPU;
        alt = CRIT_TEMP_GPU;
    }
    if(temp >= alt){
        return 7;  // Alert
    }else if(temp >= wrn){
        return 14; // Warning
    }else{
        return 0;  // Normal
    }
}

void display_values(int temp, int offset, int core2_offset){
    char text[5], num1, num2, num3, num4;

    sprintf(text, "%03d", temp);
    num1 = (text[0] - '0');
    num2 = (text[1] - '0');
    num3 = (text[2] - '0');
    num4 = (text[3] - '0');
    if(num1)
        copyXPMArea(5 * num1, 65 + core2_offset, 5, 7, 31, 7 + offset);
    else
        copyXPMArea(60, 65 + core2_offset, 5, 7, 31, 7 + offset);
    copyXPMArea(5 * num2, 65 + core2_offset, 5, 7, 38, 7 + offset);
    copyXPMArea(5 * num3, 65 + core2_offset, 5, 7, 45, 7 + offset);
    copyXPMArea(5 * num4, 65 + core2_offset, 5, 7, 51, 7 + offset);
}

int get_temp(int core_number, Display *disp){
    // Core temperature. argument is core number.
    char filename[MAXFNAME];
    int core_temp = 0;

    snprintf(filename, MAXFNAME,
            "/sys/bus/platform/devices/coretemp.0/temp%d_input",
            core_number + 2);
    read_file_into(filename, &core_temp);
    return core_temp;
}

int get_gpu_temp(char* path, Display *disp){
    // return GPU temperature. Argument is path in sysfs or empty string.
    int gpu_temp = 0;
	Bool res;

    if (strcmp(path, "") == 0){
		res = XNVCTRLQueryTargetAttribute(disp,
				NV_CTRL_TARGET_TYPE_GPU, 0, 0,
				NV_CTRL_GPU_CORE_TEMPERATURE, &gpu_temp);

		if (res == False) gpu_temp = 0;
    }else{
        read_file_into(path, &gpu_temp);
    }
    return gpu_temp;
}

void read_file_into(char *filepath, int *output) {
    // Read an integer from the provided filepath and write it in the address
    FILE *fp;
    if((fp = fopen(filepath, "r")) != NULL){
        if(fscanf(fp, "%d", output) != EOF){
            *output = *output / 1000;
            fclose(fp);
        }
    }
}

void display_help(char* progname){
    printf("Dockapp for monitoring CPU and Nvidia GPU temperatures.\n");
    printf("Usage:\n\t%s [full path for temp in sysfs]\n\n", progname);
    printf("As an optional parameter you can provide `temp_input' ");
    printf("full path from sysfs,\ncorresponding to your gfx card ");
    printf("to read temperature from. Otherwise\nfunctionality of ");
    printf("nv-control will be used (package nvidia-settings).\n");
}
