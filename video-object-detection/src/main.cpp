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

extern "C" {
    #include "darknet.h"
}
#include "codec_def.h"
#include "h264dec.h"
#include "utils.h"

int frames_processed = 0;
// Network state, to be initialized by `init_detector()`
char **names;
network *net;
image **alphabet;

// the function to test a object detector, aka detection inference. The prediction
// is outputted as a `prediction` file
// - Input: 1) data cfg (name of all objects), 2) network cfg, 3) weights file
//          4) file to be tested (e.g., a image) 5) threshold for detection
//          6) hierarchy threshold 7) output file path
// - Output: None
void init_darknet_detector(char *datacfg, char *cfgfile, char *weightfile)
{
    // read cfg file
    list *options = read_data_cfg(datacfg);
    char *name_list = option_find_str(options, "names", "data/names.list");
    names = get_labels(name_list);

    // load network
    net = load_network(cfgfile, weightfile, 0);
    set_batch_network(net, 1);

    // load image
    alphabet = load_alphabet();
}

// Two images are required: the image to be processed by the model, and the initial Darknet image to be annotated with the detection boxes
void run_darknet_detector(image im, image im_sized, float thresh, float hier_thresh, char *outfile, bool draw_detection_boxes)
{
    float nms = .45;

    // Run network prediction
    float *X = im_sized.data;
    network_predict(net, X);

    // Get detections
    int nboxes = 0;
    layer l = net->layers[net->n - 1];
    detection *dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, 0, 1, &nboxes);
    if (nms)
        do_nms_sort(dets, nboxes, l.classes, nms);
    printf("Detection probabilities:\n");
    print_detection_probabilities(im, dets, nboxes, thresh, names, l.classes);

    // add boxes for objects
    if (draw_detection_boxes) {
        draw_detections(im, dets, nboxes, thresh, names, alphabet, l.classes);

        // output the file
        if (outfile)
        {
            save_image(im, outfile);
        }
        else
        {
            save_image(im, "predictions");
        }
    }
    free_detections(dets, nboxes);

    free_image(im);
    free_image(im_sized);
}

// Convert OpenH264 I420 frame into a darknet image structure, then convert it to RGB
image normalize_frame(SBufferInfo *bufInfo)
{
    int width = bufInfo->UsrData.sSystemBuffer.iWidth;
    int height = bufInfo->UsrData.sSystemBuffer.iHeight;
    unsigned char *yuv_data_linearized;
    image im;

    yuv_data_linearized = (unsigned char *) malloc(width*height + 2*width/2*height/2);
    linearize_openh264_frame_buffer(bufInfo, yuv_data_linearized);

    im = load_image_color_from_raw_yuv(yuv_data_linearized, width, height);

    free(yuv_data_linearized);

    return im;
}

// Callback called by the H.264 decoder whenever a frame is decoded and ready
void onFrameReady(SBufferInfo *bufInfo) {
    image im, im_sized;
    double time;

    printf("Image %d ===========================\n", frames_processed);

    time = what_time_is_it_now();

    im = normalize_frame(bufInfo);

    // Resize image to fit the darknet model
    im_sized = letterbox_image(im, net->w, net->h);

    debug_print("Image normalized and resized: %lf seconds\n", what_time_is_it_now() - time);

    time = what_time_is_it_now();
    run_darknet_detector(im, im_sized, .5, .5, "predictions", true);
    debug_print("Detector run: %lf seconds\n", what_time_is_it_now() - time);
	frames_processed++;
}

int main(int argc, char **argv)
{
    double time;
	char *input_file = argv[1];

    printf("Initizalizing detector...\n");
    time  = what_time_is_it_now();
    init_darknet_detector("cfg/coco.data", "cfg/yolov3.cfg", "model/yolov3.weights");
    debug_print("Arguments loaded and network parsed: %lf seconds\n", what_time_is_it_now() - time);

    printf("Starting decoding...\n");
    time  = what_time_is_it_now();
    int x = h264_decode(input_file, "", false, &onFrameReady);
    debug_print("Finished decoding: %lf seconds\n", what_time_is_it_now() - time);

    return x;
}
