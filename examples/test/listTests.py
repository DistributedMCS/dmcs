#!/usr/bin/env python

import getopt, sys
import optparse
import string

def main(argv):
    toponames      = ['diamond', 'tree', 'zigzag', 'ring']
    system_sizes   = [[10, 20, 40], # diamond
                      [50, 100],    # tree
                      [20, 80],     # zigzag
                      [40, 100]     # ring
                     ]

    no_atoms       = [10, 40 ]
    no_interface   = [ 5, 20 ]
    no_bridgerules = [ 5, 10 ]
    instances      = [ 'a', 'b' ]
    #instances      = [ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' ]

    for i in range(0,len(toponames)):
        filename = 'config/' + toponames[i] + '.cfg'
        with open(filename, 'w') as f:
            for j in range(0,len(system_sizes[i])):
                for k in range(0,len(no_atoms)):
                    for ins in range(0,len(instances)):
                        tmp = str(i+1) + ',' + str(system_sizes[i][j]) + ',' + str(no_atoms[k]) + ',' + str(no_interface[k]) + ',' + str(no_bridgerules[k]) + ',' + instances[ins] + '\n'
                        f.write(tmp)
                
        f.closed        

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
