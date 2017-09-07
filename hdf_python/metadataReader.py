#!/usr/bin/env python

"""
Extract the metadata and Thumbnail out of a hdf5 file or xmp sidecar file
Metadata is outputted as a JSON format
The thumbnail has to be written to a file
"""

import argparse
import base64
import json
import os
import sys

import xmltodict

try:
    import thumbnailInserter as inserter
except ImportError:
    from hdf_python import thumbnailInserter as inserter


def try_sidecar(args):
    file_name_data = os.path.splitext(
        os.path.abspath(args.inputFile))
    if file_name_data[-1] in '.xmp':
        xmp_file = args.inputFile
    else:
        xmp_file = file_name_data[0] + ".xmp"

    # If the file doesn't exist print a warning and exit
    if not os.path.isfile(xmp_file):
        print('Warning: No thumbnail or Sidecar found')
        exit(-2)

    with open(xmp_file, 'r') as xf:
        # Remove the xmp headers and our signatures
        return xf.read()


def extract_xmp(args):
    # If xmp is given directly don't read it twice
    if '.xmp' in os.path.splitext(
            os.path.abspath(args.inputFile))[-1]:
        return try_sidecar(args)

    with open(args.inputFile, 'rb') as hf:

        # Check if the file is a HDF5-File
        fileSize = os.fstat(hf.fileno()).st_size
        hdf_pos = 0
        while not inserter.check_header(hf, hdf_pos, inserter.MAGIC_HDF) and hdf_pos < fileSize:
            hdf_pos = inserter.next_power2(hdf_pos + 1)

        if not inserter.check_header(hf, hdf_pos, inserter.MAGIC_HDF):
            return try_sidecar(args)

        xmp_search_pos = 0
        hf.seek(0)

        while not inserter.check_header(hf, xmp_search_pos, inserter.XMP_OUR_MAGIC) and \
                not inserter.check_header(hf, xmp_search_pos, inserter.MAGIC_HDF) and \
                xmp_search_pos < fileSize:
            data_size = inserter.read_datablock_size(hf, xmp_search_pos)
            # If the data size is 0 then padding is found
            if not inserter.exists_header(hf, xmp_search_pos):
                break
            # Advance to the next datablock
            hf.seek(hf.tell() + data_size + 8)
            xmp_search_pos = hf.tell()

        # If an XMP is already there just update it
        if inserter.check_header(hf, xmp_search_pos, inserter.XMP_OUR_MAGIC):
            xmp_size = inserter.read_datablock_size(hf, xmp_search_pos)
            return hf.read(xmp_size)

    return try_sidecar(args)


def read_desc(xmp_desc, args):
    if not args.thumbnail:
        # Extract the key/values pairs starting with 'xap:'
        # Remove the 'xap:' in front of it
        result = {k.replace('xap:', ''): v for k, v in xmp_desc.items()
                  if 'xap:' in k and 'xap:Thumbnails' not in k}
        return result
    else:
        thumb_data = xmp_desc['xap:Thumbnails']['rdf:Alt']['rdf:li']
        # If there are multiple thumbnails return the first one
        if isinstance(thumb_data, list):
            return thumb_data[0]['xapGImg:image']
        return thumb_data['xapGImg:image']


def read_data(data, args):
    xmp_desc = xmltodict.parse(data)['x:xmpmeta']['rdf:RDF']['rdf:Description']

    if isinstance(xmp_desc, list):
        result = {}
        for desc in xmp_desc:
            result.update(read_desc(desc, args))
            if args.thumbnail:
                return result

        return result
    else:
        return read_desc(xmp_desc, args)


def main(args):
    if not os.path.isfile(args.inputFile):
        print("Error " + args.inputFile + " is not a file")
        sys.exit(-1)

    if args.thumbnail:
        if args.outputFile is None:
            print('Error! OutputFile argument required when using with --thumbnail')
            sys.exit(-1)

    xml_data = extract_xmp(args)
    if args.thumbnail:
        with open(args.outputFile, 'wb') as of:
            b64string = read_data(xml_data, args)
            of.write(base64.b64decode(b64string))
    else:
        json.dump(read_data(xml_data, args), sys.stdout)


def __hdf_internal_main():
    args_parser = argparse.ArgumentParser(description=__doc__)
    args_parser.add_argument('inputFile',
                             help='file to extract metadata from')
    args_parser.add_argument('outputFile', nargs='?',
                             help='file to write the output to. If not extracting a thumbnail this can be ommited and the output is instead directed to stdout')
    args_parser.add_argument('--thumbnail', action='store_true',
                             help='Pass this if you want the thumbnail only')
    args = args_parser.parse_args()
    main(args)


if __name__ == '__main__':
    __hdf_internal_main()
