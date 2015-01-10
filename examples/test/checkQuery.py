#!/usr/bin/env python

import getopt, sys
import optparse
import os
import re
from optparse import OptionParser

def main(argv):
    parser = OptionParser()
    parser.add_option("-i", "--instance", dest="instance",    help="instance to check", metavar="INSTANCE")
    parser.add_option("-f", "--file",     dest="filename",    help="file to check",     metavar="FILENAME")
    parser.add_option("-l", "--logfile",  dest="logfilename", help="file to log",       metavar="LOGFILENAME")
    parser.add_option("-c", "--context",  dest="context",     help="context to check",  metavar="CONTEXT")
    parser.add_option("-a", "--atom",     dest="atom",        help="atom to check",     metavar="ATOM")

    (options, args) = parser.parse_args()

    instance = options.instance
    filename = options.filename
    logfilename = options.logfilename
    context = options.context
    atom = options.atom

    foundAtom = False
    s = context + ':' + atom
    neg = '-' + s
    with open(filename, 'r') as f:
        for line in f:
            if line.find(s) != -1 and line.find(neg) == -1:
                foundAtom = True
                break;
    f.closed

    with open(logfilename, 'a') as f:
        if foundAtom:
            f.write(instance + ': OK\n')
        else:
            f.write(instance + ': BAD\n')
    f.closed
        

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
