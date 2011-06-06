#!/usr/bin/python

# convert dmcs .lp and .br files into query plan: .top and .sh file

import sys
import os.path
import re

def debug(msg):
  print >>sys.stderr, "DBG: %s" % (msg,)

def die(msg):
  raise RuntimeError(msg)

###########################
# process commandline
###########################

def usage(identity):
  print >>sys.stderr, '''
    usage: %s <outputname> <ctxlist>

  creates <outputname>.top and <outputname>.sh files

  <ctxlist> consists of pairs of files <ctx.lp> <ctx.br>
  (knowledge base and bridge rule files, respectively)
  (contexts are numbered starting from 1)
  ''' % (identity,)

if len(sys.argv) < 4 or (len(sys.argv) - 2) % 2 != 0:
  usage(sys.argv[0])
  sys.exit(-1)

outputname = sys.argv[1]
contexts = {}
for i in range(1,len(sys.argv)/2):
  lpfile = sys.argv[2*i]
  brfile = sys.argv[2*i+1]
  if (not lpfile.endswith('.lp')) or (not brfile.endswith('.br')):
    raise RuntimeError("lp files must end with '.lp' and br files must end with '.br': %s %s" % (lpfile,brfile))
  # internally we index from 1
  contexts[i] = {
    'idx0' : i-1,
    'idx1' : i,
    'lpfile' : lpfile,
    'brfile' : brfile
  }

debug("outputname=%s contexts=%s" % (outputname, contexts))

###########################
# parse context program and get beliefs as set
###########################
def readContextFile(ctxidx1, lpfile):
  #debug("processing context file %s for ctx %d" % (lpfile,ctxid1))
  ctxf = open(lpfile,"r")
  outputset = set()
  for line in ctxf:
    line = line.strip(". \n\r\t")
    r = re.findall(r'[a-zA-Z_][a-zA-Z0-9_]*',line)
    #debug("finding all atoms in '%s' yielded '%s'" % (line, r))
    if len(r) < 2:
      raise RuntimeError("unexpected rule encountered: '%s'" % (line,))
    r = filter(lambda x: x != 'not', r)
    #debug("filtering out NAF yielded '%s'" % (r,))
    outputset.update(set(r))
  #debug("got set %s" % (outputset,))
  return outputset

for ctx in contexts.itervalues():
  ctx['lpbeliefs'] = readContextFile(ctx['idx1'], ctx['lpfile'])

###########################
# parse bridge rule file and get (context-indexed) beliefs as set
###########################
def readBridgeRuleFile(ctxidx1, brfile):
  debug("processing bridge rule file %s for ctx %d" % (brfile,ctxidx1))
  brf = open(brfile,"r")
  outputdict = {}
  for line in brf:
    line = line.strip(". \n\r\t")
    bodies = re.findall(r'\(\s*([0-9]+)\s*:\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*\)',line)
    heads = re.findall(r'^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*:-',line)
    debug("finding all bridge rule atoms in '%s' yielded heads %s and bodies %s" % (line, heads, bodies))
    if len(heads) != 1:
      raise RuntimeError("unexpected bridge rule encountered: '%s'" % (line,))
    bodies.append( (str(ctxidx1),heads[0]) )
    for (idx1,belief) in bodies:
      iidx1 = int(idx1)
      debug("adding (%d:%s)" % (iidx1,belief))
      if iidx1 not in outputdict:
        outputdict[iidx1] = set()
      outputdict[iidx1].add(belief)
  return outputdict

# prepare all contexts
for ctx in contexts.itervalues():
  # those are the beliefs of this context used by bridge rules in this or other contexts
  ctx['exportedbeliefs'] = set()
  # those are the beliefs of contexts used by bridge rules in this context
  ctx['importedbeliefs'] = {}

# collect all beliefs we use in bridge rules
for ctx in contexts.itervalues():
  brbeliefs = readBridgeRuleFile(ctx['idx1'], ctx['brfile'])
  ibeliefs = contexts[ctx['idx1']]['importedbeliefs']
  for (cidx,beliefs) in brbeliefs.iteritems():
    #debug("got br items ctx %d => %s" % (cidx, beliefs))
    ebeliefs = contexts[cidx]['exportedbeliefs']
    ebeliefs.update(beliefs)
    ibeliefs[cidx] = beliefs

debug("number of contexts = %d" % (len(contexts),))

###########################
# collect exportedbeliefs and lpbeliefs into beliefs and create indices as required by query plan
###########################
for ctx in contexts.itervalues():
  debug("context %d: %s" % (ctx['idx1'],ctx))
  ctx['beliefs'] = {}
  idx = 0
  for belief in ctx['lpbeliefs']:
    ctx['beliefs'][idx] = belief
    idx+= 1
  for belief in ctx['exportedbeliefs'] - ctx['lpbeliefs']:
    ctx['beliefs'][idx] = belief
    idx+= 1
  debug("indexed beliefs of context %d: %s" % (ctx['idx1'],ctx['beliefs']))

###########################
# start output
###########################
topfile = "%s.top" % (outputname,)
if os.path.exists(topfile):
  die("output file %s already exists!" % (topfile,))

ftop = open(topfile,"w+")
print >>ftop, 'digraph G {'
print >>ftop, 'graph [name="%s"]'
print >>ftop, '}'

ftop.close()
