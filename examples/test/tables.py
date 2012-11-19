#!/usr/bin/env python

import getopt, sys
import optparse
import string
import re


def get_running_time(log_time):
    # Watchout: we only expect time of the form MM:SS.SS
    str_time = 'Notime'
    print "checking " + log_time
    with open(log_time, 'r') as l:
        print 'File opened'
        for timeline in l:
            print timeline
            time_sign = timeline.find('Elapsed (wall clock) time')
            if time_sign != -1:
                print 'Found time'
                last_colon = timeline.rfind(':')
                last_dot = timeline.rfind('.')

                str_minutes = timeline[last_colon-2 : last_colon]
                str_secs = timeline[last_colon+1 : last_dot]
                str_psecs = timeline[last_dot+1 : len(timeline)-1]

                print timeline
                print log_time
                print str_minutes

                minutes = string.atoi(str_minutes)
                secs = string.atoi(str_secs)

                secs += 60*minutes
                str_time = str(secs) + '.' + str_psecs

                break
    l.closed

    return str_time



# For the header, footer of the table, we just copy from some template file outside
def copy_text(output_file, template_file):
    with open(template_file, 'r') as t:
        lines = t.readlines()
        for i in range(len(lines)):
            output_file.write(lines[i])
    t.closed



def readline(logfile, topo_abbreviation):
    line = logfile.readline()

    if line == "":
        return []

    info = re.split(',', line)
    subinfo = re.split('-', info[1])
    retval  = [ info[0], topo_abbreviation[info[0]], subinfo[1], subinfo[2], subinfo[3], subinfo[4], subinfo[5], info[2] ]
    
    if info[3] != '\n':
        # get rid of the last \n character
        retval.append(info[3][:len(info[3])-1])

    #print retval
    return retval


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
def list_compare(instance1, instance2):
    if instance1 == [] and instance2 == []:
        return 0
    elif instance1 == []:
        return -1
    elif instance2 == []:
        return 1

    for i in range(6):
        if instance1[i] != instance2[i]:
            return element_compare(instance1[i], instance2[i])

    return 0


def grap_current_test_case(current_passed_instance,
                           current_failed_instance,
                           passedtests,
                           failedtests,
                           topo_abbreviation):
    
    # [0] for passed instances
    # [1] for failed instances
    current_test_case = [ [], [] ]
    next_test_case = []
    
    while True:
        diff = list_compare(current_passed_instance, current_failed_instance)
        
        if diff == 0:
            if list_compare(current_passed_instance, next_test_case) == 0:
                break;
            current_test_case[0].append(current_passed_instance)
            current_test_case[1].append(current_failed_instance)
            current_passed_instance = readline(passedtests, topo_abbreviation)
            current_failed_instance = readline(failedtests, topo_abbreviation)
            next_test_case = []
        elif diff == -1:
            current_test_case[0].append(current_passed_instance)
            current_failed_instance = readline(failedtests, topo_abbreviation)
            next_test_case = current_failed_instance
        elif diff == 1:
            current_test_case[1].append(current_failed_instance)
            current_passed_instance = readline(passedtests, topo_abbreviation)
            next_test_case = current_passed_instance

    return current_test_case,current_passed_instance,current_failed_instance

def list_current_test_case(current_test_case):
    for instance in current_test_case[0]:
        print instance

    print "\n"

    for instance in current_test_case[1]:
        print instance


def main(argv):
    # copy header
    tex_output = open('./table.tex', 'w')
    copy_text(tex_output, 'templates/tex_header.tpl')

    # read row template
    with open('templates/tex_row.tpl', 'r') as t:
        tex_row_template = t.read()
    t.closed

    topo_abbreviation = {'diamond' : 'D', 'ring' : 'R', 'tree' : 'T', 'zig-zag' : 'Z'}

    with open('passedtests.log', 'r') as passedtests:
        with open('failedtests.log', 'r') as failedtests:
            current_passed_instance = readline(passedtests, topo_abbreviation)
            current_failed_instance = readline(failedtests, topo_abbreviation)

            while True:
                current_test_case,current_passed_instance,current_failed_instance = grap_current_test_case(current_passed_instance,
                                                                                                           current_failed_instance,
                                                                                                           passedtests,
                                                                                                           failedtests,
                                                                                                           topo_abbreviation)
                if current_test_case == [ [],[] ]:
                    break;

                list_current_test_case(current_test_case)

        failedtests.closed
    passedtests.closed

    

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
