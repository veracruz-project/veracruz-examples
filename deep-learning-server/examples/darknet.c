#include "darknet.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_STRINGS 100

extern void average(int argc, char *argv[]);
extern void predict_classifier(char *datacfg, char *cfgfile, char *weightfile, char *filename, int top);
extern void test_detector(char *datacfg, char *cfgfile, char *weightfile, char *filename, float thresh, float hier_thresh, char *outfile, int fullscreen);
// extern void run_yolo(int argc, char **argv);
extern void run_detector(int argc, char **argv);
// extern void run_coco(int argc, char **argv);
// extern void run_nightmare(int argc, char **argv);
extern void run_classifier(int argc, char **argv);
// extern void run_regressor(int argc, char **argv);
// extern void run_segmenter(int argc, char **argv);
// extern void run_isegmenter(int argc, char **argv);
// extern void run_char_rnn(int argc, char **argv);
// extern void run_tag(int argc, char **argv);
// extern void run_cifar(int argc, char **argv);
// extern void run_go(int argc, char **argv);
// extern void run_art(int argc, char **argv);
// extern void run_super(int argc, char **argv);
// extern void run_lsd(int argc, char **argv);


long numops(network *net)
{
    int i;
    long ops = 0;
    for(i = 0; i < net->n; ++i){
        layer l = net->layers[i];
        if(l.type == CONVOLUTIONAL){
            ops += 2l * l.n * l.size*l.size*l.c/l.groups * l.out_h*l.out_w;
        } else if(l.type == CONNECTED){
            ops += 2l * l.inputs * l.outputs;
        } else if (l.type == RNN){
            ops += 2l * l.input_layer->inputs * l.input_layer->outputs;
            ops += 2l * l.self_layer->inputs * l.self_layer->outputs;
            ops += 2l * l.output_layer->inputs * l.output_layer->outputs;
        } else if (l.type == GRU){
            ops += 2l * l.uz->inputs * l.uz->outputs;
            ops += 2l * l.uh->inputs * l.uh->outputs;
            ops += 2l * l.ur->inputs * l.ur->outputs;
            ops += 2l * l.wz->inputs * l.wz->outputs;
            ops += 2l * l.wh->inputs * l.wh->outputs;
            ops += 2l * l.wr->inputs * l.wr->outputs;
        } else if (l.type == LSTM){
            ops += 2l * l.uf->inputs * l.uf->outputs;
            ops += 2l * l.ui->inputs * l.ui->outputs;
            ops += 2l * l.ug->inputs * l.ug->outputs;
            ops += 2l * l.uo->inputs * l.uo->outputs;
            ops += 2l * l.wf->inputs * l.wf->outputs;
            ops += 2l * l.wi->inputs * l.wi->outputs;
            ops += 2l * l.wg->inputs * l.wg->outputs;
            ops += 2l * l.wo->inputs * l.wo->outputs;
        }
    }
    return ops;
}

void speed(char *cfgfile, int tics)
{
    if (tics == 0) tics = 1000;
    network *net = parse_network_cfg(cfgfile);
    set_batch_network(net, 1);
    int i;
    double time=what_time_is_it_now();
    image im = make_image(net->w, net->h, net->c*net->batch);
    for(i = 0; i < tics; ++i){
        network_predict(net, im.data);
    }
    double t = what_time_is_it_now() - time;
    long ops = numops(net);
    printf("\n%d evals, %f Seconds\n", tics, t);
    printf("Floating Point Operations: %.2f Bn\n", (float)ops/1000000000.);
    printf("FLOPS: %.2f Bn\n", (float)ops/1000000000.*tics/t);
    printf("Speed: %f sec/eval\n", t/tics);
    printf("Speed: %f Hz\n", tics/t);
}

void operations(char *cfgfile)
{
    gpu_index = -1;
    network *net = parse_network_cfg(cfgfile);
    long ops = numops(net);
    printf("Floating Point Operations: %ld\n", ops);
    printf("Floating Point Operations: %.2f Bn\n", (float)ops/1000000000.);
}

void oneoff(char *cfgfile, char *weightfile, char *outfile)
{
    gpu_index = -1;
    network *net = parse_network_cfg(cfgfile);
    int oldn = net->layers[net->n - 2].n;
    int c = net->layers[net->n - 2].c;
    scal_cpu(oldn*c, .1, net->layers[net->n - 2].weights, 1);
    scal_cpu(oldn, 0, net->layers[net->n - 2].biases, 1);
    net->layers[net->n - 2].n = 11921;
    net->layers[net->n - 2].biases += 5;
    net->layers[net->n - 2].weights += 5*c;
    if(weightfile){
        load_weights(net, weightfile);
    }
    net->layers[net->n - 2].biases -= 5;
    net->layers[net->n - 2].weights -= 5*c;
    net->layers[net->n - 2].n = oldn;
    printf("%d\n", oldn);
    layer l = net->layers[net->n - 2];
    copy_cpu(l.n/3, l.biases, 1, l.biases +   l.n/3, 1);
    copy_cpu(l.n/3, l.biases, 1, l.biases + 2*l.n/3, 1);
    copy_cpu(l.n/3*l.c, l.weights, 1, l.weights +   l.n/3*l.c, 1);
    copy_cpu(l.n/3*l.c, l.weights, 1, l.weights + 2*l.n/3*l.c, 1);
    *net->seen = 0;
    save_weights(net, outfile);
}

void oneoff2(char *cfgfile, char *weightfile, char *outfile, int l)
{
    gpu_index = -1;
    network *net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights_upto(net, weightfile, 0, net->n);
        load_weights_upto(net, weightfile, l, net->n);
    }
    *net->seen = 0;
    save_weights_upto(net, outfile, net->n);
}

void partial(char *cfgfile, char *weightfile, char *outfile, int max)
{
    gpu_index = -1;
    network *net = load_network(cfgfile, weightfile, 1);
    save_weights_upto(net, outfile, max);
}

void print_weights(char *cfgfile, char *weightfile, int n)
{
    gpu_index = -1;
    network *net = load_network(cfgfile, weightfile, 1);
    layer l = net->layers[n];
    int i, j;
    //printf("[");
    for(i = 0; i < l.n; ++i){
        //printf("[");
        for(j = 0; j < l.size*l.size*l.c; ++j){
            //if(j > 0) printf(",");
            printf("%g ", l.weights[i*l.size*l.size*l.c + j]);
        }
        printf("\n");
        //printf("]%s\n", (i == l.n-1)?"":",");
    }
    //printf("]");
}

void rescale_net(char *cfgfile, char *weightfile, char *outfile)
{
    gpu_index = -1;
    network *net = load_network(cfgfile, weightfile, 0);
    int i;
    for(i = 0; i < net->n; ++i){
        layer l = net->layers[i];
        if(l.type == CONVOLUTIONAL){
            rescale_weights(l, 2, -.5);
            break;
        }
    }
    save_weights(net, outfile);
}

void rgbgr_net(char *cfgfile, char *weightfile, char *outfile)
{
    gpu_index = -1;
    network *net = load_network(cfgfile, weightfile, 0);
    int i;
    for(i = 0; i < net->n; ++i){
        layer l = net->layers[i];
        if(l.type == CONVOLUTIONAL){
            rgbgr_weights(l);
            break;
        }
    }
    save_weights(net, outfile);
}

void reset_normalize_net(char *cfgfile, char *weightfile, char *outfile)
{
    gpu_index = -1;
    network *net = load_network(cfgfile, weightfile, 0);
    int i;
    for (i = 0; i < net->n; ++i) {
        layer l = net->layers[i];
        if (l.type == CONVOLUTIONAL && l.batch_normalize) {
            denormalize_convolutional_layer(l);
        }
        if (l.type == CONNECTED && l.batch_normalize) {
            denormalize_connected_layer(l);
        }
        if (l.type == GRU && l.batch_normalize) {
            denormalize_connected_layer(*l.input_z_layer);
            denormalize_connected_layer(*l.input_r_layer);
            denormalize_connected_layer(*l.input_h_layer);
            denormalize_connected_layer(*l.state_z_layer);
            denormalize_connected_layer(*l.state_r_layer);
            denormalize_connected_layer(*l.state_h_layer);
        }
    }
    save_weights(net, outfile);
}

layer normalize_layer(layer l, int n)
{
    int j;
    l.batch_normalize=1;
    l.scales = calloc(n, sizeof(float));
    for(j = 0; j < n; ++j){
        l.scales[j] = 1;
    }
    l.rolling_mean = calloc(n, sizeof(float));
    l.rolling_variance = calloc(n, sizeof(float));
    return l;
}

void normalize_net(char *cfgfile, char *weightfile, char *outfile)
{
    gpu_index = -1;
    network *net = load_network(cfgfile, weightfile, 0);
    int i;
    for(i = 0; i < net->n; ++i){
        layer l = net->layers[i];
        if(l.type == CONVOLUTIONAL && !l.batch_normalize){
            net->layers[i] = normalize_layer(l, l.n);
        }
        if (l.type == CONNECTED && !l.batch_normalize) {
            net->layers[i] = normalize_layer(l, l.outputs);
        }
        if (l.type == GRU && l.batch_normalize) {
            *l.input_z_layer = normalize_layer(*l.input_z_layer, l.input_z_layer->outputs);
            *l.input_r_layer = normalize_layer(*l.input_r_layer, l.input_r_layer->outputs);
            *l.input_h_layer = normalize_layer(*l.input_h_layer, l.input_h_layer->outputs);
            *l.state_z_layer = normalize_layer(*l.state_z_layer, l.state_z_layer->outputs);
            *l.state_r_layer = normalize_layer(*l.state_r_layer, l.state_r_layer->outputs);
            *l.state_h_layer = normalize_layer(*l.state_h_layer, l.state_h_layer->outputs);
            net->layers[i].batch_normalize=1;
        }
    }
    save_weights(net, outfile);
}

void statistics_net(char *cfgfile, char *weightfile)
{
    gpu_index = -1;
    network *net = load_network(cfgfile, weightfile, 0);
    int i;
    for (i = 0; i < net->n; ++i) {
        layer l = net->layers[i];
        if (l.type == CONNECTED && l.batch_normalize) {
            printf("Connected Layer %d\n", i);
            statistics_connected_layer(l);
        }
        if (l.type == GRU && l.batch_normalize) {
            printf("GRU Layer %d\n", i);
            printf("Input Z\n");
            statistics_connected_layer(*l.input_z_layer);
            printf("Input R\n");
            statistics_connected_layer(*l.input_r_layer);
            printf("Input H\n");
            statistics_connected_layer(*l.input_h_layer);
            printf("State Z\n");
            statistics_connected_layer(*l.state_z_layer);
            printf("State R\n");
            statistics_connected_layer(*l.state_r_layer);
            printf("State H\n");
            statistics_connected_layer(*l.state_h_layer);
        }
        printf("\n");
    }
}

void denormalize_net(char *cfgfile, char *weightfile, char *outfile)
{
    gpu_index = -1;
    network *net = load_network(cfgfile, weightfile, 0);
    int i;
    for (i = 0; i < net->n; ++i) {
        layer l = net->layers[i];
        if ((l.type == DECONVOLUTIONAL || l.type == CONVOLUTIONAL) && l.batch_normalize) {
            denormalize_convolutional_layer(l);
            net->layers[i].batch_normalize=0;
        }
        if (l.type == CONNECTED && l.batch_normalize) {
            denormalize_connected_layer(l);
            net->layers[i].batch_normalize=0;
        }
        if (l.type == GRU && l.batch_normalize) {
            denormalize_connected_layer(*l.input_z_layer);
            denormalize_connected_layer(*l.input_r_layer);
            denormalize_connected_layer(*l.input_h_layer);
            denormalize_connected_layer(*l.state_z_layer);
            denormalize_connected_layer(*l.state_r_layer);
            denormalize_connected_layer(*l.state_h_layer);
            l.input_z_layer->batch_normalize = 0;
            l.input_r_layer->batch_normalize = 0;
            l.input_h_layer->batch_normalize = 0;
            l.state_z_layer->batch_normalize = 0;
            l.state_r_layer->batch_normalize = 0;
            l.state_h_layer->batch_normalize = 0;
            net->layers[i].batch_normalize=0;
        }
    }
    save_weights(net, outfile);
}

void mkimg(char *cfgfile, char *weightfile, int h, int w, int num, char *prefix)
{
    network *net = load_network(cfgfile, weightfile, 0);
    image *ims = get_weights(net->layers[0]);
    int n = net->layers[0].n;
    int z;
    for(z = 0; z < num; ++z){
        image im = make_image(h, w, 3);
        fill_image(im, .5);
        int i;
        for(i = 0; i < 100; ++i){
            image r = copy_image(ims[rand()%n]);
            rotate_image_cw(r, rand()%4);
            random_distort_image(r, 1, 1.5, 1.5);
            int dx = rand()%(w-r.w);
            int dy = rand()%(h-r.h);
            ghost_image(r, im, dx, dy);
            free_image(r);
        }
        char buff[256];
        sprintf(buff, "%s/gen_%d", prefix, z);
        save_image(im, buff);
        free_image(im);
    }
}

void visualize(char *cfgfile, char *weightfile)
{
    network *net = load_network(cfgfile, weightfile, 0);
    visualize_network(net);
}


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
    gpu_index = find_int_arg(argc, argv, "-i", 0);
    if(find_arg(argc, argv, "-nogpu")) {
        gpu_index = -1;
    }

#ifndef GPU
    gpu_index = -1;
#else
    if(gpu_index >= 0){
        cuda_set_device(gpu_index);
    }
#endif
    if (0 == strcmp(argv[1], "aggregation")){
        average(argc, argv);
    // } else if (0 == strcmp(argv[1], "yolo")){
    //     run_yolo(argc, argv);
    // } else if (0 == strcmp(argv[1], "super")){
    //     run_super(argc, argv);
    // } else if (0 == strcmp(argv[1], "lsd")){
    //     run_lsd(argc, argv);
    // } else if (0 == strcmp(argv[1], "detector")){
    //     run_detector(argc, argv);
    } else if (0 == strcmp(argv[1], "detect")){
        float thresh = find_float_arg(argc, argv, "-thresh", .5);
        char *filename = (argc > 4) ? argv[4]: 0;
        char *outfile = find_char_arg(argc, argv, "-out", 0);
        int fullscreen = find_arg(argc, argv, "-fullscreen");
        test_detector("cfg/coco.data", argv[2], argv[3], filename, thresh, .5, outfile, fullscreen);
    // } else if (0 == strcmp(argv[1], "cifar")){
    //     run_cifar(argc, argv);
    // } else if (0 == strcmp(argv[1], "go")){
    //     run_go(argc, argv);
    // } else if (0 == strcmp(argv[1], "rnn")){
    //     run_char_rnn(argc, argv);
    // } else if (0 == strcmp(argv[1], "coco")){
    //     run_coco(argc, argv);
    // } else if (0 == strcmp(argv[1], "classify")){
    //     predict_classifier("cfg/imagenet1k.data", argv[2], argv[3], argv[4], 5);
    } else if (0 == strcmp(argv[1], "classifier")){
        run_classifier(argc, argv);
    // } else if (0 == strcmp(argv[1], "regressor")){
    //     run_regressor(argc, argv);
    // } else if (0 == strcmp(argv[1], "isegmenter")){
    //     run_isegmenter(argc, argv);
    // } else if (0 == strcmp(argv[1], "segmenter")){
    //     run_segmenter(argc, argv);
    // } else if (0 == strcmp(argv[1], "art")){
    //     run_art(argc, argv);
    // } else if (0 == strcmp(argv[1], "tag")){
    //     run_tag(argc, argv);
    // } else if (0 == strcmp(argv[1], "3d")){
    //     composite_3d(argv[2], argv[3], argv[4], (argc > 5) ? atof(argv[5]) : 0);
    // } else if (0 == strcmp(argv[1], "test")){
    //     test_resize(argv[2]);
    // } else if (0 == strcmp(argv[1], "nightmare")){
    //     run_nightmare(argc, argv);
    // } else if (0 == strcmp(argv[1], "rgbgr")){
    //     rgbgr_net(argv[2], argv[3], argv[4]);
    // } else if (0 == strcmp(argv[1], "reset")){
    //     reset_normalize_net(argv[2], argv[3], argv[4]);
    // } else if (0 == strcmp(argv[1], "denormalize")){
    //     denormalize_net(argv[2], argv[3], argv[4]);
    // } else if (0 == strcmp(argv[1], "statistics")){
    //     statistics_net(argv[2], argv[3]);
    // } else if (0 == strcmp(argv[1], "normalize")){
    //     normalize_net(argv[2], argv[3], argv[4]);
    // } else if (0 == strcmp(argv[1], "rescale")){
    //     rescale_net(argv[2], argv[3], argv[4]);
    // } else if (0 == strcmp(argv[1], "ops")){
    //     operations(argv[2]);
    // } else if (0 == strcmp(argv[1], "speed")){
    //     speed(argv[2], (argc > 3 && argv[3]) ? atoi(argv[3]) : 0);
    // } else if (0 == strcmp(argv[1], "oneoff")){
    //     oneoff(argv[2], argv[3], argv[4]);
    // } else if (0 == strcmp(argv[1], "oneoff2")){
    //     oneoff2(argv[2], argv[3], argv[4], atoi(argv[5]));
    // } else if (0 == strcmp(argv[1], "print")){
    //     print_weights(argv[2], argv[3], atoi(argv[4]));
    // } else if (0 == strcmp(argv[1], "partial")){
    //     partial(argv[2], argv[3], argv[4], atoi(argv[5]));
    // } else if (0 == strcmp(argv[1], "average")){
    //     average(argc, argv);
    // } else if (0 == strcmp(argv[1], "visualize")){
    //     visualize(argv[2], (argc > 3) ? argv[3] : 0);
    // } else if (0 == strcmp(argv[1], "mkimg")){
    //     mkimg(argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), argv[7]);
    // } else if (0 == strcmp(argv[1], "imtest")){
    //     test_resize(argv[2]);
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

