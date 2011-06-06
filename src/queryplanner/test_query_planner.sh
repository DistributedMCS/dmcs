#!/bin/bash

rm dia-4-4-2-2.top

./create_query_plan.py dia-4-4-2-2 \
  ../../examples/dia-4-4-2-2-1-1.{lp,br} \
  ../../examples/dia-4-4-2-2-1-2.{lp,br} \
  ../../examples/dia-4-4-2-2-1-3.{lp,br} \
  ../../examples/dia-4-4-2-2-1-4.{lp,br}
