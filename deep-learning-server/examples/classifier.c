/*
This file contains the functions for classification in both training and
inference.

AUTHORS

The Veracruz Development Team.

COPYRIGHT AND LICENSING

See the `LICENSE_MIT.markdown` file in the Veracruz deep learning server 
example repository root directory for copyright and licensing information.
Based on darknet, YOLO LICENSE https://github.com/pjreddie/darknet/blob/master/LICENSE
*/

#include "darknet.h"

#include <sys/time.h>
#include <assert.h>

// The function for training the classifier. Weights file are saved after
// training.
// - Input: 1) data cfg (list of data paths), 2) network cfg, 3) weights file
//          4) gpus index 5) number of gpus 6) whether to clear the trained
//          previous batches
// - Output: None
void train_classifier(char *datacfg, char *cfgfile, char *weightfile, int *gpus, int ngpus, int clear)
{
    double time;
    time  = what_time_is_it_now(); // time stamp for loading and parsing arguments

    // read cfg files
    int i;
    float avg_loss = -1;
    char *base = basecfg(cfgfile);
    printf("%s\n", base);
    printf("%d\n", ngpus);
    network **nets = calloc(ngpus, sizeof(network *));
    if (nets == NULL)
    {
        printf("ERROR: allocating memory failure. \n");
        exit(EXIT_FAILURE);
    }

    // load into network based cfg and weights file
    for (i = 0; i < ngpus; ++i)
    {
        nets[i] = load_network(cfgfile, weightfile, clear);
        nets[i]->learning_rate *= ngpus;
    }
    network *net = nets[0];
    int imgs = net->batch * net->subdivisions * ngpus;

    // load cfg args
    printf("Learning Rate: %g, Momentum: %g, Decay: %g\n", net->learning_rate, net->momentum, net->decay);
    list *options = read_data_cfg(datacfg);

    char *backup_directory = option_find_str(options, "backup", "/backup/");
    int tag = option_find_int_quiet(options, "tag", 0);
    char *label_list = option_find_str(options, "labels", "data/labels.list");
    char *train_list = option_find_str(options, "train", "data/train.list");
    char *tree = option_find_str(options, "tree", 0);
    if (tree)
        net->hierarchy = read_tree(tree);
    int classes = option_find_int(options, "classes", 2);

    char **labels = 0;
    if (!tag)
    {
        labels = get_labels(label_list);
    }
    list *plist = get_paths(train_list);
    char **paths = (char **)list_to_array(plist);
    printf("%d\n", plist->size);
    int N = plist->size;

    load_args args = {0};
    args.w = net->w;
    args.h = net->h;
    args.hierarchy = net->hierarchy;

    args.min = net->min_ratio * net->w;
    args.max = net->max_ratio * net->w;
    printf("%d %d\n", args.min, args.max);
    args.angle = net->angle;
    args.aspect = net->aspect;
    args.exposure = net->exposure;
    args.saturation = net->saturation;
    args.hue = net->hue;
    args.size = net->w;

    args.paths = paths;
    args.classes = classes;
    args.n = imgs;
    args.m = N;
    args.labels = labels;
    if (tag)
    {
        args.type = TAG_DATA;
    }
    else
    {
        args.type = CLASSIFICATION_DATA;
    }

    data train;
    data buffer;
    args.d = &buffer;
    load_data_single_thread(args);
    
    debug_print("1- Arguments loaded and network parsed: %lf seconds\n", what_time_is_it_now() - time);

    // start training on each batch
    int count = 0;
    int epoch = (*net->seen) / N;
    while (get_current_batch(net) < net->max_batches)
    {
        if (net->random && count++ % 40 == 0)
        {
            printf("Resizing\n");
            int dim = (rand() % 11 + 4) * 32;
            printf("%d\n", dim);
            args.w = dim;
            args.h = dim;
            args.size = dim;
            args.min = net->min_ratio * dim;
            args.max = net->max_ratio * dim;
            printf("%d %d\n", args.min, args.max);

            train = buffer;
            free_data(train);
            load_data_single_thread(args);

            for (i = 0; i < ngpus; ++i)
            {
                resize_network(nets[i], dim, dim);
            }
            net = nets[0];
        }
        time = what_time_is_it_now();  // time stamp for loading dataset (one batch)

        train = buffer;
        load_data_single_thread(args);

        debug_print("2- One batch data loaded: %lf seconds\n", what_time_is_it_now() - time);
        time = what_time_is_it_now(); // time stamp for training on one batch

        float loss = 0;

        loss = train_network(net, train);

        if (avg_loss == -1)
            avg_loss = loss;
        avg_loss = avg_loss * .9 + loss * .1;
        printf("%ld, %.3f: %f, %f avg, %f rate, %lf seconds, %ld images\n", get_current_batch(net), (float)(*net->seen) / N, loss, avg_loss, get_current_rate(net), what_time_is_it_now() - time, *net->seen);
        debug_print("3- One batch data trained: %lf seconds\n", what_time_is_it_now() - time);
        free_data(train);

        // save weights as backup
        if (*net->seen / N > epoch)
        {
            epoch = *net->seen / N;
            char buff[256];
            sprintf(buff, "%s/%s_%d.weights", backup_directory, base, epoch);
            save_weights(net, buff);
        }
        if (get_current_batch(net) % 1000 == 0)
        {
            char buff[256];
            sprintf(buff, "%s/%s.backup", backup_directory, base);
            save_weights(net, buff);
        }
    }

    time = what_time_is_it_now(); // time stamp for saving the weights

    // save weights in the end
    char buff[256];
    sprintf(buff, "%s/%s.weights", backup_directory, base);
    save_weights(net, buff);
    debug_print("4- Weights saved: %lf seconds\n", what_time_is_it_now() - time);

    // free buffer
    free_network(net);
    if (labels)
        free_ptrs((void **)labels, classes);
    free_ptrs((void **)paths, plist->size);
    free_list(plist);
    free(base);
}

//  The function for validating the classifier on one dataset, aka, a list of
//  data samples. Predictions are outputted in command line.
//
// - Input: 1) data cfg (list of data paths), 2) network cfg, 3) weights file
// - Output: None
void validate_classifier_single(char *datacfg, char *filename, char *weightfile)
{
    double time;
    time  = what_time_is_it_now(); // time stamp for loading and parsing arguments

    // load network based on cfg and weights files
    int i, j;
    network *net = load_network(filename, weightfile, 0);
    set_batch_network(net, 1);

    // load data cfg and all paths
    list *options = read_data_cfg(datacfg);
    char *label_list = option_find_str(options, "labels", "data/labels.list");
    char *leaf_list = option_find_str(options, "leaves", 0);
    if (leaf_list)
        change_leaves(net->hierarchy, leaf_list);
    char *valid_list = option_find_str(options, "valid", "data/train.list");
    int classes = option_find_int(options, "classes", 2);
    int topk = option_find_int(options, "top", 1);

    char **labels = get_labels(label_list);
    list *plist = get_paths(valid_list);

    char **paths = (char **)list_to_array(plist);
    int m = plist->size;
    free_list(plist);

    float avg_acc = 0;
    float avg_topk = 0;
    int *indexes = calloc(topk, sizeof(int));
    if (indexes == NULL)
    {
        printf("ERROR: allocating memory failure. \n");
        exit(EXIT_FAILURE);
    }

    debug_print("1- Arguments loaded and network parsed: %lf seconds\n", what_time_is_it_now() - time);

    // predict on all data samples
    for (i = 0; i < m; ++i)
    {
        int class = -1;
        char *path = paths[i];
        for (j = 0; j < classes; ++j)
        {
            if (strstr(path, labels[j]))
            {
                class = j;
                break;
            }
        }

        // load one image and predict
        time = what_time_is_it_now(); // time stamp for loading one image
        image im = load_image_color(paths[i], 0, 0);
        image crop = center_crop_image(im, net->w, net->h);
        
        debug_print("2- One image loaded: %lf seconds\n", what_time_is_it_now() - time);
        time = what_time_is_it_now(); // time stamp for predicting one image

        float *pred = network_predict(net, crop.data);
        if (net->hierarchy)
            hierarchy_predictions(pred, net->outputs, net->hierarchy, 1, 1);

        free_image(im);
        free_image(crop);
        top_k(pred, classes, topk, indexes);

        if (indexes[0] == class)
            avg_acc += 1;
        for (j = 0; j < topk; ++j)
        {
            if (indexes[j] == class)
                avg_topk += 1;
        }

        printf("%s, %d, %f, %f, \n", paths[i], class, pred[0], pred[1]);
        printf("%d: top 1: %f, top %d: %f\n", i, avg_acc / (i + 1), topk, avg_topk / (i + 1));
        debug_print("3- One image predicted: %lf seconds\n", what_time_is_it_now() - time);
    }
}

//  The function for validating the classifier on one data sample. Prediction
//  is outputted in command line.
//
// - Input: 1) data cfg (list of data paths), 2) network cfg, 3) weights file
//          4) the path of the data sample to be classified 5) top-k
// - Output: None
void predict_classifier(char *datacfg, char *cfgfile, char *weightfile, char *filename, int top)
{
    // load network based cfg and weights files
    network *net = load_network(cfgfile, weightfile, 0);
    set_batch_network(net, 1);

    // load data based cfg
    list *options = read_data_cfg(datacfg);
    char *name_list = option_find_str(options, "names", 0);
    if (!name_list)
        name_list = option_find_str(options, "labels", "data/labels.list");
    if (top == 0)
        top = option_find_int(options, "top", 1);

    int i = 0;
    char **names = get_labels(name_list);

    int *indexes = calloc(top, sizeof(int));
    if (indexes == NULL)
    {
        printf("ERROR: allocating memory failure. \n");
        exit(EXIT_FAILURE);
    }

    char buff[256];
    char *input = buff;

    if (!filename)
    {
        fprintf(stderr, "file not exists: %s\n", filename);
    }
    else
    {
        strncpy(input, filename, 256);
        image im = load_image_color(input, 0, 0);
        image r = letterbox_image(im, net->w, net->h);

        float *X = r.data;
        float *predictions = network_predict(net, X);
        if (net->hierarchy)
            hierarchy_predictions(predictions, net->outputs, net->hierarchy, 1, 1);
        top_k(predictions, net->outputs, top, indexes);
        fprintf(stderr, "%s: Predicted.\n", input);
        for (i = 0; i < top; ++i)
        {
            int index = indexes[i];
            printf("%5.2f%%: %s\n", predictions[index] * 100, names[index]);
        }
        if (r.data != im.data)
            free_image(r);
        free_image(im);
    }
}

// this function is the entry to run the classifier in terms of both training
// and inference.
//
// - Input: all input arguments, including 1) cfg file, 2) weight files
//          if exists, 3) input data if exists
// - Ouput: NONE
void run_classifier(int argc, char **argv)
{
    // parse all arguments
    if (argc < 4)
    {
        fprintf(stderr, "usage: %s %s [train/predict/valid] [cfg] [weights (optional)]\n", argv[0], argv[1]);
        return;
    }

    char *gpu_list = find_char_arg(argc, argv, "-gpus", 0);
    int ngpus;
    int *gpus = read_intlist(gpu_list, &ngpus, gpu_index);

    int top = find_int_arg(argc, argv, "-t", 0);
    int clear = find_arg(argc, argv, "-clear");
    char *data = (argc > 3) ? argv[3] : 0;
    char *cfg = (argc > 4) ? argv[4] : 0;
    char *weights = (argc > 5) ? argv[5] : 0;
    char *filename = (argc > 6) ? argv[6] : 0;

    // use test or train function
    if (0 == strcmp(argv[2], "predict"))
        predict_classifier(data, cfg, weights, filename, top);
    else if (0 == strcmp(argv[2], "train"))
        train_classifier(data, cfg, weights, gpus, ngpus, clear);
    else if (0 == strcmp(argv[2], "valid"))
        validate_classifier_single(data, cfg, weights);
    else
    {
        fprintf(stderr, "Not an option under classifier: %s\n", argv[2]);
    }
}
