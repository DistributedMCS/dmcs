#!/usr/bin/python

# convert dmcs .lp and .br files into query plan: .top and .sh file

# some assumptions/configuration:

# QUERYCTX: 1-based index of context where we want to see all beliefs = where we query
QUERYCTX=1

# FIRSTPORT: network port where we want the first context to operate, other contexts use ports above
FIRSTPORT=5001

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
# parse bridge rule file and get tuple of beliefs as sets:
# (set(brhead1,brhead2), { ctx : set(bratom1,...), ctx : ... }
###########################
def readBridgeRuleFile(ctxidx1, brfile):
  debug("processing bridge rule file %s for ctx %d" % (brfile,ctxidx1))
  brf = open(brfile,"r")
  outputset = set()
  outputdict = {}
  for line in brf:
    line = line.strip(". \n\r\t")
    bodies = re.findall(r'\(\s*([0-9]+)\s*:\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*\)',line)
    heads = re.findall(r'^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*:-',line)
    debug("finding all bridge rule atoms in '%s' yielded heads %s and bodies %s" % (line, heads, bodies))
    if len(heads) != 1:
      raise RuntimeError("unexpected bridge rule encountered: '%s'" % (line,))
    outputset.update(set(heads))
    for (idx1,belief) in bodies:
      iidx1 = int(idx1)
      debug("adding (%d:%s)" % (iidx1,belief))
      if iidx1 not in outputdict:
        outputdict[iidx1] = set()
      outputdict[iidx1].add(belief)
  return (outputset, outputdict)

# prepare all contexts
for ctx in contexts.itervalues():
  # those are the beliefs of this context used by bridge rules in this or other contexts
  ctx['exportedbeliefs'] = set()
  # those are the beliefs of contexts used by bridge rules in this context
  ctx['importedbeliefs'] = {}

# collect all beliefs we use in bridge rules
for ctx in contexts.itervalues():
  (brheads,brbeliefs) = readBridgeRuleFile(ctx['idx1'], ctx['brfile'])

  # those are the beliefs added by bridge rules of this context
  ctx['injectedbeliefs'] = brheads

  ibeliefs = contexts[ctx['idx1']]['importedbeliefs']
  for (cidx,beliefs) in brbeliefs.iteritems():
    debug("ctx %d got br items ctx %d => %s" % (ctx['idx1'], cidx, beliefs))
    contexts[cidx]['exportedbeliefs'].update(beliefs)
    ibeliefs[cidx] = beliefs

debug("number of contexts = %d" % (len(contexts),))

###########################
# collect exportedbeliefs and lpbeliefs into beliefs and create indices as required by query plan
###########################
for ctx in contexts.itervalues():
  debug("context %d: %s" % (ctx['idx1'],ctx))
  ctx['beliefs'] = {}
  idx = 1
  for belief in ctx['lpbeliefs']:
    ctx['beliefs'][idx] = belief
    idx+= 1
  for belief in ctx['injectedbeliefs'] - ctx['lpbeliefs']:
    ctx['beliefs'][idx] = belief
    idx+= 1
  for belief in ctx['exportedbeliefs'] - ctx['injectedbeliefs'] - ctx['lpbeliefs']:
    ctx['beliefs'][idx] = belief
    idx+= 1
  debug("indexed beliefs of context %d: %s" % (ctx['idx1'],ctx['beliefs']))
  # reverse dict to get lookup of belief indices
  ctx['beliefidx'] = {}
  for (idx,bel) in ctx['beliefs'].iteritems():
    ctx['beliefidx'][bel] = idx

###########################
# calculate query mask
###########################
querymask = []
for ctxidx1 in range(1,len(contexts)+1):
  mask = 0
  if ctxidx1 == QUERYCTX:
    # take all beliefs
    for beliefidx in contexts[ctxidx1]['beliefs'].iterkeys():
      mask |= 1 << beliefidx
  for belief in contexts[ctxidx1]['exportedbeliefs']:
    mask |= 1 << (contexts[ctxidx1]['beliefidx'][belief])
  if mask != 0:
    mask |= 1 # epsilon bit
  debug("created mask 0x%x = %dd for ctx %d" % (mask, mask, ctxidx1))
  querymask.append(mask)
querymask = " ".join(map(str,querymask)) + " "

###########################
# calculate signatures
###########################
for ctx in contexts.itervalues():
  ctx['sigma'] = ",".join(map(\
      lambda x: "(%s %d %s %s)" % (x[1],ctx['idx1'],x[0],x[0]), \
    ctx['beliefs'].iteritems()))
  debug("sigma for ctx %d = '%s'" % (ctx['idx1'],ctx['sigma']))

###########################
# calculate dependencies
# first level of keys are contexts containing bridge rules
# second level of keys are contexts in body atoms
###########################
dependencies = {}
for ctx in contexts.itervalues():
  if len(ctx['importedbeliefs']) > 0:
    dependencies[ctx['idx0']] = {}
    for (cidx1,beliefs) in ctx['importedbeliefs'].iteritems():
      debug("processing dependency from ctx %d to ctx %d and beliefs %s" % (ctx['idx0'],cidx1-1,beliefs))
      thisdeps = {}
      dependencies[ctx['idx0']][cidx1-1] = thisdeps
      # preset
      for i in range(0,len(contexts)):
        thisdeps[i] = 0
      # epsilon bit
      thisdeps[cidx1-1] |= 1
      for belief in beliefs:
        # belief bit
        thisdeps[cidx1-1] |= 1 << (contexts[cidx1]['beliefidx'][belief])
      debug("dependencies for ctx %d -> ctx %d = '%s'" % \
        (ctx['idx1'],cidx1,thisdeps))

###########################
# do output of .top file
###########################
topfile = "%s.top" % (outputname,)
if os.path.exists(topfile):
  die("output file %s already exists!" % (topfile,))

ftop = open(topfile,"w+")
print >>ftop, 'digraph G {'
print >>ftop, 'graph [name="%s"]' % (querymask,)
for ctx in contexts.itervalues():
  print >>ftop, '%d [hostname="localhost", port="%d", sigma="%s"];' % \
    (ctx['idx0'],FIRSTPORT+ctx['idx0'],ctx['sigma'])
for (depfrom,depdict) in dependencies.iteritems():
  for (depto,depmasks) in depdict.iteritems():
    #debug(depmasks)
    print >>ftop, '%d->%d [interface="%s"];' % \
      (depfrom,depto,"".join(map(lambda x: "%d " % (x,),depmasks.itervalues())))
print >>ftop, '}'
ftop.close()

###########################
# do output of .sh file
###########################
shfile = "%s.sh" % (outputname,)
if os.path.exists(shfile):
  die("output file %s already exists!" % (shfile,))

fsh = open(shfile,"w+")
# context daemons
for ctx in contexts.itervalues():
  print >>fsh, '$DMCSPATH/dmcsd --context=%d --port=%d --kb=%s --br=%s --topology=%s >ctx%d.log 2>&1 &' % \
    (ctx['idx1'], FIRSTPORT+ctx['idx0'], ctx['lpfile'], ctx['brfile'], topfile, ctx['idx1'])
# wait for startup
print >>fsh, 'sleep 5'
# display daemon outputs
print >>fsh, 'tail -f ctx*.log &'
# querying client
print >>fsh, 'time $DMCSPATH/dmcsc --hostname=localhost --port=%d --system-size=%d --query-variables="%s"' % \
  (FIRSTPORT+QUERYCTX-1, len(contexts), querymask)
# kill daemons (TODO improve this!)
print >>fsh, 'killall dmcsd'
fsh.close()
