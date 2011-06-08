#!/bin/bash

LASTDIR=`pwd`
cd branchdir

rm dia-4-4-2-2.{top,sh}

${LASTDIR}/create_query_plan.py \
  dia-4-4-2-2 \
  dia-4-4-2-2-1-1.{lp,br} \
  dia-4-4-2-2-1-2.{lp,br} \
  dia-4-4-2-2-1-3.{lp,br} \
  dia-4-4-2-2-1-4.{lp,br}
