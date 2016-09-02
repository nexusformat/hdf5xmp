#!/usr/bin/python

"""Insert image as thumbnail into hdf5 file"""

import sys
import os
import h5py
import argparse


def main():

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('hdf5File',
                        help='filename to insert the image into')
    parser.add_argument('imageFile',
                        help='filename of the image to insert')
    args = parser.parse_args()

    #  Checks if the hdf5File exists
    if(not os.path.isfile(args.hdf5File)):
        print("Error " + args.hdf5File + " is not a file")
        sys.exit()

    #  Checks if the imageFile exists
    if(not os.path.isfile(args.imageFile)):
        print("Error " + args.imageFile + " is not a file")
        sys.exit()

    #  Makes sure that both files aren't the same
    if(args.hdf5File == args.imageFile):
        print("Error! Both files can't be the same")
        sys.exit()

    #  Open the hdf5 file
    with h5py.File(args.hdf5File, 'a') as hf:
        imgf = open(args.imageFile, 'rb')

        #  Creates a new dataSet
        hf.create_dataset("thumb", data=bytearray(imgf.read()))

        # close files
        hf.close()
        imgf.close()


if __name__ == "__main__":
    main()
