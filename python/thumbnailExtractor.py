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
    imageFile = sys.argv[2]

    #  Checks if the file exists
    if(not os.path.isfile(file)):
        print("Error " + str(file) + " is not a file")
        sys.exit()

    #  Makes sure that both files aren't the same
    if(file == imageFile):
        print("Error! Both files can't be the same")
        sys.exit()

    #  Open the hdf5 file
    with h5py.File(file, 'r') as hf:
        imgf = open(imageFile, 'w+')

        #  Creates a new dataSet
        dset = hf["thumb"]

        data = dset.attrs.get("image")

        imgf.write(base64.b64decode(data))

        hf.close()
        imgf.close()
