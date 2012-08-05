#!/usr/bin/env python
#
# Python Serial Port Extension for Win32, Linux, BSD, Jython
# module for serial IO for POSIX compatible systems, like Linux
# see __init__.py
#
# (C) 2001-2010 Chris Liechti <cliechti@gmx.net>
# this is distributed under a free software license, see license.txt
#
# parts based on code from Grant B. Edwards  <grante@visi.com>:
#  ftp://ftp.visi.com/users/grante/python/PosixSerial.py
#
# references: http://www.easysw.com/~mike/serial/serial.html

import sys, fcntl, termios

# Do check the Python version as some constants have moved.
if (sys.hexversion < 0x020100f0):
    import TERMIOS
else:
    TERMIOS = termios

if (sys.hexversion < 0x020200f0):
    import FCNTL
else:
    FCNTL = fcntl

# try to detect the OS so that a device can be selected...
# this code block should supply a device() and set_special_baudrate() function
# for the platform
plat = sys.platform.lower()

if   plat[:5] == 'linux':    # Linux (confirmed)

    ASYNC_SPD_MASK = 0x1030
    ASYNC_SPD_CUST = 0x0030

    def set_special_baudrate(port, baudrate):
        import array
        buf = array.array('i', [0] * 32)

        # get serial_struct
        FCNTL.ioctl(port.fd, TERMIOS.TIOCGSERIAL, buf)

        # set custom divisor
        buf[6] = (13000000 + 7 * baudrate) / (14 * baudrate) - 1

        # update flags
        buf[4] &= ~ASYNC_SPD_MASK
        buf[4] |= ASYNC_SPD_CUST

        # set serial_struct
        try:
            res = FCNTL.ioctl(port.fd, TERMIOS.TIOCSSERIAL, buf)
        except IOError:
            raise ValueError('Failed to set custom baud rate: %r' % baudrate)
