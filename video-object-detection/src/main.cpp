/*
This file contains the main functions for performing object detection.
First, the object detection model is loaded, then the video decoder executes
until every frame in the video is decoded.
A callback is configured to be called whenever a frame is available, whereupon
it is fed to the object detection model which outputs a prediction.

AUTHORS

The Veracruz Development Team.

COPYRIGHT AND LICENSING

See the `LICENSE_MIT.markdown` file in the example's root directory for
copyright and licensing information.
Based on darknet, YOLO LICENSE https://github.com/pjreddie/darknet/blob/master/LICENSE
*/

extern "C" {
    #include "darknet.h"
}
#include "codec_def.h"
#include "h264dec.h"
#include "utils.h"

// Keep track of the number of frames processed
int frames_processed = 0;

// Network state, to be initialized by `init_detector()`
char **names;
network *net;
image **alphabet;

// Initialize the Darknet model (neural network)
// The prediction is outputted as a `prediction` file
// Input:
//   - data cfg (name of all objects)
//   - network cfg
//   - weights file
//   - whether detection boxes should be annotated with the name of the detected
//     object (requires an alphabet)
// Output: None
void init_darknet_detector(char *name_list_file, char *cfgfile, char *weightfile,
                           bool annotate_boxes)
{
    // Get name list
    names = get_labels(name_list_file);

    // Load network
    net = load_network(cfgfile, weightfile, 0);
    set_batch_network(net, 1);

    // Load alphabet (set of images corresponding to characters)
    if (annotate_boxes)
        alphabet = load_alphabet();
}

// Feed an image to the object detection model.
// Input:
//   - initial image to be annotated with the detection boxes
//   - image to be processed by the model
//   - detection threshold
//   - hierarchy threshold
//   - output file path
//   - whether detection boxes should be drawn and saved to a file
// Output: None
void run_darknet_detector(image im, image im_sized, float thresh,
                          float hier_thresh, char *outfile,
                          bool draw_detection_boxes)
{
    float nms = .45;

    // Run network prediction
    float *X = im_sized.data;
    network_predict(net, X);

    // Get detections
    int nboxes = 0;
    layer l = net->layers[net->n - 1];
    detection *dets = get_network_boxes(net, im.w, im.h, thresh, hier_thresh, 0,
                                        1, &nboxes);
    if (nms)
        do_nms_sort(dets, nboxes, l.classes, nms);
    printf("Detection probabilities:\n");
    print_detection_probabilities(im, dets, nboxes, thresh, names, l.classes);

    // add boxes for objects
    if (draw_detection_boxes) {
        draw_detections(im, dets, nboxes, thresh, names, alphabet, l.classes);

        // output the file
        if (outfile)
            save_image(im, outfile);
        else
            save_image(im, "predictions");
    }
    free_detections(dets, nboxes);

    free_image(im);
    free_image(im_sized);
}

// Callback called by the H.264 decoder whenever a frame is decoded and ready
// Input: OpenH264 I420 frame buffer
// Output: None
void onFrameReady(SBufferInfo *bufInfo)
{
    image im, im_sized;
    double time;

    printf("Image %d ===========================\n", frames_processed);

    time = what_time_is_it_now();

    im = load_image_from_raw_yuv(bufInfo);

    // Resize image to fit the darknet model
    im_sized = letterbox_image(im, net->w, net->h);

    debug_print("Image normalized and resized: %lf seconds\n",
                what_time_is_it_now() - time);

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
    init_darknet_detector("data/coco.names", "cfg/yolov3.cfg",
                          "model/yolov3.weights", false);
    debug_print("Arguments loaded and network parsed: %lf seconds\n",
                what_time_is_it_now() - time);

    printf("Starting decoding...\n");
    time  = what_time_is_it_now();
    int x = h264_decode(input_file, "", false, &onFrameReady);
    debug_print("Finished decoding: %lf seconds\n",
                what_time_is_it_now() - time);

    return x;
}
