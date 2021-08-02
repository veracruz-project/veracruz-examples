#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy as np
import cv2
import os
import argparse

num_clients = 10

# arguments
parser = argparse.ArgumentParser(description='Arguments to process the dataset')
parser.add_argument('--num_clients', type=int, default=1,
                    help='the number of clients who held frictions of the dataset')
parser.add_argument('--data_dist', type=str, default='iid',
                    help="the data distribution among clients [iid] or [non-iid]")
parser.add_argument('--relat_path_list', type=bool, default=True,
                    help="which to use relative image path in the loading list")
args = parser.parse_args()


# if MNIST file not exist, then download
if not os.path.exists("train-images-idx3-ubyte"):
    print("Downloading mnist files...")
    data_files=["train-images-idx3-ubyte.gz",
    "train-labels-idx1-ubyte.gz",
    "t10k-images-idx3-ubyte.gz",
    "t10k-labels-idx1-ubyte.gz"]

    for data_file in data_files:
        os.system('curl -O http://yann.lecun.com/exdb/mnist/'+data_file)
        os.system('gunzip '+data_file)


# image dir
cwd = os.getcwd()
print("the number of clients are: {}".format(args.num_clients))
if args.num_clients == 1:
    img_path = cwd + "/images"
    relat_img_path = "data/mnist/images"
    if not os.path.exists(img_path):
        os.mkdir(img_path)
else:
    client_no = list(range(1, args.num_clients+1))
    img_path = [cwd + "/images_client" + str(i) for i in client_no]
    relat_img_path = ["data/mnist" + "/images_client" + str(i) for i in client_no]
    for _path in img_path:
        if not os.path.exists(_path):
            os.mkdir(_path)


# convert function
def conv_mnist(img_file, label_file, num, path, relat_path, list, label):

    # open image file
    imgs = open(img_file,"rb").read()
    imgs = np.fromstring(imgs,dtype=np.uint8)
    imgs = imgs[16:] # skip header
    imgs = imgs.reshape((num,28,28))

    # open label file
    labels = open(label_file,"rb").read()
    labels = np.fromstring(labels,dtype=np.uint8)
    labels = labels[8:] # skip header

    # for one in all images
    fw = open(list,"w")

    for i in range(num):
        class_id = labels[i]
        img_name = "%s_%05d_c%d.png" % (label,i,class_id)

        if args.num_clients == 1:
            img_path = path + "/" + img_name
            relat_img_path = relat_path + "/" + img_name
        else:
            p_i = int(i/num * args.num_clients) # 0~9
            img_path = path[p_i] + "/" + img_name
            relat_img_path = relat_path[p_i] + "/" + img_name
        
        # write to iamge files
        cv2.imwrite(img_path, imgs[i])

        # write to the path list
        if args.relat_path_list:
            fw.write(relat_img_path + "\n")
        else:
            fw.write(img_path + "\n")

    fw.close()


conv_mnist("t10k-images-idx3-ubyte", "t10k-labels-idx1-ubyte", 10000, img_path, relat_img_path, "mnist.valid.list", "v")
conv_mnist("train-images-idx3-ubyte", "train-labels-idx1-ubyte", 60000, img_path, relat_img_path, "mnist.train.list", "t")