#!/usr/bin/env python

import os
import re


def convertLocalRule(r):
    marker1 = ':- not '

    pos1 = r.find(marker1)
    pos2 = r.find('.')

    head = r[:pos1]
    body = r[pos1 + len(marker1) : pos2]

    return ('~' + body + '_local -> ' + head + '_local')



def convertBridgeRule(r, currentCtx):
    marker1 = ':- '
    
    pos1 = r.find(marker1)
    
    head = r[:pos1] + '_peer' + str(currentCtx)

    pos2 = r.find('(')
    pos3 = r.find(')')

    body = ''

    while 1:
        neg = r.find('not', pos1, pos2)
        if neg != -1:
            body = body + '~'

        fullbody = r[pos2+1:pos3]
        pos4 = fullbody.find(':')
        ctx = fullbody[:pos4]
        atom = fullbody[pos4+1:]

        pos1 = pos3
        pos2 = r.find('(', pos3+1)
        pos3 = r.find(')', pos3+1)

        if pos2 != -1:
            body = body + atom + '_peer' + ctx + ', '
        else:
            body = body + atom + '_peer' + ctx
            break

    return body + ' -> ' + head



def createRuleFile(fileP2PDR_rules, fileLocalKB, fileBridgeRules, currentCtx):
    with open(fileP2PDR_rules, 'w') as f:
        with open(fileLocalKB, 'r') as localKB:
            rules = localKB.readlines()
            for i in range(len(rules)):
                convertedRule = convertLocalRule(rules[i])
                f.write('L'+ str(i+1) + ': ' + convertedRule + '\n')
        localKB.closed

        f.write('')
        with open(fileBridgeRules, 'r') as brRules:
            rules = brRules.readlines()
            for i in range(len(rules)):
                convertedRule = convertBridgeRule(rules[i], currentCtx)
                f.write('M'+ str(i+1) + ': ' + convertedRule + '\n')
        brRules.closed
    f.closed



def createTrustFile(fileP2PDR_trust, currentCtx, systemSize):
    with open(fileP2PDR_trust, 'w') as f:
        for j in range(systemSize):
            if j != currentCtx:
                f.write('peer' + str(j) + '\n')
    f.closed



def convert(currentDir, topo, line):
    line = line[:len(line)-1]
    currentInstance = re.split(',', line)
    systemSize = int(currentInstance[1])

    testcaseDir = currentDir + '/data/' + topo + '/' + topo
    outputDir = currentDir + '/data-p2p-dr/' + topo + '/' + topo

    for i in range(1,6):
        testcaseDir = testcaseDir + '-' + currentInstance[i]
        outputDir = outputDir + '-' + currentInstance[i]

    if not os.path.exists(outputDir):
        print "Make dir " + outputDir
        os.makedirs(outputDir)

    for i in range(systemSize):
        fileLocalKB = testcaseDir + '/' + topo + '-' + str(i) + '.lp'
        fileBridgeRule = testcaseDir + '/' + topo + '-' + str(i) + '.br'
        fileP2PDR_rules = outputDir + '/peer' + str(i) + '_rules.txt'
        fileP2PDR_trust = outputDir + '/peer' + str(i) + '_trust.txt'
        
        createRuleFile(fileP2PDR_rules, fileLocalKB, fileBridgeRule, i)
        createTrustFile(fileP2PDR_trust, i, systemSize)



def main(argv):
    currentDir = os.getcwd()
    #topologies = ['diamond', 'tree', 'zigzag', 'ring']
    topologies = ['diamond']

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
