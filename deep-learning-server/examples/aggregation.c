/*
This file contains the functions for supporting aggregation in federated
learning (currently only plain average supported).

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


// this function reads all clients' weights file, load them into seperate 
// networks, then from the first layer to the last layer, averages all 
// weights if this layer has (i.e., Sum / No. of clients).
//
// - Input: all input arguments, including 1) network cfg 2) clients weight files
// - Ouput: Aggregated weights file
//
void average(int argc, char *argv[])
{
    // parse network based on cfg file
    if(argc < 4){
        fprintf(stderr, "usage: %s %s [cfg] [weights_1/weights_2/...]\n", argv[0], argv[1]);
        return;
    }

    char *cfgfile = argv[2];
    char *outfile = argv[3];
    gpu_index = -1;
    network *net = parse_network_cfg(cfgfile);
    network *sum = parse_network_cfg(cfgfile);

    // load the first weights file into sum network (as output later)
    char *weightfile = argv[4];
    load_weights(sum, weightfile);

    int i, j;
    int n = argc - 5;

    // for all other weights files
    for(i = 0; i < n; ++i){
        weightfile = argv[i+5];
        load_weights(net, weightfile);

        // SUM: for every layer, do addition opration
        // only CONVOLUTIONAL and CONNECTED has weights
        for(j = 0; j < net->n; ++j){
            layer l = net->layers[j];
            layer out = sum->layers[j];
            if(l.type == CONVOLUTIONAL){
                int num = l.n*l.c*l.size*l.size;
                axpy_cpu(l.n, 1, l.biases, 1, out.biases, 1);
                axpy_cpu(num, 1, l.weights, 1, out.weights, 1);
                if(l.batch_normalize){
                    axpy_cpu(l.n, 1, l.scales, 1, out.scales, 1);
                    axpy_cpu(l.n, 1, l.rolling_mean, 1, out.rolling_mean, 1);
                    axpy_cpu(l.n, 1, l.rolling_variance, 1, out.rolling_variance, 1);
                }
            }
            if(l.type == CONNECTED){
                axpy_cpu(l.outputs, 1, l.biases, 1, out.biases, 1);
                axpy_cpu(l.outputs*l.inputs, 1, l.weights, 1, out.weights, 1);
            }
        }
    }

    // /No. of clients: for every layer, do division operation
    n = n+1;
    for(j = 0; j < net->n; ++j){
        layer l = sum->layers[j];
        if(l.type == CONVOLUTIONAL){
            int num = l.n*l.c*l.size*l.size;
            scal_cpu(l.n, 1./n, l.biases, 1);
            scal_cpu(num, 1./n, l.weights, 1);
                if(l.batch_normalize){
                    scal_cpu(l.n, 1./n, l.scales, 1);
                    scal_cpu(l.n, 1./n, l.rolling_mean, 1);
                    scal_cpu(l.n, 1./n, l.rolling_variance, 1);
                }
        }
        if(l.type == CONNECTED){
            scal_cpu(l.outputs, 1./n, l.biases, 1);
            scal_cpu(l.outputs*l.inputs, 1./n, l.weights, 1);
        }
    }

    // save the weights of sum network
    save_weights(sum, outfile);
}