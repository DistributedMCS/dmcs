#!/usr/bin/env python

import getopt, sys
import optparse
import string
import re
import os


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



def get_error_code(log_time):
    error_code = '---'
    with open(log_time, 'r') as l:
        line = l.readline()
        sign = line.find('Command exited with non-zero status')
        error_code = line[sign + len('Command exited with non-zero status')+1 : len(line)-1]
    l.closed
    return '1000' + error_code



# For the header, footer of the table, we just copy from some template file outside
def copy_text(output_file, template_file):
    with open(template_file, 'r') as t:
        lines = t.readlines()
        for i in range(len(lines)):
            output_file.write(lines[i])
    t.closed



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



def get_no_answers(filename):
    no_answers = '---'
    with open(filename, 'r') as f:
        for line in f.readlines():
            sign = line.find('Total Number of Equilibria:')
            if sign != -1:
                no_answers = line[sign + len('Total Number of Equilibria:') + 1 : len(line)-1]
    f.closed
    return no_answers


def get_no_intermediate_answers(filename):
    no_answers = '---'
    print'"filename = ' + filename
    with open(filename, 'r') as f:
        lines = f.readlines()
        if len(lines) > 0:
            last_line = lines[len(lines)-1]
            print 'last_line = ' + last_line
            sign = last_line.find('Received partial equilibria:')
            if sign != -1:
                no_answers = last_line[sign + len('Received partial equilibria:') + 1 : len(last_line)-1]
                print 'no_answers = ' + no_answers
    f.closed
    return no_answers


def get_outcome(toponame, dirname):
    err_filename    = dirname + '/' + toponame + '-err.log'
    status_filename = dirname + '/' + toponame + '-status.log'
    time_filename   = dirname + '/' + toponame + '-time.log'
    log_filename    = dirname + '/' + toponame + '.log'

    passed = False
    with open(status_filename, 'r') as status_file:
        line = status_file.readline()
        passed_sign = line.find('PASSED')
        if passed_sign != -1:
            passed = True
    status_file.closed

    outcome = ''
    noans = '---'
    if passed:
        outcome = get_running_time(time_filename)
        noans = get_no_answers(log_filename)
    else:
        outcome = get_error_code(time_filename)
        noans = get_no_intermediate_answers(err_filename)

    return outcome, noans



def complete_test_case(dirname, subdirs):
    if os.path.exists(dirname) == False:
        return False

    for subdir in subdirs:
        if os.path.exists(dirname + subdir) == False:
            return False

    return True



def ordered_push(outcomes, outcome, sorted_testcases, testcase):
    if outcomes == []:
        outcomes.append(outcome)
        sorted_testcases.append(testcase)
    else:
        i = 0
        new_val = float(outcome[0][0])
        found = False

        while i < len(outcomes):
            old_val = float(outcomes[i][0][0])
            i = i + 1
            if old_val > new_val:
                found = True
                break

        if found == True:
            outcomes.insert(i-1, outcome)
            sorted_testcases.insert(i-1, testcase)
        else:
            outcomes.insert(i, outcome)
            sorted_testcases.insert(i, testcase)



def process_test_cases(toponame, testpacks, current_test_case):
    outcomes = []
    sorted_testcases = []

    test_runs = []
    for p in testpacks:
        if (p != 'all') and (p != '1'):
            test_runs.append('/noloop_' + p)
        test_runs.append('/' + p)

        if (p != 'all') and (p != '1'):
            test_runs.append('/opt_noloop_' + p)
        test_runs.append('/opt_' + p)

    for instance in current_test_case:
        dirname = 'output/' + toponame + '/' + toponame + '-' + instance[1] + '-' + instance[2] + '-' + instance[3] + '-' + instance[4] + '-' + instance[5]
        if complete_test_case(dirname, test_runs):
            outcome = []
            for r in test_runs:
                outcome.append(get_outcome(toponame, dirname + r))

            ordered_push(outcomes, outcome, sorted_testcases, instance)

    return outcomes, sorted_testcases



def final_display(val):
    if val == '1000124':
        return '---'
    elif val == '10001':
        return 'E'
    else:
        return val



def nice_display(time, val):
    if time == '1000124':        
        return '(' + val + ')'
    elif time == '10001':
        return 'E'
    else:
        return time

def combined_display(time, val):
    if time == '1000124':        
        return '(' + val + ')'
    elif time == '10001':
        return 'E'
    else:
        blank_count = 4 - len(val)
        blanks = ''
        for i in range(blank_count):
            blanks = blanks + '\ '
        return time + blanks + '(' + val + ')'


def build_row(tex_output, tex_row_template, 
              outcomes, sorted_testcases,
              topo, topo_abbr):
    copy_text(tex_output, 'templates/tex_separator.tpl')
    for i in range(len(outcomes)):
        outcome = outcomes[i]
        instance = sorted_testcases[i]

        #print len(outcome)
        #print instance
        mem = topo + '-' + instance[1] + '-' + instance[2] + '-' + instance[3] + '-' + instance[4] + '-' + instance[5] 
        str_i = '{' + str(i+1) + '}'

        print outcome

        tex_output.write(tex_row_template.format(mem,
                                                 topo_abbr,
                                                 str_i,
                                                 instance[1],
                                                 instance[2],
                                                 instance[3],
                                                 instance[4],
                                                 final_display(outcome[0][0]),
                                                 final_display(outcome[0][1]),
                                                 final_display(outcome[1][0]),
                                                 final_display(outcome[1][1]),
                                                 nice_display(outcome[2][0], outcome[2][1]),
                                                 nice_display(outcome[3][0], outcome[3][1]),
                                                 final_display(outcome[4][0]), 
                                                 outcome[4][1],
                                                 nice_display(outcome[5][0], outcome[5][1]),
                                                 final_display(outcome[6][0]), 
                                                 outcome[6][1],
                                                 nice_display(outcome[7][0], outcome[7][1]),
                                                 final_display(outcome[8][0]), 
                                                 outcome[8][1],
                                                 nice_display(outcome[9][0], outcome[9][1]),
                                                 final_display(outcome[10][0]), 
                                                 outcome[10][1],
                                                 nice_display(outcome[11][0], outcome[11][1])))


def main(argv):
    # copy header
    tex_output = open('./table.tex', 'w')
    copy_text(tex_output, 'templates/tex_header.tpl')

    # read row template
    with open('templates/tex_row.tpl', 'r') as t:
        tex_row_template = t.read()
    t.closed

    topo_abbreviation = {'diamond' : 'D', 'ring' : 'R', 'tree' : 'T', 'zigzag' : 'Z'}
    #topologies = ['diamond', 'ring', 'tree', 'zigzag']
    topologies = ['ring']
    testpacks  = ['all', '1', '10', '100' ]
    subdirs = ['', '/all', '/opt_all', '/1', '/opt_1', '/10', '/noloop_10', '/opt_10', '/opt_noloop_10', '/100', '/noloop_100', '/opt_100', '/opt_noloop_100']

    for topo in topologies:
        filename = 'config/' + topo + '.out'
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

                outcomes, sorted_testcases = process_test_cases(topo, testpacks, current_test_case)

                if outcomes != []:
                    build_row(tex_output, tex_row_template, 
                              outcomes, sorted_testcases,
                              topo, topo_abbreviation[topo])

                #print outcomes
                #print "\n"
                #print sorted_testcases
                #print "\n"

                if line == "":
                    break
                
                current_test_case = [current_instance]
        config_file.closed

        copy_text(tex_output, 'templates/tex_footer.tpl')
    tex_output.closed

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
