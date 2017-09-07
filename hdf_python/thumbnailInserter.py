#!/usr/bin/python

"""Insert image as thumbnail into hdf5 file"""

import argparse
import base64
import math
import os
import sys
import xmltodict

try:
    import metadataReader as reader
except ImportError:
    from .hdf_python import metadataReader as reader

MAGIC_HDF = 0x89484446
MAGIC_HDF_STRING = '\x89\x48\x44\x46'

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


def check_hdf_header(file, location):
    """Check if at the specified position The header of a hdf5 file is found"""
    file.seek(location)
    sig = file.read(4)
    return int.from_bytes(sig, byteorder="big") == MAGIC_HDF


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
                of.write(bytearray(construct_xmp_file(
                    img, args.data), encoding="utf-8"))

            # Go to the next power of 2
            if not is_power2(of.tell()):
                of.seek(next_power2(of.tell()))

            # Write the HDF5 File
            for c in read_in_chunks(hf):
                of.write(c)


def write_into_sidecar(args):
    """Write the xmp into a sidecar file"""
    h5FileName = os.path.splitext(os.path.abspath(args.hdf5File))[0]
    path = h5FileName + ".xmp"
    if args.outfile != None:
        path = args.outfile

    with open(path, "w") as of:
        with open(args.imageFile, "rb") as img:
            of.write(construct_xmp_file(img, args.data)
                     .replace(reader.XMP_OUR_MAGIC, '').replace(reader.XMP_SIG_MAGIC, ''))


def get_updated_xmp(data, args):
    # If a imageFile is given replace the current thumbnail
    if args.imageFile is not None:
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
    xmp = ''
    xmp_start_byte = 0
    xmp_end_byte = 0
    with open(args.hdf5File, 'rb') as hf:
        xmp_start = False
        for line, pos in reader.file_split(hf, reader.XMP_SIG_MAGIC, bufsize=1024):
            if reader.XMP_OUR_MAGIC in line and not xmp_start:
                # Declare the start of the XMP Data as the end of this line
                xmp_start = True
                xmp_start_byte = pos

            if reader.XMP_FOOTER_MAGIC in line and xmp_start:
                xmp = line
                xmp_end_byte = xmp_start_byte + \
                    len(xmp) + len(reader.XMP_SIG_MAGIC)
                break

            if MAGIC_HDF_STRING in line:
                print("Error! No xmp found in userblock!")
                exit(-1)

        if len(xmp) == 0:
            print("Error! No xmp found in file")
            exit(-1)

        # Update the xmp
        xmp = get_updated_xmp(xmltodict.parse(xmp), args)

        # Add the signature lenght to the start
        xmp_start_byte += len(reader.XMP_SIG_MAGIC)
        hf.seek(0)
        with open(args.outfile, 'wb') as of:
            # Write everything before the xmp and the xmp to the userblock
            of.write(hf.read(xmp_start_byte))
            of.write(xmp.encode('utf-8'))
            hf.seek(0)

            # Determine the start of the hdf5 data in the inputfile
            fileSize = os.fstat(hf.fileno()).st_size
            hdfpos = 0
            while not check_hdf_header(hf, hdfpos) and hdfpos < fileSize:
                hdfpos = next_power2(hdfpos + 1)

            # If it doesn't find a hdf5 signature at the right place
            # It will just append the rest of the file
            if not check_hdf_header(hf, hdfpos):
                hf.seek(xmp_end_byte)
                print(
                    "No hdf5 signature found! Appending rest of file to updated version")
                for chunk in read_in_chunks(hf):
                    of.write(chunk)
                return

            # Write everything between the xmp end and signature start to the new file
            hf.seek(xmp_end_byte)
            after_xmp = hf.read(hdfpos - xmp_end_byte)
            i = 0
            while i < len(after_xmp) and after_xmp[::-1][i] == 0:
                i += 1
            hf.seek(xmp_end_byte)
            # Don't write the padding except for 4 bytes in case they are relevant to other stored information
            of.write(hf.read(len(after_xmp) - i + 4))
            hf.seek(hdfpos)

            of.seek(next_power2(of.tell()))
            for chunk in read_in_chunks(hf):
                of.write(chunk)


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
    parser.add_argument('--update', action='store_true',
                        help='update the xmp metadata')
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

    #  Checks if the imageFile exists
    if not args.update:
        if args.imageFile is None or not os.path.isfile(args.imageFile):
            if args.imageFile is None:
                print("Error imageFile argument missing")
            else:
                print("Error " + args.imageFile + " is not a file")
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
    else:
        if args.imageFile is not None and not os.path.isfile(args.imageFile):
            print("Error " + args.imageFile + " is not a file")
            exit(-1)

        if not args.sidecar:
            if args.outfile is None:
                print("Error! Outfile argument required when using without --sidecar")
                sys.exit(-1)
            update_hdf(args)
        else:
            update_sidecar(args)


if __name__ == "__main__":
    main()
