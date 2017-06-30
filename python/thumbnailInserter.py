#!/usr/bin/python

"""Insert image as thumbnail into hdf5 file"""

import sys
import struct
import os
import argparse
import math

MAGIC_HDF = 0x89484446

# 00000000: 8954 484d 4235      .THMB5
MAGIC_THUMB = [0x8954, 0x484d, 0x4235]


def read_in_chunks(file, chunk_size=1024):
    while True:
        data = file.read(chunk_size)
        if not data:
            break
        yield data


def is_power2(num):
    return ((num & (num - 1)) == 0) and num != 0


def next_power2(num):
    return 2**(math.ceil(math.log(num, 2)))


def check_hdf_header(file, location):
    file.seek(location)
    sig = file.read(4)
    return int.from_bytes(sig, byteorder="big") == MAGIC_HDF


def write_signature(file):
    for magic_byte in MAGIC_THUMB:
        file.write(struct.pack(">H", magic_byte))


def main():

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('hdf5File',
                        help='filename to insert the image into')
    parser.add_argument('imageFile',
                        help='filename of the image to insert')
    parser.add_argument('outfile',
                        help='filename of the output where the hdf5 with thumbnail should be saved')
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

    with open(args.hdf5File, "rb") as hf:

        # Check if the file is a HDF5-File
        fileSize = os.fstat(hf.fileno()).st_size
        i = 0
        while not check_hdf_header(hf, i) and i < fileSize:
            i = next_power2(i + 1)

        if not check_hdf_header(hf, i):
            print("Not a HDF5 File")
            exit(-1)

        hf.seek(0)

        # If the hdf5 file has no user block
        if i == 0:
            of = open(args.outfile, "wb")
            # Write the thumbnail signature
            write_signature(of)

            # Write the imagesize
            img = open(args.imageFile, "rb")
            imageSize = os.fstat(img.fileno()).st_size
            of.write(struct.pack(">I", int(imageSize)))

            # Write the image chunkwise
            for c in read_in_chunks(img):
                of.write(c)

            img.close()

            # Go to the next power of 2
            if not is_power2(of.tell()):
                of.seek(next_power2(of.tell()))

            # Write the HDF5 File
            for c in read_in_chunks(hf):
                of.write(c)

            of.close()

        hf.close()

    #  Open the hdf5 file
#    with open(outfile, "wb") as of:

#        img = open(args.imageFile, "rb")

#        imageSize = os.fstat(img.fileno()).st_size

        # Write the size of the image into the file
#        of.write(struct.pack('>I', int(imageSize)))

        # Write the image in chunks
#        for c in read_in_chunks(img):
#            of.write(c)

#        currentSize = imageSize + 4

        # Align the bytes for the hdf5
#        if not is_power2(currentSize):
#            of.seek(next_power2(currentSize) - 1)
#            of.write(struct.pack('b', 0))

#        h5 = args.hdf5File
#        hf = open(h5, "rb")
#        for c in read_in_chunks(hf):
#            of.write(c)
            
#        # close files
#        hf.close()
#        img.close()
#        of.close()


if __name__ == "__main__":
    main()
