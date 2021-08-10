/*
This file defines function calls and the entry in the main wasm binary.

AUTHORS

The Veracruz Development Team.

COPYRIGHT AND LICENSING

See the `LICENSE_MIT.markdown` file in the Veracruz deep learning server 
example repository root directory for copyright and licensing information.
Based on darknet, YOLO LICENSE https://github.com/pjreddie/darknet/blob/master/LICENSE
*/

#include "darknet.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_STRINGS 100

extern void average(int argc, char *argv[]);
extern void predict_classifier(char *datacfg, char *cfgfile, char *weightfile, char *filename, int top);
extern void test_detector(char *datacfg, char *cfgfile, char *weightfile, char *filename, float thresh, float hier_thresh, char *outfile, int fullscreen);
extern void run_detector(int argc, char **argv);
extern void run_classifier(int argc, char **argv);


void file_args_parser(char * args_file)
{
    int argc = 1;
    char argv_[10][MAX_STRINGS];
    char line[MAX_STRINGS];

    FILE *file; 
    file = fopen(args_file, "r"); 

    while(fgets(line, sizeof line, file) != NULL) 
    {
        line[strcspn(line, "\n")] = 0;
        strcpy(argv_[argc], line);
        printf("argv[%d] = '%s'\n", argc, argv_[argc]);
        argc++;
    }
    fclose(file);

    char *argv[] = {&argv_[0][0], &argv_[1][0], &argv_[2][0], &argv_[3][0], &argv_[4][0], &argv_[5][0], 
                    &argv_[6][0], &argv_[7][0], &argv_[8][0], &argv_[9][0], NULL};

    if (0 == strcmp(argv[1], "aggregation")){
        average(argc, argv);
    } else if (0 == strcmp(argv[1], "detect")){
        float thresh = find_float_arg(argc, argv, "-thresh", .5);
        char *filename = (argc > 4) ? argv[4]: 0;
        char *outfile = find_char_arg(argc, argv, "-out", 0);
        int fullscreen = find_arg(argc, argv, "-fullscreen");
        test_detector("cfg/coco.data", argv[2], argv[3], filename, thresh, .5, outfile, fullscreen);
    } else if (0 == strcmp(argv[1], "classifier")){
        run_classifier(argc, argv);
    } else {
        fprintf(stderr, "Not an option: %s\n", argv[1]);
    }
}


void cmdline_args_parser(int argc, char **argv)
{
    gpu_index = -1;

    if (0 == strcmp(argv[1], "aggregation")){
        average(argc, argv);
    } else if (0 == strcmp(argv[1], "detect")){
        float thresh = find_float_arg(argc, argv, "-thresh", .5);
        char *filename = (argc > 4) ? argv[4]: 0;
        char *outfile = find_char_arg(argc, argv, "-out", 0);
        int fullscreen = find_arg(argc, argv, "-fullscreen");
        test_detector("cfg/coco.data", argv[2], argv[3], filename, thresh, .5, outfile, fullscreen);
    } else if (0 == strcmp(argv[1], "classifier")){
        run_classifier(argc, argv);
    } else {
        fprintf(stderr, "Not an option: %s\n", argv[1]);
    }
}


int main(int argc, char **argv)
{
    char *args_file = "args_file.cfg";
    FILE *fp = fopen(args_file, "r");
    
    if(argc >= 2){
        cmdline_args_parser(argc, argv);
    }else if(fp != NULL)
    {
        file_args_parser(args_file);
    }else
    {
        fprintf(stderr, "Argument file not found. \nCommand-line arguments not found.\n");
    }
    return 0;
}

