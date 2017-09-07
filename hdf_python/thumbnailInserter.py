#!/usr/bin/python

"""Insert image as thumbnail into hdf5 file"""

import argparse
import struct
import base64
import math
import os
import sys
import xmltodict

try:
    import metadataReader as reader
except ImportError:
    from .hdf_python import metadataReader as reader

MAGIC_HDF = 0x894844460d0a1a0a
XMP_OUR_MAGIC = 0x89484D500d0a1a0a

XMP_HEADER = '<?xpacket begin="" id="W5M0MpCehiHzreSzNTczkc9d"?>\n'
XMP_FOOTER = '\n<?xpacket end="w"?>'


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


def check_image_files(args):
    #  Checks if the imageFile exists
    if args.imageFile is None or not os.path.isfile(args.imageFile):
        if args.imageFile is None:
            print("Error imageFile argument missing")
        else:
            print("Error " + args.imageFile + " is not a file")
            sys.exit(-1)


def check_header(file, location, header):
    """Check if at the specified position The header of a hdf5 file is found"""
    file.seek(location)
    sig = file.read(8)
    return int.from_bytes(sig, byteorder="big") == header


def exists_header(file, location):
    """Check if at the specified position is a valid header"""
    file.seek(location)
    sig = file.read(8)
    # Compare but ignore the two changeable byts
    return int.from_bytes(sig, byteorder="big") & 0xffff0000ffffffff == MAGIC_HDF & 0xffff0000ffffffff


def read_datablock_size(file, location):
    """Return the size of the datablock starting at location"""
    # Advance 8 bytes to skip signature
    file.seek(location + 8)
    # Return the 8 byte long size
    return int.from_bytes(file.read(8), byteorder="big")


def construct_xmp_file(imageFile, key_value_data):
    """Construct the xmp file based on the imageFile and key_value_data input"""
    # Remove the quotes and leading b from the string
    base64Image = base64.b64encode(imageFile.read()).decode('utf-8')
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
            xmp = xmp.replace('{{CUSTOM_KEY_VALUES}}', '')

        return xmp


def write_into_userblock(args):
    """Write the xmp into the userblock"""
    with open(args.hdf5File, "rb") as hf:

        # Check if the file is a HDF5-File
        fileSize = os.fstat(hf.fileno()).st_size
        hdf_pos = 0
        while not check_header(hf, hdf_pos, MAGIC_HDF) and hdf_pos < fileSize:
            hdf_pos = next_power2(hdf_pos + 1)

        if not check_header(hf, hdf_pos, MAGIC_HDF):
            print("Not a HDF5 File")
            exit(-1)

        xmp_search_pos = 0
        hf.seek(0)

        more = True
        # Find out if there is already a xmp block present
        while not check_header(hf, xmp_search_pos, XMP_OUR_MAGIC) and \
                not check_header(hf, xmp_search_pos, MAGIC_HDF) and \
                xmp_search_pos < fileSize:
            data_size = read_datablock_size(hf, xmp_search_pos)
            # If the data size is 0 then padding is found
            if not exists_header(hf, xmp_search_pos):
                more = False
                break
            # Advance to the next datablock
            hf.seek(hf.tell() + data_size + 8)
            xmp_search_pos = hf.tell()

        xmp = ''

        # If an XMP is already there just update it
        if check_header(hf, xmp_search_pos, XMP_OUR_MAGIC):
            xmp_size = read_datablock_size(hf, xmp_search_pos)
            xmp = get_updated_xmp(xmltodict.parse(
                hf.read(xmp_size)), args).encode('utf-8')
        # If there isn't already an xmp create a new one. A image parameter is required for this
        else:
            check_image_files(args)
            with open(args.imageFile, "rb") as img:
                xmp = bytearray(construct_xmp_file(
                    img, args.data), encoding="utf-8")

        with open(args.outfile, "wb") as of:
            hf.seek(0)
            # Write everything before the xmp block from the userblock
            # If there isn't a xmp block write the entire userblock instead
            of.write(hf.read(xmp_search_pos))

            # Add the headers to the xmp
            of.write(struct.pack('!Q', XMP_OUR_MAGIC))
            of.write(struct.pack('!Q', len(xmp)))
            of.write(xmp)

            # If there is more data in the userblock after the xmp
            if more:
                search_pos = xmp_search_pos
                # As long as it still finds a datablock header
                while not exists_header(hf, search_pos) and not check_header(hf, search_pos, MAGIC_HDF) and search_pos < fileSize:
                    data_size = read_datablock_size(hf, search_pos)

                    # If we find another xmp block remove it. There should be only one block of xmp
                    if check_header(hf, search_pos, XMP_OUR_MAGIC):
                        # 8 Bytes = We are in the middle of the 16 byte long header
                        hf.seek(hf.tell() + data_size + 8)
                    else:
                        hf.seek(search_pos)
                        # We are at the start of the header so we advance 16 bytes
                        of.write(hf.read(data_size + 16))
                    search_pos = hf.tell()

            # Go to the next power of 2
            if not is_power2(of.tell()):
                of.seek(next_power2(of.tell()))

            # Write the HDF5 File
            hf.seek(hdf_pos)
            for c in read_in_chunks(hf):
                of.write(c)


def write_into_sidecar(args):
    """Write the xmp into a sidecar file"""
    h5FileName = os.path.splitext(os.path.abspath(args.hdf5File))[0]
    path = h5FileName + ".xmp"
    if args.outfile != None:
        path = args.outfile

    if os.path.isfile(path):
        return update_sidecar(args)

    check_image_files(args)
    with open(path, "w") as of:
        with open(args.imageFile, "rb") as img:
            of.write(construct_xmp_file(img, args.data))


def get_updated_xmp(data, args):
    # If a imageFile is given replace the current thumbnail
    if args.imageFile is not None:
        check_image_files(args)
        with open(args.imageFile, 'rb') as img:
            base64image = base64.b64encode(img.read()).decode('utf-8')
            data['x:xmpmeta']['rdf:RDF']['rdf:Description']['xap:Thumbnails']['rdf:Alt']['rdf:li']['xapGImg:image'] = base64image

    if args.data is not None:
        for kv_pair in args.data:
            # Ability to remove metadata
            if kv_pair[1] == 'None' and 'xap:' + kv_pair[0] in data['x:xmpmeta']['rdf:RDF']['rdf:Description']:
                del data['x:xmpmeta']['rdf:RDF']['rdf:Description']['xap:' +
                                                                    kv_pair[0]]
            else:
                data['x:xmpmeta']['rdf:RDF']['rdf:Description']['xap:' +
                                                                kv_pair[0]] = kv_pair[1]
    return XMP_HEADER + xmltodict.unparse(data,
                                          pretty=True, full_document=False) + XMP_FOOTER


def update_sidecar(args):
    h5FileName = os.path.splitext(os.path.abspath(args.hdf5File))[0]
    path = h5FileName + ".xmp"
    with open(path, 'r+') as xf:
        data = xmltodict.parse(xf.read())
        if args.outfile is not None and args.outfile != path:
            with open(args.outfile, 'w') as of:
                of.write(get_updated_xmp(data, args))
        else:
            xf.seek(0)
            xf.write(get_updated_xmp(data, args))
            xf.truncate()


def update_hdf(args):
    print('Deprecated')
    exit(-1)


def main():

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('hdf5File',
                        help='filename to insert the image into')
    parser.add_argument('--img', '--imageFile', nargs='?',
                        help='filename of the image to insert')
    parser.add_argument('--outfile', '-o', nargs='?',
                        help='filename of the output file. Only required when using without --sidecar')
    parser.add_argument('-d', '--data', action='append', nargs=2,
                        help='custom user-defined Key-Value pairs')

    parser.add_argument('--sidecar', action='store_true')
    args = parser.parse_args()
    args.imageFile = args.img

    # Ensure that no user defined key value pairs have the same key
    if args.data is not None:
        for k in args.data:
            matches = 0
            for v in args.data:
                if k[0] == v[0]:
                    matches += 1
            if matches > 1:
                print("Error metadata names must be unique")
                exit(-1)

    #  Checks if the hdf5File exists
    if not os.path.isfile(args.hdf5File):
        print("Error " + args.hdf5File + " is not a file")
        sys.exit(-1)

    #  Makes sure that both files aren't the same
    if args.hdf5File == args.imageFile:
        print("Error! HDF5-File can't be the imageFile at the same time")
        sys.exit(-1)

    if not args.sidecar:
        if args.outfile is None:
            print("Error! Outfile argument required when using without --sidecar")
            sys.exit(-1)
        write_into_userblock(args)
    else:
        write_into_sidecar(args)


if __name__ == "__main__":
    main()
