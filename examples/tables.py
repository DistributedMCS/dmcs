#!/usr/bin/env python

import getopt, sys
import optparse
import string



# Check whether the test got PASS or FAILED
def check_status(log_status):
    with open(log_status, 'r') as s:
        for line in s:
            status_sign = line.find('PASS')
            if status_sign != -1:
                s.closed
                return 'PASS'
            else:
                status_sign = line.find('FAILED')
                if status_sign != -1:
                    s.closed
                    return 'FAILED'
    s.closed
    return 'UNKNOWN'



# Check 'out of time' in status file
def check_timeout_status(log_status):
    with open(log_status, 'r') as s:
        for line in s:
            status_sign = line.find('out of time')
            if status_sign != -1:
                s.closed
                return 'TIMEOUT'

    s.closed
    return 'UNKNOWN'


def why_failed(testpath, testname, context, testmode, log_tail):

    no_context = string.atoi(context)
    str_mem_record = './' + testname + '-' + testmode + '-memory.' + log_tail

    mem_record = open(str_mem_record, 'w')
    failed_reason = 'UNKNOWN'

    for i in range(1, no_context+1):
        # record memory used
        log_run = testpath + '/' + testname + '-' + testmode + '-run-' + str(i) + '.' + log_tail
        print "Checking " + log_run
        space_used = ' No information\n'
        with open(log_run, 'r') as r:
            lines = r.readlines()

            # a reverse 'for' loop to find the space consumtion faster
            for j in range(len(lines) - 1, 0, -1):
                line = lines[j]
                space_sign = line.find('space:')
                if (space_sign != -1):
                    print line
                    space_used = line[space_sign+6:]
                    break

        mem_record.write(str(i) + ":" + space_used)
        r.closed


        # find whether it was MEMOUT
        log_context = testpath + '/' + testname + '-' + testmode + '-' + str(i) + '.' + log_tail
        with open(log_context, 'r') as c:
            for line in c:
                memout_sign = line.find('std::bad_alloc')
                if memout_sign != -1:
                    failed_reason = 'MEMOUT'
                    break
        c.closed

    mem_record.close

    # find whether it was TIMEOUT
    if failed_reason == 'UNKNOWN':
        log_time = testpath + '/' + testname + '-' + testmode + '-time.' + log_tail
        print log_time
        str_running_time = get_running_time(log_time);
        running_time = float(str_running_time)
        if running_time >= 180:
            failed_reason = 'TIMEOUT'

    # check whether it was TIMEOUT by looking into status file
    # This information is returned to us by runlim
    # It's weird that runlim says 'out of time' while measured running time is less than 180 secs. Need to check this.
    if failed_reason == 'UNKNOWN':
        log_status = testpath + '/' + testname + '-' + testmode + '-status.' + log_tail
        failed_reason = check_timeout_status(log_status)

    return failed_reason

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
            if toponame == 'zig':
                toponame = 'zig-zag'
                first_dash = second_dash
                second_dash = third_dash
                third_dash = fourth_dash
                fourth_dash = fifth_dash
                fifth_dash = testname.find('-', fifth_dash+1)

            context = testname[first_dash+1 : second_dash]
            signature = testname[second_dash+1 : third_dash]
            interface = testname[third_dash+1 : fourth_dash]
            bridge_rules = testname[fourth_dash+1 : fifth_dash]
            instance = testname[fifth_dash+1:]
            parameter_setting = testname[first_dash+1 : fifth_dash]

            testpath = 'experiments/' + toponame + '/' + parameter_setting + '/' + testname

            #testmodes = ['opt', 'streaming', 'parallel']
            testmodes = ['opt', 'streaming']
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
                        # find the reason for FAILING, which is either TIMEOUT or MEMOUT
                        failed_reason = why_failed(testpath, testname, context, mode, log_tail)
                        if failed_reason == 'TIMEOUT':
                            times[1] = '---'
                        elif failed_reason == 'MEMOUT':
                            times[1] = 'M'
                        else:
                            times[1] = 'U'
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
                            # find the reason for FAILING, which is either TIMEOUT or MEMOUT
                            failed_reason = why_failed(testpath, testname, context, mode + '-k' + package, log_tail)
                            if failed_reason == 'TIMEOUT':
                                times[pos] = '---'
                            elif failed_reason == 'MEMOUT':
                                times[pos] = 'M'
                            else:
                                times[pos] = 'U'

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
