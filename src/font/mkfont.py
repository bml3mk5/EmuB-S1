#!/usr/bin/python
# S1エミュレータのフォントファイルを作成
#
# Copyright (c) 2020, Sasaji

import struct

infile="display_font.bmp"
outfile="S1FONT.ROM"

print(f'-------- {infile}')

## @param[in] data: input file
def check_bmp_header(data: bytes) -> int:
    global offset

    # file header 14 bytes
    if data.find(b'BM', 0, 2) != 0:
        print('This is not BMP format.')
        return 1
    
    offset = struct.unpack('<I', data[0x0a:0x0e])
    print(f'offset: {offset[0]}')

    # info header
    infosize = struct.unpack('<I', data[0x0e:0x12])
    if infosize[0] != 40:
        print('Windows BMP format only.')
        return 1
    
    width = struct.unpack('<I', data[0x12:0x16])
    if width[0] != 256:
        print('width must be 256 pixel.')
        return 1
    
    height = struct.unpack('<I', data[0x16:0x1a])
    if height[0] != 256:
        print('height must be 128 pixel.')
        return 1
    
    bpp = struct.unpack('<H', data[0x1c:0x1e])
    if bpp[0] != 1:
        print('Supported data is only 1bit per pixel(B/W data).')
        return 1
    
    compress = struct.unpack('<I', data[0x1e:0x22])
    if compress[0] != 0:
        print('Supported data is only no compression.')
        return 1

    return 0

## main

with open(infile, "rb") as f:
    indata = f.read()

offset = [0]
if check_bmp_header(indata):
    raise Exception('Invalid format in the input file.')

outdatas = dict()

chcode = 0
while chcode < 0x200:
    outdatas[chcode] = bytes()
    x = chcode % 32
    y = int((0x1ff - chcode) / 32)
    print('{0:3X}: X:{1:3d} Y:{2:3d}'.format(chcode, x, y), end = '')
    yy = 0
    while yy < 16:
        pos = y * 32 * 16 + (15 - yy) * 32 + x + offset[0]
        val = indata[pos]
        print(' {0:2X}'.format(val), end = '')
        outdatas[chcode] += struct.pack('B', val)
        yy += 1
    print()
    chcode += 1

outdata = bytes()
chcode = 0
while chcode < 0x200:
    x = 0
    while x < 16:
        outdata += struct.pack('B', outdatas[chcode][x])
        x += 1
    chcode += 1

with open(outfile, "wb") as f:
    f.write(outdata)

input('Complete.')


