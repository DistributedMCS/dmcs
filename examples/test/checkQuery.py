#!/usr/bin/env python

import getopt, sys
import optparse
import os
import re
from optparse import OptionParser



def searchAtom(filename, context, kb_size):
    foundAtom = False

    with open(filename, 'r') as f:
        lines = f.readlines()
        for i in range(1,int(kb_size)+1):
            atom = 'c0a' + str(i)
            s = context + ':' + atom + ','
            neg = '-' + s

            for j in range (len(lines)):
                if ((lines[j].find(s) != -1) and (lines[j].find(neg) == -1)):
                    f.closed
                    return atom
    f.closed
    return 'c0a1,NoAtomFound'



def main(argv):
    parser = OptionParser()
    parser.add_option("-i", "--instance", dest="instance",    help="instance to check", metavar="INSTANCE")
    parser.add_option("-f", "--file",     dest="filename",    help="file to check",     metavar="FILENAME")
    parser.add_option("-l", "--logfile",  dest="logfilename", help="file to log",       metavar="LOGFILENAME")
    parser.add_option("-c", "--context",  dest="context",     help="context to check",  metavar="CONTEXT")

    (options, args) = parser.parse_args()

    instance = options.instance
    filename = options.filename
    logfilename = options.logfilename
    context = options.context

    params = re.split('-', instance)
    toponame = params[0]
    kb_size = params[2]

    with open(logfilename, 'a') as f:
        f.write(toponame + ',' + instance + ',' + searchAtom(filename, context, kb_size) + '\n')
    f.closed
        

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
