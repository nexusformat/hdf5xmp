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
import re
import sys

import xmltodict

try:
    import thumbnailInserter as inserter
except ImportError:
    from hdf_python import thumbnailInserter as inserter

XMP_OUR_MAGIC = "MTDXMP%"
XMP_SIG_MAGIC = "SIGXMP%"
XMP_FOOTER_MAGIC = "<?xpacket"

# The magic number of XMP files
MAGIC_NUMBER_XMP = "W5M0MpCehiHzreSzNTczkc9d"

# Regex used to remove the HDF5 Header and footer
MAGIC_XMP_HEADER_REGEX = "(<\\?xpacket begin=(\"|')(.?.?.?)(\"|') \
id=(\"|')W5M0MpCehiHzreSzNTczkc9d(\"|')\\?>)|\
(<\\?xpacket end=(\"|')(w|r)(\"|')\\?>)"


def file_split(f, delim='>', bufsize=64):
    prev = ''
    first_found = False
    while True:
        s = f.read(bufsize).decode('utf-8', 'replace')

        if not s or f.tell() == os.fstat(f.fileno()).st_size:
            break

        split = s.split(delim)

        if len(split) > 1:
            first_found = True
            yield prev + split[0], f.tell() + len(split[0]) - bufsize
            prev = split[-1]
            for x in split[1:-1]:
                yield x, f.tell() + len(split[0]) - bufsize
        else:
            prev += s
            # Move half the buffer forward to prevent the problem of having half the signature cut off
            if first_found:
                prev = prev[0:-(bufsize // 2)]
            f.seek(f.tell() - bufsize // 2)
    if prev:
        yield prev, 0


def remove_xmp_header(xmp):
    return re.sub(MAGIC_XMP_HEADER_REGEX, '', xmp)


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
        return remove_xmp_header(xf.read())\
            .replace(XMP_OUR_MAGIC, '').replace(XMP_SIG_MAGIC, '')


def extract_xmp(args):
    # If xmp is given directly don't read it twice
    if '.xmp' in os.path.splitext(
            os.path.abspath(args.inputFile))[-1]:
        return try_sidecar(args)

    with open(args.inputFile, 'rb') as inFile:
        # If the HDF5 header is found the file doesn't contain a thumbnail
        # If this happens a sidecar is tried
        if inserter.check_hdf_header(inFile, 0):
            return try_sidecar(args)

        inFile.seek(0)

        # Find the beginning of the XMP Data
        xmp_start = False
        for line, p in file_split(inFile, XMP_SIG_MAGIC, bufsize=1024):
            if XMP_OUR_MAGIC in line and not xmp_start:
                # Declare the start of the XMP Data as the end of this line
                xmp_start = True

            if XMP_FOOTER_MAGIC in line and xmp_start:
                return remove_xmp_header(line)

            if inserter.MAGIC_HDF_STRING in line:
                return try_sidecar(args)

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
            print(b64string)
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
