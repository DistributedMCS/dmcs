#!/usr/bin/env python

import getopt, sys
import optparse
import string



# Check whether the test got PASS or FAILED
def check_status(log_status):
    with open(log_status, 'r') as s:
        lines = s.readlines()
        status = lines[2][0:4]
    s.closed

    return status



def get_no_answer(log_answer):
    no_answer = '---'

    with open(log_answer, 'r') as a:
        lines = a.readlines()
        for i in range(len(lines)):
            line = lines[i]
            answer_signal = line.find("Total Number of Equilibria:")
            if answer_signal != -1:
                colon = line.rfind(':')
                plus = line.rfind('+')
                if plus == -1:
                    plus = len(line)
            no_answer = line[colon+2 : plus]
            break
    a.closed
    return no_answer



def get_running_time(log_time):
    # Watchout: we only expect time of the form MM:SS.SS
    with open(log_time, 'r') as l:
        lines = l.readlines()
        timeline = lines[4]
        last_colon = timeline.rfind(':')
        last_dot = timeline.rfind('.')

        str_minutes = timeline[last_colon-2 : last_colon]
        str_secs = timeline[last_colon+1 : last_dot]
        str_psecs = timeline[last_dot+1 : len(timeline)-1]

        minutes = string.atoi(str_minutes)
        secs = string.atoi(str_secs)

        secs += 60*minutes
        str_time = str(secs) + '.' + str_psecs
    l.closed

    return str_time



def getPosition(mode, package):
    mode_index = 0
    if mode == 'parallel':
        mode_index = 1

    package_index = 0
    if package == '10':
        package_index = 1
    elif package == '100':
        package_index = 2

    position = 2 + mode_index*3 + package_index

    return position



# For the header, footer of the table, we just copy from some template file outside
def copy_text(output_file, template_file):
    with open(template_file, 'r') as t:
        lines = t.readlines()
        for i in range(len(lines)):
            output_file.write(lines[i])
    t.closed



def main(argv):

    parser = optparse.OptionParser()
    parser.add_option('-f', '--file', dest="filename")

    (options, args) = parser.parse_args()

    # copy header
    tex_output = open('./table.tex', 'w')
    copy_text(tex_output, 'template/tex_header.tpl')

    html_output = open('./table.html', 'w')
    copy_text(html_output, 'template/html_header.tpl')

    # read row template
    with open('template/tex_row.tpl', 'r') as t:
        tex_row_template = t.read()
    t.closed

    with open('template/html_first_row.tpl', 'r') as h1:
        html_first_row_template = h1.read()
    h1.closed

    with open('template/html_row.tpl', 'r') as h2:
        html_row_template = h2.read()
    h2.closed

    with open('template/html_row_odd.tpl', 'r') as h3:
        html_row_odd_template = h3.read()
    h3.closed

    # read row separation template
    with open('template/tex_row_separation.tpl', 'r') as rs:
        row_sep = rs.readlines()
    rs.closed

    topo_abbreviation = {'tree' : 'T', 'diamond' : 'D', 'zig-zag' : 'Z', 'ring' : 'R'}

    vals = []

    # read test cases and process
    count = 0
    with open(options.filename, 'r') as f:
        for testname in f:
            # get rid of the last '\n' character
            testname = testname[:len(testname)-1]

            if testname == 'end':
                for i in range(len(row_sep)):
                   tex_output.write(row_sep[i])
                tex_output.write("")

                # for HTML tables, we want to group all results belonging to one parameter setting
                html_output.write(html_first_row_template.format(testname, toponame, count, 
                                                                 context, signature, interface, bridge_rules, 
                                                                 vals[0][0], vals[0][1], vals[0][2], vals[0][3], 
                                                                 vals[0][4], vals[0][5], vals[0][6], vals[0][7]))

                for i in range(1, len(vals)):
                    if i % 2 == 1:
                        html_output.write(html_row_template.format(vals[i][0], vals[i][1], vals[i][2], vals[i][3],
                                                                   vals[i][4], vals[i][5], vals[i][6], vals[i][7]))
                    else:
                        html_output.write(html_row_odd_template.format(vals[i][0], vals[i][1], vals[i][2], vals[i][3],
                                                                       vals[i][4], vals[i][5], vals[i][6], vals[i][7]))

                vals = []
                count = 0
                continue

            count += 1
            first_dash  = testname.find('-')
            second_dash = testname.find('-', first_dash+1)
            third_dash  = testname.find('-', second_dash+1)
            fourth_dash = testname.find('-', third_dash+1)
            fifth_dash = testname.find('-', fourth_dash+1)

            toponame = testname[: first_dash]
            context = testname[first_dash+1 : second_dash]
            signature = testname[second_dash+1 : third_dash]
            interface = testname[third_dash+1 : fourth_dash]
            bridge_rules = testname[fourth_dash+1 : fifth_dash]
            instance = testname[fifth_dash+1:]
            parameter_setting = testname[first_dash+1 : fifth_dash]

            testpath = 'experiments/' + toponame + '/' + parameter_setting + '/' + testname

            testmodes = ['opt', 'streaming', 'parallel']
            testpackage = ['0', '10', '100']
            log_tail = 'log'
            tableline = ''
            no_answer = '---'

            times = [''] * 8

            for mode in testmodes:
                if mode == 'opt':
                    log_status = testpath + '/' + testname + '-opt-status.' + log_tail
                    status = check_status(log_status)
                    if status == 'PASS':
                        log_time = testpath + '/' + testname + '-opt-time.' + log_tail
                        log_answer = testpath + '/' + testname + '-opt.' + log_tail
                        no_answer = get_no_answer(log_answer)
                        times[1] = get_running_time(log_time)
                    else:
                        times[1] = '---'            
                else:
                    for package in testpackage:
                        pos = getPosition(mode, package)
                        log_status = testpath + '/' + testname + '-' + mode + '-k' + package + '-status.' + log_tail
                        status = check_status(log_status)
                        if status == 'PASS':
                            log_time = testpath + '/' + testname + '-' + mode + '-k' + package + '-time.' + log_tail
                            times[pos] = get_running_time(log_time)

                            # If the opt case failed, there is still a hope that steaming or parallel found all answers.
                            # This is the case when the number of answers returned is smaller than the package size.

                            if no_answer == '---':
                                log_answer = testpath + '/' + testname + '-' + mode + '-k' + package + '.' + log_tail
                                no_answer = get_no_answer(log_answer)
                                if no_answer == package:
                                    no_answer = '---'

                        else:
                            times[pos] = '---'

            times[0] = no_answer
            vals.append(times)

            tex_output.write(tex_row_template.format(testname, topo_abbreviation[toponame], count, 
                                                     context, signature, interface, bridge_rules, times[0],
                                                     times[1], times[2], times[3], times[4], times[5], times[6], times[7]))


    copy_text(tex_output, 'template/tex_footer.tpl')
    copy_text(html_output, 'template/html_footer.tpl')
    tex_output.close
    html_output.close
    f.closed



if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
