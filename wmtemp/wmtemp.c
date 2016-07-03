/*
 * wmtemp: a temperature monitor for WindowMaker. This little app is mainly
 * based on wmsensormon and other simple dockapps, although it doesn't use 
 * lmsensors, just provided information from kernel via /sys filesystem.
 *
 * version = 0.5
 *
 * licence: GPL2
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include <sys/param.h>
#include <sys/types.h>

#include "standards.h"
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"
#include "../wmgeneral/misc.h"
#include "wmtemp_master.xpm"
#include "wmtemp_mask.xbm"

#define MAXLEN 200
#define MAXFNAME 70
#define MAXCORENUM 4

struct entry {
    char *path;
    long critical;
    long warning;
};

struct config {
    struct entry cpu1;
    struct entry cpu2;
    struct entry cpu3;
    struct entry cpu4;
    struct entry gpu;
};

char cpu1_path[100];
int cpu1_warn = 0;
int cpu1_crit = 0;
char cpu2_path[100];
int cpu2_warn = 0;
int cpu2_crit = 0;
char cpu3_path[100];
int cpu3_warn = 0;
int cpu3_crit = 0;
char cpu4_path[100];
int cpu4_warn = 0;
int cpu4_crit = 0;
char gpu1_path[100];
int gpu1_warn = 0;
int gpu1_crit = 0;

typedef struct _core {
    int temp;
    /* offset is one of 0 (normal), 7 (critical), 14 (warning) */
    short offset;
} core;

Display *display;

char *vcopy(char *str);
int get_gpu_temp(char *path, Display *disp);
int get_temp(struct entry *etr);
short get_offset(short temp, struct entry *etr);
void conf_read(char *filename);
void display_help(char *progname);
void display_values(int, short, short);
void draw_cpu_temp(short core_no, core *cpu);
void draw_gpu_temp(core *gpu);
void read_file_into(char *filepath, int *output);
void set_defaults(struct config *conf);
char *strip(char * string);
void parse_config(struct config *conf);

int main(int argc, char **argv){
    int i;
    short counter = 0;
    struct config conf;

    set_defaults(&conf);

    parse_config(&conf);

    display = XOpenDisplay(NULL);
    core gpu, *cpus = malloc(MAXCORENUM * sizeof (core));

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
    }

    openXwindow(argc, argv, wmtemp_master, wmtemp_mask_bits,
            wmtemp_mask_width, wmtemp_mask_height);

    while(TRUE){
        if (counter < 1){
            counter = 5;
            cpus[0].temp = get_temp(&conf.cpu1);
            cpus[0].offset = get_offset(cpus[0].temp, &conf.cpu1);
            cpus[1].temp = get_temp(&conf.cpu2);
            cpus[1].offset = get_offset(cpus[1].temp, &conf.cpu2);
            cpus[2].temp = get_temp(&conf.cpu3);
            cpus[2].offset = get_offset(cpus[2].temp, &conf.cpu3);
            cpus[3].temp = get_temp(&conf.cpu4);
            cpus[3].offset = get_offset(cpus[3].temp, &conf.cpu4);
            gpu.temp = get_temp(&conf.gpu);
            gpu.offset = get_offset(gpu.temp, &conf.gpu);
        }

        // cpu's
        for (i=0; i < 4; i++){
            draw_cpu_temp(i, &cpus[i]);
        }

        // gpu
        draw_gpu_temp(&gpu);

        RedrawWindow();
        counter--;
        usleep(100000);
    }
    free(conf.cpu1.path);
    free(conf.cpu2.path);
    free(conf.cpu3.path);
    free(conf.cpu4.path);
    free(conf.gpu.path);
}

void set_defaults(struct config *conf) {
    struct config configuration;
    configuration = *conf;
    configuration.cpu1.critical = 80;
    configuration.cpu1.warning = 65;
    configuration.cpu2.critical = 80;
    configuration.cpu2.warning = 65;
    configuration.cpu3.critical = 80;
    configuration.cpu3.warning = 65;
    configuration.cpu4.critical = 80;
    configuration.cpu4.warning = 65;
    configuration.gpu.critical = 80;
    configuration.gpu.warning = 65;

    configuration.cpu1.path = malloc(sizeof(char));
    strcpy(configuration.cpu1.path, "");
    configuration.cpu2.path = malloc(sizeof(char));
    strcpy(configuration.cpu2.path, "");
    configuration.cpu3.path = malloc(sizeof(char));
    strcpy(configuration.cpu3.path, "");
    configuration.cpu4.path = malloc(sizeof(char));
    strcpy(configuration.cpu4.path, "");
    configuration.gpu.path = malloc(sizeof(char));
    strcpy(configuration.gpu.path, "");
    *conf = configuration;
}

char *strip(char * string) {
    char *string1 = string,
         *string2 = &string[strlen (string) - 1];

    /* Strip right side */
    while ((isspace(*string2)) && (string2 >= string1))
        string2--;

    *(string2+1) = '\0';

    /* Strip left side */
    while ((isspace(*string1)) && (string1 < string2))
        string1++;

    strcpy (string, string1);
    return string;
}

void parse_config(struct config *conf) {
    char *item, 
         *conf_file,
         buff[256],
         name[MAXLEN],
         value[MAXLEN];
    struct config cfg;
    FILE *fp;

    conf_file = malloc(strlen(getenv("HOME")) + strlen("/.wmtemp") + 1);
    sprintf(conf_file, "%s/.wmtemp", getenv("HOME"));

    cfg = *conf;
    fp = fopen (conf_file, "r");
    if (fp == NULL) {
        return;
    }

    while ((item = fgets (buff, sizeof buff, fp)) != NULL) {
        if (buff[0] == '\n' || buff[0] == '#') 
            continue;

        /* Parse name/value pair from item */
        item = strtok(buff, "=");
        strip(item);
        if (item == NULL)
            continue;
        else
            strncpy (name, item, MAXLEN);

        item = strtok (NULL, "=");
        if (item == NULL)
            continue;
        else
            strncpy (value, item, MAXLEN);
        strip(value);

        if (!strcmp(name, "cpu1_path")){
            free(cfg.cpu1.path);
            cfg.cpu1.path = malloc(sizeof(value) + 1);
            strcpy(cfg.cpu1.path, value);
        }

        if (!strcmp(name, "cpu2_path")){
            free(cfg.cpu2.path);
            cfg.cpu2.path = malloc(sizeof(value) + 1);
            strcpy(cfg.cpu2.path, value);
        }

        if (!strcmp(name, "cpu1_path")){
            free(cfg.cpu3.path);
            cfg.cpu3.path = malloc(sizeof(value) + 1);
            strcpy(cfg.cpu3.path, value);
        }

        if (!strcmp(name, "cpu1_path")){
            free(cfg.cpu4.path);
            cfg.cpu4.path = malloc(sizeof(value) + 1);
            strcpy(cfg.cpu4.path, value);
        }

        if (!strcmp(name, "gpu_path")){
            free(cfg.gpu.path);
            cfg.gpu.path = malloc(sizeof(value) + 1);
            strcpy(cfg.gpu.path, value);
        }

        if (!strcmp(name, "cpu1_critical"))
            cfg.cpu1.critical = atoi(value);
        if (!strcmp(name, "cpu2_critical"))
            cfg.cpu2.critical = atoi(value);
        if (!strcmp(name, "cpu3_critical"))
            cfg.cpu3.critical = atoi(value);
        if (!strcmp(name, "cpu4_critical"))
            cfg.cpu4.critical = atoi(value);
        if (!strcmp(name, "gpu_critical"))
            cfg.gpu.critical = atoi(value);
       
        if (!strcmp(name, "cpu1_warning"))
            cfg.cpu1.warning = atoi(value);
        if (!strcmp(name, "cpu2_warning"))
            cfg.cpu2.warning = atoi(value);
        if (!strcmp(name, "cpu3_warning"))
            cfg.cpu3.warning = atoi(value);
        if (!strcmp(name, "cpu4_warning"))
            cfg.cpu4.warning = atoi(value);
        if (!strcmp(name, "gpu_warning"))
            cfg.gpu.warning = atoi(value);
    }

    *conf = cfg;
    fclose (fp);
    free(conf_file);
}

short get_offset(short temp, struct entry *etr){
    if(temp >= etr->critical){
        return 7;  // Alert
    }else if(temp >= etr->warning){
        return 14; // Warning
    }else{
        return 0;  // Normal
    }
}

void display_values(int temp, short offset, short core2_offset){
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

int get_temp(struct entry *etr){
    int core_temp = 0;
    read_file_into(etr->path, &core_temp);
    return core_temp;
}

void read_file_into(char *filepath, int *output) {
    // Read an integer from the provided filepath and write it in the address
    FILE *fp;
    if((fp = fopen(filepath, "r")) != NULL){
        if(fscanf(fp, "%d", output) != EOF){
            *output = *output / 1000;
            fclose(fp);
        }
    }else{
        *output = 0;
    }
}

void draw_cpu_temp(short core_no, core *cpu) {
    // Copy prepared bitmap for the core. Cores are enumerated from 0. offset
    // is warning/critical (orange/red) shift in the bitmap

    short y_offset = core_no * 9;

    copyXPMArea(0, 87 + cpu->offset, 23, 7, 4, 7 + y_offset); // "CPU"
    // number of cpu
    copyXPMArea(5 + core_no * 5, 65 + cpu->offset, 5, 7, 22, 7 + y_offset);
    copyXPMArea(66, 65 + cpu->offset, 9, 7, 51, 7 + y_offset); // "°C"
    display_values(cpu->temp, y_offset, cpu->offset); // temp
}

void draw_gpu_temp(core *gpu) {
    copyXPMArea(23, 87 + gpu->offset, 23, 7, 4, 49);
    copyXPMArea(66, 65 + gpu->offset , 9, 7, 51, 49);
    display_values(gpu->temp, 42, gpu->offset);
}

void display_help(char *progname){
    printf("Dockapp for monitoring CPU and Nvidia GPU temperatures.\n");
    printf("Usage:\n\t%s [full path for temp in sysfs]\n\n", progname);
    printf("As an optional parameter you can provide `temp_input' ");
    printf("full path from sysfs,\ncorresponding to your gfx card ");
    printf("to read temperature from. Otherwise\nfunctionality of ");
    printf("nv-control will be used (package nvidia-settings).\n");
}
