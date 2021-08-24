/*
This file contains the functions for supporting aggregation in federated
learning (currently only plain average supported).

AUTHORS

The Veracruz Development Team.

COPYRIGHT AND LICENSING

See the `LICENSE_MIT.markdown` file in the Veracruz deep learning server 
example repository root directory for copyright and licensing information.
Based on darknet, YOLO LICENSE https://github.com/pjreddie/darknet/blob/master/LICENSE
Based on cONNXr, MIT LICENSE https://github.com/alrevuelta/cONNXr/blob/master/LICENSE
*/

#include "darknet.h"
#include "../connxr/include/utils.h"
#include "../connxr/protobuf/onnx.pb-c.h"
#include "../connxr/include/inference.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// For Darknet format weights
// this function reads all clients' weights file, load them into seperate 
// networks, then from the first layer to the last layer, averages all 
// weights if this layer has (i.e., Sum / No. of clients).
//
// - Input: all input arguments, including 1) network cfg 2) clients weight files
// - Ouput: Aggregated weights file
//
void average_darknet(int argc, char *argv[])
{
    char *cfgfile = argv[3];
    char *outfile = argv[4];
    gpu_index = -1;
    network *net = parse_network_cfg(cfgfile);
    network *sum = parse_network_cfg(cfgfile);

    // load the first weights file into sum network (as output later)
    char *weightfile = argv[5];
    load_weights(sum, weightfile);

    int i, j;
    int n = argc - 5;

    // for all other weights files
    for(i = 0; i < n; ++i){
        weightfile = argv[i+6];
        load_weights(net, weightfile);

        // SUM: for every layer, do addition opration
        // only CONVOLUTIONAL and CONNECTED have weights
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


// For ONNX format weights
// this function reads all clients' weights file, load them into seperate 
// networks, then from the first layer to the last layer, averages all 
// weights if this layer has (i.e., Sum / No. of clients).
//
// - Input: all input arguments, including 1) network cfg 2) clients weight files
// - Ouput: Aggregated weights file
//
void average_onnx(int argc, char *argv[])
{
    char *output_file_dir = argv[3];

    // load the first weights file into sum network (as output later)
    Onnx__ModelProto *model_sum = openOnnxFile(argv[4]);
    if (model_sum != NULL){printf("model c1 loaded\n");}

    // for all other weights files
    int n = argc - 5;
    for(int i = 0; i < n; ++i){
        Onnx__ModelProto *model_c = openOnnxFile(argv[i+5]);
        if (model_c != NULL){printf("model c%d loaded\n", i+2);}

        // SUM: for each tensor in the graph
        for(int j=0; j < model_sum->graph->n_initializer; j++){
            int32_t dt = model_sum->graph->initializer[j]->data_type; //data type

            //FLOAT or COMPLEX64
            if (dt == 1 || dt == 14){
                int ts = model_sum->graph->initializer[j]->n_float_data;
                float *td_sum = model_sum->graph->initializer[j]->float_data;
                float *td_c = model_c->graph->initializer[j]->float_data;
                for(int k=0; k < ts; k++){
                    td_sum[k] = td_sum[k] + td_c[k];
                }
                
            // INT32, INT16, INT8, UINT16, UINT8, BOOL, or FLOAT16
            }else if (dt==2 || dt==3 || dt==4 || dt==5 || dt==6 || dt==9 || dt==10){
                int ts = model_sum->graph->initializer[j]->n_int32_data;
                int32_t *td_sum = model_sum->graph->initializer[j]->int32_data;
                int32_t *td_c = model_c->graph->initializer[j]->int32_data;
                for(int k=0; k < ts; k++){
                    td_sum[k] = td_sum[k] + td_c[k];
                }

            // INT64
            }else if (dt == 7){
                int ts = model_sum->graph->initializer[j]->n_int64_data;
                int64_t *td_sum = model_sum->graph->initializer[j]->int64_data;
                int64_t *td_c = model_c->graph->initializer[j]->int64_data;
                for(int k=0; k < ts; k++){
                    td_sum[k] = td_sum[k] + td_c[k];
                }
            
            // DOUBLE or COMPLEX128
            }else if (dt == 11 || dt == 15){
                int ts = model_sum->graph->initializer[j]->n_double_data;
                double *td_sum = model_sum->graph->initializer[j]->double_data;
                double *td_c = model_c->graph->initializer[j]->double_data;
                for(int k=0; k < ts; k++){
                    td_sum[k] = td_sum[k] + td_c[k];
                }

            }else{
                fprintf(stderr, "unknown or undefined data types [%d]: tensor no.%d\n", dt, j);
            }
        }
    }

    // AVERAGE: for each tensor in the graph
    for(int j=0; j < model_sum->graph->n_initializer; j++){
        //printf("aggregating layer %d in %d\n", j, model_sum->graph->n_initializer);
        int32_t dt = model_sum->graph->initializer[j]->data_type; //data type

        //FLOAT or COMPLEX64
        if (dt == 1 || dt == 14){
            int ts = model_sum->graph->initializer[j]->n_float_data;
            float *td_sum = model_sum->graph->initializer[j]->float_data;
            for(int k=0; k < ts; k++){
                td_sum[k] = td_sum[k] / (n+1);
            }

        // INT32, INT16, INT8, UINT16, UINT8, BOOL, or FLOAT16
        }else if (dt==2 || dt==3 || dt==4 || dt==5 || dt==6 || dt==9 || dt==10){
            int ts = model_sum->graph->initializer[j]->n_int32_data;
            int32_t *td_sum = model_sum->graph->initializer[j]->int32_data;
            for(int k=0; k < ts; k++){
                td_sum[k] = td_sum[k] / (n+1);
            }
            
        // INT64
        }else if (dt == 7){
            int ts = model_sum->graph->initializer[j]->n_int64_data;
            int64_t *td_sum = model_sum->graph->initializer[j]->int64_data;
            for(int k=0; k < ts; k++){
                td_sum[k] = td_sum[k] / (n+1);
            }

        // DOUBLE or COMPLEX128
        }else if (dt == 11 || dt == 15){
            int ts = model_sum->graph->initializer[j]->n_double_data;
            double *td_sum = model_sum->graph->initializer[j]->double_data;
            for(int k=0; k < ts; k++){
                td_sum[k] = td_sum[k] / (n+1);
            }
        }else{
            fprintf(stderr, "unknown or undefined data types [%d]: tensor no.%d\n", dt, j);
        }
    }

    // save onnx weights
    saveOnnxFile(model_sum, output_file_dir);
    printf("save aggregation model to '%s' done!\n", output_file_dir);
}



// this function here is provided to test the onnx model by using a  
// inference function. Prediction is outputted with `n_float_data`
//
// - Input: all input arguments, including 1) model 2) input data
// - Ouput: NONE
void predict_onnx(int argc, char *argv[]){

    // load the weights file
    Onnx__ModelProto *model = openOnnxFile(argv[3]);
    if (model != NULL){printf("model loaded\n");}
    
    // load the input data
    char *input_str = argv[4];
    Onnx__TensorProto *inp0set0 = openTensorProtoFile(input_str);
    if (inp0set0 != NULL){printf("Loading input %s... ok!\n", input_str);}
    convertRawDataOfTensorProto(inp0set0);
    inp0set0->name = model->graph->input[0]->name;
    Onnx__TensorProto *inputs[] = { inp0set0 };

    // resolve the model and run inference
    printf("Resolving model...\n");
    resolve(model, inputs, 1);
    printf("Running inference on %s model...\n", model->graph->name);
    inference(model, inputs, 1);
    printf("finished!\n");

    // print the last output which should be the model output
    for (int i = 0; i < all_context[_populatedIdx].outputs[0]->n_float_data; i++){
        printf("n_float_data[%d] = %f\n", i, all_context[_populatedIdx].outputs[0]->float_data[i]);
    }
}



// this function is the entry to run the aggregation in terms of two format
// : Darknet or ONNX.
//
// - Input: all input arguments, including 1) cfg file, 2) weight files 
//          if exists, 3) input data if exists
// - Ouput: NONE
void run_aggregation(int argc, char **argv)
{   
    // parse network based on cfg file
    if(argc < 4){
        fprintf(stderr, "usage: %s %s [weights_format] [cfg]/NONE [weights_1/weights_2/...]\n", argv[0], argv[1]);
        return;
    }

    // use darknet or onnx weight format
    if(0==strcmp(argv[2], "darknet")) average_darknet(argc, argv);
    else if(0==strcmp(argv[2], "onnx")) average_onnx(argc, argv);
    else if(0==strcmp(argv[2], "onnx_predict")) predict_onnx(argc, argv);
    else{
        fprintf(stderr, "Not an option under aggregation: %s\n", argv[2]);
    }
}