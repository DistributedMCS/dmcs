#!/usr/bin/python

# convert dmcs .lp and .br files into query plan: .top and .sh file

# some assumptions/configuration:

# QUERYCTX: 1-based index of context where we want to see all beliefs = where we query
QUERYCTX=1

# FIRSTPORT: network port where we want the first context to operate, other contexts use ports above
FIRSTPORT=5001

# CREATE_TREE_OPT: create .opt output (this only works if the underlying MCS is tree-shaped!
CREATE_TREE_OPT=True

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
  beliefs = []
  if ctxidx1 == QUERYCTX:
    # take all belief indices
    beliefs = list(contexts[ctxidx1]['beliefs'].iterkeys())
  else:
    for belief in contexts[ctxidx1]['exportedbeliefs']:
      beliefs.append( contexts[ctxidx1]['beliefidx'][belief] )
  if len(beliefs) != 0:
    beliefs.append(0) # epsilon 'bit'
  debug("beliefs for ctx %d: '%s'" % (ctxidx1,beliefs))
  querymask.append("{%s}" % (" ".join(map(str,beliefs)),))

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
# values are again another level
###########################
dependencies = {}
if CREATE_TREE_OPT:
  tree_opt_dependencies = {}
for ctx in contexts.itervalues():
  if len(ctx['importedbeliefs']) > 0:
    ctxdeps = {}
    dependencies[ctx['idx0']] = ctxdeps
    for (cidx1,beliefs) in ctx['importedbeliefs'].iteritems():
      debug("processing dependency from ctx %d to ctx %d and beliefs %s" % (ctx['idx0'],cidx1-1,beliefs))
      thisdeps = {}
      # preset
      for i in range(0,len(contexts)):
        thisdeps[i] = []
      # beliefs
      for belief in beliefs:
        thisdeps[cidx1-1].append(contexts[cidx1]['beliefidx'][belief])
      # epsilon bit(s)
      for dep in thisdeps.itervalues():
        if len(dep) != 0:
          dep.append(0) # epsilon 'bit'
      debug("converted dependencies for ctx %d to %d = '%s'" % \
        (ctx['idx1'],cidx1,ctxdeps))
      # map lists of ints to "{i1 i2 i3}" format
      keys = thisdeps.keys()
      keys.sort
      sorteddeps = []
      for idx in keys:
        singledep = thisdeps[idx]
        debug("singledep %s (should be a list)" % (singledep,))
        sorteddeps.append("{%s}" % (" ".join(map(str,singledep)),))
      # map hash of third level into single string
      ctxdeps[cidx1-1] = " ".join(sorteddeps)
      debug("final dependencies for ctx %d to %d = '%s'" % \
        (ctx['idx1'],cidx1,ctxdeps[cidx1-1]))
    debug("dependencies for ctx %d = '%s'" % \
      (ctx['idx1'],ctxdeps))

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
  for (depto,deps) in depdict.iteritems():
    debug("from %s to %s dep %s" % (depfrom, depto, deps))
    print >>ftop, '%d->%d [interface="%s"];' % (depfrom,depto,deps)
print >>ftop, '}'
ftop.close()

###########################
# do output of .top.sh file
###########################
topshfile = "%s.top.sh" % (outputname,)
if os.path.exists(topshfile):
  die("output file %s already exists!" % (topshfile,))

ftopsh = open(topshfile,"w+")
# reset logfiles
print >>ftopsh, 'rm ctx*.log'
# context daemons
for ctx in contexts.itervalues():
  print >>ftopsh, '$DMCSPATH/dmcsd --context=%d --port=%d --kb=%s --br=%s --topology=%s >ctx%d.log 2>&1 &' % \
    (ctx['idx1'], FIRSTPORT+ctx['idx0'], ctx['lpfile'], ctx['brfile'], topfile, ctx['idx1'])
# wait for startup
print >>ftopsh, 'sleep 1'
# display daemon outputs
print >>ftopsh, 'tail -f ctx*.log &'
# wait for tail-f startup
print >>ftopsh, 'sleep 1'
# querying client
print >>ftopsh, 'time $DMCSPATH/dmcsc --hostname=localhost --port=%d --system-size=%d --streaming=0 --query-variables="%s"' % \
  (FIRSTPORT+QUERYCTX-1, len(contexts), querymask)
# kill all backgrounded jobs
print >>ftopsh, 'jobs -p |xargs kill'
ftopsh.close()

if CREATE_TREE_OPT:
  ###########################
  # do output of .opt file
  ###########################
  optfile = "%s.opt" % (outputname,)
  if os.path.exists(optfile):
    die("output file %s already exists!" % (optfile,))

  fopt = open(optfile,"w+")
  print >>fopt, 'digraph G {'
  print >>fopt, 'graph [name="%s"]' % (querymask,)
  for ctx in contexts.itervalues():
    print >>fopt, '%d [hostname="localhost", port="%d", sigma="%s"];' % \
      (ctx['idx0'],FIRSTPORT+ctx['idx0'],ctx['sigma'])
  for (depfrom,depdict) in dependencies.iteritems():
    for (depto,deps) in depdict.iteritems():
      debug("from %s to %s dep %s" % (depfrom, depto, deps))
      print >>fopt, '%d->%d [interface="%s"];' % (depfrom,depto,deps)
  print >>fopt, '}'
  fopt.close()

  ###########################
  # do output of .opt.sh file
  ###########################
  optshfile = "%s.opt.sh" % (outputname,)
  if os.path.exists(optshfile):
    die("output file %s already exists!" % (optshfile,))

  foptsh = open(optshfile,"w+")
  # reset logfiles
  print >>foptsh, 'rm ctx*.log'
  # context daemons
  for ctx in contexts.itervalues():
    print >>foptsh, '$DMCSPATH/dmcsd --context=%d --port=%d --kb=%s --br=%s --topology=%s >ctx%d.log 2>&1 &' % \
      (ctx['idx1'], FIRSTPORT+ctx['idx0'], ctx['lpfile'], ctx['brfile'], optfile, ctx['idx1'])
  # wait for startup
  print >>foptsh, 'sleep 1'
  # display daemon outputs
  print >>foptsh, 'tail -f ctx*.log &'
  # wait for tail-f startup
  print >>foptsh, 'sleep 1'
  # querying client
  print >>foptsh, 'time $DMCSPATH/dmcsc --hostname=localhost --port=%d --system-size=%d --streaming=1 --packsize=100' % \
    (FIRSTPORT+QUERYCTX-1, len(contexts))
  # kill all backgrounded jobs
  print >>foptsh, 'jobs -p |xargs kill'
  foptsh.close()
