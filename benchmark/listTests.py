#!/usr/bin/env python

import getopt, sys
import optparse
import string
from optparse import OptionParser

def main(argv):
    parser = OptionParser()
    parser.add_option("-e", "--ext", dest="extension",
                      help="write configuration with extension EXT", metavar="EXT")

    (options, args) = parser.parse_args()

    extension = options.extension

    #toponames      = ['diamond', 'tree', 'zigzag', 'ring']
    #topo_ids       = [ 1, 6, 3, 4 ]
    #system_sizes   = [[4],  # diamond
    #                  [4], # tree
    #                  [4],  # zigzag
    #                  [4]  # ring
    #                 ]

    toponames      = ['diamond', 'tree', 'zigzag', 'ring']
    topo_ids       = [ 1, 6, 3, 4 ]

    system_sizes   = [[4,7,10,13,25,31],  # diamond
                      [7,10,15,31,70,100], # tree
                      [4,7,10,13,25,31,70],  # zigzag
                      [4,7,10,13,70]  # ring
                     ]

    #system_sizes   = [[4, 7, 10, 13, 70]]

    #toponames      = ['diamond']
    #topo_ids       = [ 1 ]
    #system_sizes   = [[4, 7, 10, 13, 25, 31]]

    
    no_atoms       = [ 10, 20 ]
    no_interface   = [ 5,  10 ]
    no_bridgerules = [ 5,  10 ]
    #instances      = [ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' ]
    instances      = [ 'a', 'b', 'c', 'd', 'e']
    

    for i in range(0,len(toponames)):
        filename = 'config/' + toponames[i] + '.' + extension
        with open(filename, 'w') as f:
            for k in range(0,len(no_atoms)):
                for j in range(0,len(system_sizes[i])):
                    for ins in range(0,len(instances)):
                        tmp = str(topo_ids[i]) + ',' + str(system_sizes[i][j]) + ',' + str(no_atoms[k]) + ',' + str(no_interface[k]) + ',' + str(no_bridgerules[k]) + ',' + instances[ins] + '\n'
                        f.write(tmp)
                
        f.closed        

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
