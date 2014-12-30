#!/usr/bin/env python

import os
import re

def addCtxID(ctxID, rule):
    ruleSign = rule.find(':-')
    ruleHead = rule[:ruleSign]
    ruleBody = rule[ruleSign+2:]
    newRule = '(' + str(ctxID) + ':' + ruleHead + ') :-' + ruleBody

    return newRule

def convert(currentDir, topo, line):
    line = line[:len(line)-1]
    currentInstance = re.split(',', line)
    systemSize = int(currentInstance[1])

    testcaseDir = currentDir + '/data/' + topo + '/' + topo
    for i in range(1,6):
        testcaseDir = testcaseDir + '-' + currentInstance[i]

    fileMasterHex = testcaseDir + '/master.hex'
    with open(fileMasterHex, 'w') as master:
        for i in range(systemSize):
            s = '#context(' + str(i) + ', "dlv_asp_context_acc", "' + topo + '-' + str(i) + '.lp").\n'
            master.write(s)

        master.write('\n')

        ruleID = 0
        for i in range(systemSize):
            fileBridgeRule = testcaseDir + '/' + topo + '-' + str(i) + '.br'
            with open(fileBridgeRule, 'r') as br:
                rules = br.readlines()
                for j in range(len(rules)):
                    convertedRule = addCtxID(i, rules[j])
                    ruleID = ruleID + 1
                    s = 'r' + str(ruleID) + ': ' + convertedRule
                    master.write(s)
            br.closed
    master.closed



    print(testcaseDir)

def main(argv):
    currentDir = os.getcwd()
    topologies = ['diamond', 'tree', 'zigzag', 'ring']
    #topologies = ['diamond']

    for topo in topologies:
        configFile = currentDir + '/config/' + topo + '.cfg'
        with open(configFile,'r') as cfg:
            lines = cfg.readlines()
            for i in range(len(lines)):
                convert(currentDir, topo, lines[i])
        cfg.closed
            

if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
