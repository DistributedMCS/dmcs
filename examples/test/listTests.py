#!/usr/bin/env python

import getopt, sys
import optparse
import string

def main(argv):
    toponames      = ['diamond', 'tree', 'zigzag', 'ring']
    topo_ids       = [ 1, 6, 3, 4 ]
    system_sizes   = [[4, 40],  # diamond
                      [4, 100], # tree
                      [4, 40],  # zigzag
                      [4, 100]  # ring
                     ]

    no_atoms       = [10 ]
    no_interface   = [ 5 ]
    no_bridgerules = [ 5 ]
    instances      = [ 'a' ]
    #instances      = [ 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' ]

    for i in range(0,len(toponames)):
        filename = 'config/' + toponames[i] + '.cfg'
        with open(filename, 'w') as f:
            for j in range(0,len(system_sizes[i])):
                for k in range(0,len(no_atoms)):
                    for ins in range(0,len(instances)):
                        tmp = str(topo_ids[i]) + ',' + str(system_sizes[i][j]) + ',' + str(no_atoms[k]) + ',' + str(no_interface[k]) + ',' + str(no_bridgerules[k]) + ',' + instances[ins] + '\n'
                        f.write(tmp)
                
        f.closed        

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
