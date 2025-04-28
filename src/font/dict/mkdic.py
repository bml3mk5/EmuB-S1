#!/usr/bin/python
#
# Make Kanji Dictionary for MB-S1
#
# Programmed by Sasaji 2025
#

import struct
import re

#

def sjis_to_jis(sjis_bytes:bytes) -> bytes:
    sjis = int.from_bytes(sjis_bytes, 'big')
    h = (sjis >> 8)
    l = (sjis & 0xff)
#   print(sjis_bytes, hex(sjis))

    if not ((0x81 <= h and h <= 0x9f) or (0xe0 <= h and h <= 0xff)):
        # 1byte
        return sjis.to_bytes(2, 'big')

    # convert the first code
    if h <= 0x9f:
        if l < 0x9f:
            h = (h << 1) - 0xe1
        else:
            h = (h << 1) - 0xe0
    else:
        if l < 0x9f:
            h = (h << 1) - 0x161
        else:
            h = (h << 1) - 0x160

    if l < 0x7f:
        l = l - 0x1f
    elif l < 0x9f:
        l = l - 0x20
    else:
        l = l - 0x7e

    jis = (h << 8 | l)
    return jis.to_bytes(2, 'big')

## main

infile = 's1dic.txt'
outfile = 'S1DIC.ROM'

try:
    # open file
    fhi = open(infile, "rb")

    # make index
    dhash = dict()
    for ch in range(0xb1, 0xde):
        dhash[ch]={'ch':ch.to_bytes(1, 'big'), 'count':0, 'data':list()}

    ch = 0xa6
    dhash[ch]={'ch':ch.to_bytes(1, 'big'), 'count':0, 'data':list()}

    #for key in dhash:
    #   print(f"{dhash[key]}")

    threesk = list()
    phase = 0
    line_num = 0
    for line in fhi:
        line = line.rstrip(b'\n')
        line = line.rstrip(b'\r')
        line_num += 1

        if re.match(rb"^$", line):
            phase += 1
            if phase < 2:
                continue
            else:
                break

        if phase == 0:
            # phase 0 : 単漢字辞書
            word = b""
            kanji = b""
            m = re.match(rb"^(.+):(.+)$", line)
            if m:
                word = m.group(1)
                kanji = m.group(2)
            else:
                # no data ??
                raise ValueError(f"Cannot parse or invalid charactor exists in line {line_num}")

            ch = word[0]
            if not ch in dhash:
                raise ValueError(f"First charactor is invalid in line {line_num}")

            dhash[ch]['count'] += 1
            dhash[ch]['data'].append({'word':word, 'kanji':kanji})
            
        elif phase == 1:
            # phase 1 : 3ストローク辞書
            word = b""
            kanji = b""
            m = re.match(rb"^(.{3}):(.+)", line)
            if m:
                word = m.group(1)
                kanji = m.group(2)
            else:
                # no data ??
                raise ValueError(f"Cannot parse or invalid charactor exists in line {line_num}")

            threesk.append({'word':word, 'kanji':kanji})

    ## ｱｲｳｴｵ .... ﾜｦﾝ
    aindex = list()
    for ch in range(0xb1, 0xdd):
        aindex.append(dhash[ch])

    ch = 0xa6
    aindex.append(dhash[ch])
    ch = 0xdd
    aindex.append(dhash[ch])

    # calculate length
    for item in aindex:
        leng = 0
        for data in item['data']:
            leng += len(data['word']) + len(data['kanji']) + 1
    #       print(f"{data['word']} {data['kanji']} len:{leng}")
        item['length'] = leng

    # output
    fho = open(outfile, "wb")

    # index
    addr = 0x008b
    for item in aindex:
        fho.write(item['ch'])
        fho.write(struct.pack('B', addr >> 8))
        fho.write(struct.pack('B', addr & 0xff))
        addr += item['length']

    fho.write(bytes(1))

    # tankanji henkan data
    for item in aindex:
        for data in item['data']:
            leng = len(data['word']) + len(data['kanji']) + 1
            fho.write(struct.pack('B', leng & 0xff))
            if len(data['word']) > 1:
                fho.write(data['word'][1:])
            fho.write(bytes(1))
            for i in range(0, len(data['kanji']), 2):
                fho.write(sjis_to_jis(data['kanji'][i:i+2]))

    if addr >= 0x7000:
        raise ValueError("Tankanji dictionary size is overflow. The size should be less than 0x7000.")

    # padding
    fho.write(bytes(0x7000 - addr))
    addr = 0x7000

    # 3 strokes
    for data in threesk:
        fho.write(data['word'])
        fho.write(sjis_to_jis(data['kanji']))
        addr+=5

    if addr >= 0x8000:
        raise ValueError("Three strokes dictionary size is overflow. The size should be less than 0x8000.")

    # padding
    fho.write(bytes(0x8000 - addr))
    addr = 0x8000

except ValueError as err:
    print(err)

except Exception as err:
    print(err)

if 'fho' in globals(): fho.close()
if 'fhi' in globals(): fhi.close()

input('Done.')

