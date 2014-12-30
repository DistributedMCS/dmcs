#!/usr/bin/env python

import getopt, sys
import optparse
import string
import re
import os
from optparse import OptionParser

def get_running_time(log_time):
    # Watchout: we only expect time of the form MM:SS.SS
    str_time = 'Notime'
    with open(log_time, 'r') as l:
        for timeline in l:
            time_sign = timeline.find('Elapsed (wall clock) time')
            if time_sign != -1:
                last_colon = timeline.rfind(':')
                last_dot = timeline.rfind('.')

                str_minutes = timeline[last_colon-2 : last_colon]
                str_secs = timeline[last_colon+1 : last_dot]
                str_psecs = timeline[last_dot+1 : len(timeline)-1]

                minutes = string.atoi(str_minutes)
                secs = string.atoi(str_secs)

                secs += 60*minutes
                str_time = str(secs) + '.' + str_psecs

                break
    l.closed
    return str_time


def element_compare(element1, element2):
    if element1 == element2:
        return 0
    elif element1 < element2:
        return -1
    else:
        return 1


#  0: if instance1 == instance2
# -1: if instance1 <  instance2
#  1: if instance1 >  instance2
# []: maximum
def instance_compare(instance1, instance2):
    for i in range(5):
        if instance1[i] != instance2[i]:
            return element_compare(instance1[i], instance2[i])

    return 0



def get_engine_running_time(outputdir, testrun, toponame):
    status_filename = outputdir + testrun + '/' + toponame + '-status.log'
    time_filename = outputdir + testrun + '/' + toponame + '-time.log'

    passed = False
    with open(status_filename, 'r') as status_file:
        line = status_file.readline()
        passed_sign = line.find('PASSED')
        if passed_sign != -1:
            passed = True
    status_file.closed

    if passed:
        running_time = get_running_time(time_filename)
    else:
        running_time = '600'

    return running_time



def ordered_push(output, 
                 dmcs_running_time, 
                 dmcsopt_running_time,
                 mcsie_running_time):
    if output == []:
        output.append([dmcs_running_time, dmcsopt_running_time, mcsie_running_time])
    else:
        d_time = float(dmcs_running_time)
        do_time = float(dmcsopt_running_time)
        i = 0
        while i < len(output):
            d_time1 = float(output[i][0])
            do_time1 = float(output[i][1])
            if ((d_time1 > d_time) or ((d_time1 == d_time) and (do_time1 > do_time)) ):
                break
            i = i + 1
        output.insert(i, [dmcs_running_time, dmcsopt_running_time, mcsie_running_time])



def process_test_cases(toponame, current_test_cases):
    # output is an array of arrays of 3 values: [dmcs_running_time, dmcsopt_running_time, mcsie_running_time]
    output = []
    for instance in current_test_cases:
        testrun = toponame + '/' + toponame + '-' + instance[1] + '-' + instance[2] + '-' + instance[3] + '-' + instance[4] + '-' + instance[5]
        
        dmcs_running_time = get_engine_running_time('output/', testrun + '/all', toponame)
        dmcsopt_running_time = get_engine_running_time('output/', testrun + '/opt_all', toponame)
        mcsie_running_time = get_engine_running_time('output-mcsie/', testrun, toponame)

        # DMCS' results are sorted increasingly by the running time of compute_all+non_streaming mode
        # This order must be respected here so that the gnuplot script will produce correct comparisons.
        ordered_push(output, dmcs_running_time, dmcsopt_running_time, mcsie_running_time)

    print output
    output_filename = 'output-mcsie/' + toponame + '/mcsie-' + toponame + '-' + current_test_cases[0][1] + '-' + current_test_cases[0][2] + '-' + current_test_cases[0][3] + '-' + current_test_cases[0][4] + '.dat'
    with open(output_filename, 'w') as ofile:
        for i in range(0,len(output)):
            ofile.write(output[i][1] + '\n')
    ofile.closed



def main(argv):
    topos = ['diamond', 'ring', 'tree', 'zigzag' ]

    for i in range(0, len(topos)):
        filename = 'config/' + topos[i] + '.cfg'
        with open(filename, 'r') as config_file:
            line = config_file.readline()
            line = line[:len(line)-1]
            current_instance = re.split(',', line)
            current_test_case = [current_instance]
            
            while True:
                while True:
                    line = config_file.readline()
                    if line == "":
                        break
                    line = line[:len(line)-1]
                    current_instance = re.split(',', line)
                    if instance_compare(current_instance, current_test_case[0]) == 0:
                        current_test_case.append(current_instance)
                    else:
                        break

                if line == "":
                    break

                process_test_cases(topos[i], current_test_case)
                
                current_test_case = [current_instance]
        config_file.closed


if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
