/*
This file contains the functions for supporting detection (YOLO object 
detection).

AUTHORS

The Veracruz Development Team.

COPYRIGHT AND LICENSING

See the `LICENSE_MIT.markdown` file in the Veracruz deep learning server 
example repository root directory for copyright and licensing information.
Based on darknet, YOLO LICENSE https://github.com/pjreddie/darknet/blob/master/LICENSE
*/

#include "darknet.h"


// the function to test a object detector, aka detection inference. The prediction
// is outputted as a `prediction` file
// - Input: 1) data cfg (name of all objects), 2) network cfg, 3) weights file
//          4) file to be tested (e.g., a image) 5) threshold for detection
//          6) hierarchy threshold 7) output file path
// - Output: None
void test_detector(char *datacfg, char *cfgfile, char *weightfile, char *filename, float thresh, float hier_thresh, char *outfile)
{
    // read cfg file
    list *options = read_data_cfg(datacfg);
    char *name_list = option_find_str(options, "names", "data/names.list");
    char **names = get_labels(name_list);

    // load network
    network *net = load_network(cfgfile, weightfile, 0);
    set_batch_network(net, 1);

    // load image
    image **alphabet = load_alphabet();
    double time;
    char buff[256];
    char *input = buff;
    float nms=.45;

    if(!filename){
        fprintf(stderr, "file not exists: %s\n", filename);

    }else{
        // load image
        strncpy(input, filename, 256);
        image im = load_image_color(input,0,0);
        image sized = letterbox_image(im, net->w, net->h);
        layer l = net->layers[net->n-1];

        // prediction
        float *X = sized.data;
        time=what_time_is_it_now();
        network_predict(net, X);
        printf("%s: Predicted in %f seconds.\n", input, what_time_is_it_now()-time);
        
        // add boxes for objects
        int nboxes = 0;
        detection *dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, 0, 1, &nboxes);
        //printf("%d\n", nboxes);
        if (nms) do_nms_sort(dets, nboxes, l.classes, nms);
        draw_detections(im, dets, nboxes, thresh, names, alphabet, l.classes);
        free_detections(dets, nboxes);

        // output the file
        if(outfile){
            save_image(im, outfile);
        }else{
            save_image(im, "predictions");
        }
        free_image(im);
        free_image(sized);
    }
}


// this function run the detector for training or inference
//
// - Input: all input arguments, including 1) cfg file, 2) weight files 
//          if exists, 3) input data if exists
// - Ouput: NONE
void run_detector(int argc, char **argv)
{
    // parse all arguments
    char *prefix = find_char_arg(argc, argv, "-prefix", 0);
    float thresh = find_float_arg(argc, argv, "-thresh", .5);
    float hier_thresh = find_float_arg(argc, argv, "-hier", .5);
    int cam_index = find_int_arg(argc, argv, "-c", 0);
    int frame_skip = find_int_arg(argc, argv, "-s", 0);
    int avg = find_int_arg(argc, argv, "-avg", 3);
    if(argc < 4){
        fprintf(stderr, "usage: %s %s [train/test/valid] [cfg] [weights (optional)]\n", argv[0], argv[1]);
        return;
    }

    char *outfile = find_char_arg(argc, argv, "-out", 0);

    int fullscreen = find_arg(argc, argv, "-fullscreen");
    int width = find_int_arg(argc, argv, "-w", 0);
    int height = find_int_arg(argc, argv, "-h", 0);
    int fps = find_int_arg(argc, argv, "-fps", 0);

    char *datacfg = argv[3];
    char *cfg = argv[4];
    char *weights = (argc > 5) ? argv[5] : 0;
    char *filename = (argc > 6) ? argv[6]: 0;

    // use test or train function
    if(0==strcmp(argv[2], "test")) test_detector(datacfg, cfg, weights, filename, thresh, hier_thresh, outfile);
    else if(0==strcmp(argv[2], "demo")) { // one particular demo using test function
        list *options = read_data_cfg(datacfg);
        int classes = option_find_int(options, "classes", 20);
        char *name_list = option_find_str(options, "names", "data/names.list");
        char **names = get_labels(name_list);
        demo(cfg, weights, thresh, cam_index, filename, names, classes, frame_skip, prefix, avg, hier_thresh, width, height, fps, fullscreen);
    }
}
