#!/usr/bin/python

"""Insert image as thumbnail into hdf5 file"""

import sys
import os
import argparse
import math
import base64

MAGIC_HDF = 0x89484446

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

def construct_xmp_file(imageFile, key_value_data):
    # Remove the quotes and leading b from the string
    base64Image = str(base64.b64encode(imageFile.read())).strip("b'").strip("'")
    # Get the xmp template
    path = os.path.dirname(os.path.abspath(__file__)) + "/template.xmp"

    with open(path, "r") as xmpFile:
        xmp = xmpFile.read()

        extension = os.path.splitext(os.path.abspath(imageFile.name))[1]
        extension = extension.split(".")[-1]
        xmp = xmp.replace("{{IMAGE_FORMAT}}", extension.upper())

        # Insert the data
        xmp = xmp.replace("{{IMAGE_DATA_BASE64}}", base64Image)

        # Insert the Key_Value pairs if they are given
        if key_value_data is not None:
            key_value_string = ''
            for pair in key_value_data:
                key_value_string += '<xap:' + pair[0] + '>' + pair[1] \
                    + '</xap:' + pair[0] + '>'
            
            xmp = xmp.replace('{{CUSTOM_KEY_VALUES}}', key_value_string)
        else:
            xmp = xmp.replace('{{CUSTOM_KEY_VALUES}}', '');

        return xmp



def write_into_userblock(args):

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

        with open(args.outfile, "wb") as of:
            of.write(hf.read(i))
            with open(args.imageFile, "rb") as img:
                of.write(bytearray(construct_xmp_file(img, args.data), encoding="utf-8"))

            # Go to the next power of 2
            if not is_power2(of.tell()):
                of.seek(next_power2(of.tell()))

            # Write the HDF5 File
            for c in read_in_chunks(hf):
                of.write(c)

def write_into_sidecar(args):
    h5FileName = os.path.splitext(os.path.abspath(args.hdf5File))[0]
    path = h5FileName + ".xmp"
    if args.outfile != None:
        path = args.outfile

    with open(path, "w") as of:
        with open(args.imageFile, "rb") as img:
            of.write(construct_xmp_file(img, args.data))


def main():

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('hdf5File',
                        help='filename to insert the image into')
    parser.add_argument('imageFile',
                        help='filename of the image to insert')
    parser.add_argument('outfile', nargs='?',
                        help='filename of the output file. Only required when using without --sidecar')
    parser.add_argument('-d', '--data', action='append', nargs=2,
                        help='custom user-defined Key-Value pairs')
    parser.add_argument('--sidecar', action='store_true')
    args = parser.parse_args()

    #  Checks if the hdf5File exists
    if not os.path.isfile(args.hdf5File):
        print("Error " + args.hdf5File + " is not a file")
        sys.exit(-1)

    #  Checks if the imageFile exists
    if not os.path.isfile(args.imageFile):
        print("Error " + args.imageFile + " is not a file")
        sys.exit(-1)

    #  Makes sure that both files aren't the same
    if args.hdf5File == args.imageFile:
        print("Error! Both files can't be the same")
        sys.exit(-1)

    print(args.data)

    if not args.sidecar:
        if args.outfile is None:
            print("Error! Outfile argument required when using without --sidecar")
            sys.exit(-1)
        write_into_userblock(args)
    else:
        write_into_sidecar(args)


if __name__ == "__main__":
    main()
