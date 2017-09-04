#!/usr/bin/env python
#
"""Test if the creation and reading of thumbnails is correct"""

import os
import json
import sys
import argparse
import filecmp
import io
from contextlib import redirect_stdout

import pytest

srcdir = os.path.dirname(__file__)
sys.path.insert(0, srcdir + '/..')

import thumbnailInserter as ins
import metadataReader as reader



def test_sidecar_data():
    ins_args = argparse.ArgumentParser()
    ins_args.hdf5File = srcdir + '/nothumb.hdf5'
    ins_args.imageFile = srcdir + '/testImage.png'
    ins_args.data = [['testField', 'testValue']]
    ins_args.outfile = srcdir + '/testoutput.xmp'
    ins.write_into_sidecar(ins_args)

    ext_args = argparse.ArgumentParser()
    ext_args.inputFile = srcdir + '/testoutput.xmp'
    ext_args.thumbnail = True
    ext_args.outputFile = srcdir + '/testoutput.png'

    # Check if the extracted thumbnail is correct
    reader.main(ext_args)

    assert filecmp.cmp(srcdir + '/testImage.png',
                       srcdir + '/testoutput.png')

    # Check if it correctly extracts metadata
    ext_args.thumbnail = False
    f = io.StringIO()
    with redirect_stdout(f):
        reader.main(ext_args)

    assert json.loads(f.getvalue())['testField'] == 'testValue'


def test_userblock_data():
    ins_args = argparse.ArgumentParser()
    ins_args.hdf5File = srcdir + '/nothumb.hdf5'
    ins_args.imageFile = srcdir + '/testImage.png'
    ins_args.data = [['testField', 'testValue']]
    ins_args.outfile = srcdir + '/testoutput.hdf5'
    ins.write_into_userblock(ins_args)

    ext_args = argparse.ArgumentParser()
    ext_args.inputFile = srcdir + '/testoutput.hdf5'
    ext_args.thumbnail = True
    ext_args.outputFile = srcdir + '/testoutput.png'

    # Check if the extracted thumbnail is correct
    reader.main(ext_args)

    assert filecmp.cmp(srcdir + '/testImage.png',
                       srcdir + '/testoutput.png')

    # Check if it correctly extracts metadata
    ext_args.thumbnail = False
    f = io.StringIO()
    with redirect_stdout(f):
        reader.main(ext_args)

    assert json.loads(f.getvalue())['testField'] == 'testValue'


if __name__ == '__main__':
    sys.exit(pytest.main([os.path.join(srcdir, 'hdf_tests.py')]))
