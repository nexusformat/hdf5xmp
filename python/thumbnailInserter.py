#!/usr/bin/python

import sys
import os
import os.path
import numpy as np
import base64
import h5py

if __name__ == "__main__":

    #  Checks if the correct number of arguments is provided
    if(len(sys.argv) == 2):
        print("Error! Invalid number of arguments provided")
        sys.exit()
    #  Get the current path
    dir_path = os.getcwd()

    #  Get the file to insert the Image into
    if(os.path.isabs(sys.argv[1])):
        file = sys.argv[1]
    else:
        file = dir_path + "/" + sys.argv[1]

    #  Get the imageFile
    if(os.path.isabs(sys.argv[2])):
        imageFile = sys.argv[2]
    else:
        imageFile = dir_path + "/" + sys.argv[2]

    #  Checks if the file exists
    if(not os.path.isfile(file)):
        print("Error " + str(file) + " is not a file")
        sys.exit()

    #  Checks if the imageFile exists
    if(not os.path.isfile(imageFile)):
        print("Error " + str(imageFile) + " is not a file")
        sys.exit()

    #  Makes sure that both files aren't the same
    if(file == imageFile):
        print("Error! Both files can't be the same")
        sys.exit()

    #  Open the hdf5 file
    with h5py.File(file, 'a') as hf:
        imgf = open(imageFile, 'rb')

        binary_image = imgf.read()
        #  Creates a new dataSet
        dset = hf.create_dataset("thumb", data=base64.b64encode(binary_image))

        hf.close()
        imgf.close()
