import sys
import time
import numpy as np
from PIL import Image

TRUE_THRESHOLD = 5


def nparray2bytearray(nparr):
    pow2 = [2**i for i in range(7, -1, -1)]

    ret = np.zeros(nparr.size / 8, dtype='uint8')

    nparr = nparr.flatten() > TRUE_THRESHOLD
    nparr = nparr.astype('uint8')

    for i in range(nparr.size/8-1):
        ret[i] = np.sum(pow2 * nparr[8*i:8*(i+1)])

    return ret


def img2bytearray(img):
    img = img.convert('L')  # to grayscale
    img = np.array(img)

    return nparray2bytearray(img)


def gif_loop(img):
    frame_dur_sec = img.info['duration'] / 1000.0

    frames = []

    stop = False
    while not stop:
        frames.append(img2bytearray(img))

        try:
            img.seek(img.tell()+1)
        except EOFError:
            stop = True

    sys.stdout.write('Extracted GIF with %d frames.\n' % len(frames))
    sys.stdout.write('Frame duration: %dms\n' % (frame_dur_sec*1000))

    i = 0
    while True:
        serial_send_barr(frames[i])
        i = (i+1) % len(frames)

        time.sleep(frame_dur_sec)


def bytearray2cdef(barr, var_name, max_width=75):
    hex_fmt = {'int': lambda x: '%#04x' % x}

    str_out = np.array2string(barr, max_line_width=max_width, separator=', ',
                              formatter=hex_fmt)

    return 'uint8_t %s[%d] = {\n %s}; ' % (var_name, barr.size, str_out[1:-1])


def serial_send_barr(npbarr):
    import serial

    ser = serial.Serial('/dev/ttyUSB0', baudrate=115200,
                        stopbits=serial.STOPBITS_ONE,
                        bytesize=serial.EIGHTBITS,
                        parity=serial.PARITY_NONE,
                        timeout=1
                        )

    barr = bytearray(npbarr)

    ser.write(barr)

    ser.close()


if __name__ == '__main__':
    if len(sys.argv) == 3:
        img = Image.open(sys.argv[2])

        if sys.argv[1] == 'print':
            barr = img2bytearray(img)
            var_name = sys.argv[2].split('.')[0]
            print bytearray2cdef(barr, var_name)
        elif sys.argv[1] == 'send':

            if img.format == 'GIF':
                gif_loop(img)
            else:
                barr = img2bytearray(img)
                serial_send_barr(barr)
    else:
        sys.stdout.write('Needs more args.\n'
                         'Usage: bmp2hex.py (send | print) img_file\n')
