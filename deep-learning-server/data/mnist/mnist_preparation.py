# This file gets the MNIST dataset prepared for training and inference on it.
# First, it downloads this dataset if the source files do not exist. Then,
# source files are unzipped, and at the same time, a list file, which contains
# the relative paths of all unzipped images are produced.
#
# AUTHORS
#
# The Veracruz Development Team.
#
# COPYRIGHT AND LICENSING
#
# See the `LICENSE_MIT.markdown` file in the Veracruz deep learning server 
# example repository root directory for copyright and licensing information.
# Based on darknet, YOLO LICENSE https://github.com/pjreddie/darknet/blob/master/LICENSE

#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy as np
import cv2
import os
import argparse

# arguments
parser = argparse.ArgumentParser(description='Arguments to process the dataset')
parser.add_argument('--num-clients', type=int, default=1,
                    help='the number of clients who held fractions of the dataset')
parser.add_argument('--data-dist', type=str, default='iid',
                    help="the data distribution among clients [iid] or [non-iid] (NOT IMPLEMENTED)")
args = parser.parse_args()

mnist_path = 'data/mnist/'

# if MNIST file not exist, then download into `mnist_path` and unzip
if not os.path.exists(mnist_path + 'train-images-idx3-ubyte'):
    print("Downloading mnist files...")
    data_files=["train-images-idx3-ubyte.gz",
    "train-labels-idx1-ubyte.gz",
    "t10k-images-idx3-ubyte.gz",
    "t10k-labels-idx1-ubyte.gz"]

    for data_file in data_files:
        os.system('wget -P ' + mnist_path + ' http://yann.lecun.com/exdb/mnist/' + data_file)
        os.system('gunzip ' + mnist_path + data_file + ' ' + mnist_path)


# image dir(s) for all clients. Images will be unzipped into folders
# `images_client#NO` for clients `#NO`
print("the number of clients are: {}".format(args.num_clients))
if args.num_clients == 1:
    img_path = mnist_path + 'images'
    if not os.path.exists(img_path):
        os.mkdir(img_path)
else:
    client_no = list(range(1, args.num_clients+1))
    img_path = [mnist_path + 'images_client' + str(i) for i in client_no]
    for _path in img_path:
        if not os.path.exists(_path):
            os.mkdir(_path)


# the main function call converting one source file to images and
# corresponding list file, which contains all images' relative path.
#
# - Input: 1) image source file, 2) label source file, 3) number of 
#           contained images, 4) output image path, 5) output list 
#           file name, 6) valid or train label
# - Ouput: 1) unzipped images 2) list files with all images' path
#
def conv_mnist(img_file, label_file, num, path, list, label):

    # open image source file
    imgs = open(mnist_path+img_file,"rb").read()
    imgs = np.fromstring(imgs,dtype=np.uint8)
    imgs = imgs[16:] # skip header
    imgs = imgs.reshape((num,28,28))

    # open label source file
    labels = open(mnist_path+label_file,"rb").read()
    labels = np.fromstring(labels,dtype=np.uint8)
    labels = labels[8:] # skip header

    # for one in all images
    fw = open(mnist_path+list,"w")
    for i in range(num):
        class_id = labels[i]
        img_name = "%s_%05d_c%d.png" % (label,i,class_id)

        if args.num_clients == 1:
            img_path = path + "/" + img_name
        else:
            p_i = int(i/num * args.num_clients) # 0~9
            img_path = path[p_i] + "/" + img_name
        
        # write to iamge files
        cv2.imwrite(img_path, imgs[i])

        # write to the path list
        fw.write(img_path + "\n")

    fw.close()


# call the conv_mnist function on 1) valid source files 2) training 
# source files
conv_mnist('t10k-images-idx3-ubyte', 't10k-labels-idx1-ubyte', 10000, img_path, "mnist.valid.list", "v")
conv_mnist('train-images-idx3-ubyte', 'train-labels-idx1-ubyte', 60000, img_path, "mnist.train.list", "t")