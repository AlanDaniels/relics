#!/usr/bin/env python

# This script takes a skybox, in the traditional "sideways T" layout, and
# extracts out the six images we'd want to load in an OpenGL cube map. This
# requires Python 3, and the Python Imaging Library. To install it, just run
# "pip install pillow" from the command line.
# -- Alan Daniels, Dec 25th, 2016.


import os.path, sys
from PIL import Image


# For now, only work with 4-by-3 skymaps,
# where it's some multiple of 512, 1024, or 2048.
valid_sizes = [
    ( 512 * 4,  512 * 3),
    (1024 * 4, 1024 * 3),
    (2048 * 4, 2048 * 3)]


def get_face_fname(rel_fname, suffix):
    '''Get the name of one of the face files we'll create.'''
    full_fname = os.path.abspath(rel_fname)
    (dir_name, plain_fname) = os.path.split(full_fname)
    (front, extn)  = os.path.splitext(plain_fname)
    return os.path.join(dir_name, front + '_' + suffix + extn)


def save_crop(orig, x, y, fname):
    '''Crop out one of our blocks, and save it to its own image.'''
    block = orig.size[1] // 3
    dims = (x * block, y * block, (x + 1) * block, (y + 1) * block)
    orig.crop(dims).save(fname)
    print('Wrote {0}'.format(fname))


def main(fname):
    '''And away we go.'''
    orig = Image.open(fname)
    if orig.size not in valid_sizes:
        raise IOError(
            'Error with skymap "{0}", not a valid image size ({1})'.format(fname, orig.size))
    face_fnames = {
        'N': get_face_fname(fname, 'N'),
        'S': get_face_fname(fname, 'S'),
        'E': get_face_fname(fname, 'E'),
        'W': get_face_fname(fname, 'W'),
        'T': get_face_fname(fname, 'T'),
        'B': get_face_fname(fname, 'B')}
    for fname in face_fnames.values():
        if os.path.isfile(fname):
            raise IOError('File {0} already exists.'.format(fname))
    save_crop(orig, 1, 0, face_fnames['T'])
    save_crop(orig, 0, 1, face_fnames['W'])
    save_crop(orig, 1, 1, face_fnames['N'])
    save_crop(orig, 2, 1, face_fnames['E'])
    save_crop(orig, 3, 1, face_fnames['S'])
    save_crop(orig, 1, 2, face_fnames['B'])
    print('All done.')


if __name__ == '__main__':
    try:
        fname = sys.argv[1]
        main(fname)
    except Exception as ex:
        print('Usage: chop_sky_map.py [image]')
        print('The error was "{0}"'.format(ex))
        sys.exit(1)

