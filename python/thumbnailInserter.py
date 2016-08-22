#!/usr/bin/python

import sys
import os
import os.path
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
        imgf = open(imageFile, 'r')

        #  Creates a new dataSet
        dset = hf.create_dataset("thumb", (100,))

        #  Makes the image data ready to be stored
        #  data = os.path.splitext(imageFile)[1][1:].strip() + ":"
        data = base64.b64encode(imgf.read())
        #  The data will look like this "fileextension:base64Data"

        #  Stores the image base64 encoded as an attribute
        dset.attrs["image"] = data

        hf.close()
        imgf.close()
