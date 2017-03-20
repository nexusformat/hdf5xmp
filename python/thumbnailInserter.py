#!/usr/bin/python

"""Insert image as thumbnail into hdf5 file"""

import sys
import struct
import os
import argparse


def read_in_chunks(file, chunk_size=1024):
    while True:
        data = file.read(chunk_size)
        if not data:
            break
        yield data

def is_power2(num):
    return ((num & (num - 1)) == 0) and num != 0

def next_power2(num):
    i = 1
    while i < num:
        i *= 2
    return i


def main():

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('hdf5File',
                        help='filename to insert the image into')
    parser.add_argument('imageFile',
                        help='filename of the image to insert')
    parser.add_argument('outfile',
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

    outfile = args.outfile

    #  Open the hdf5 file
    with open(outfile, "wb") as of:

        img = open(args.imageFile, "rb")

        imageSize = os.fstat(img.fileno()).st_size

        # Write the size of the image into the file
        of.write(struct.pack('>I', int(imageSize)))

        # Write the image in chunks
        for c in read_in_chunks(img):
            of.write(c)

        currentSize = imageSize + 4

        # Align the bytes for the hdf5
        if not is_power2(currentSize):
            of.seek(next_power2(currentSize) - 1)
            of.write(struct.pack('b', 0))

        h5 = args.hdf5File
        hf = open(h5, "rb")
        for c in read_in_chunks(hf):
            of.write(c)
            
        # close files
        hf.close()
        img.close()
        of.close()


if __name__ == "__main__":
    main()
