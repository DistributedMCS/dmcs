$DMCSPATH/dmcsd 1 localhost 5001 $TESTSPATH/dia-4-4-2-2-1-1.lp $TESTSPATH/dia-4-4-2-2-1-1.br $TESTSPATH/dia-4-4-2-2-1.opt >/dev/null 2>&1 &
$DMCSPATH/dmcsd 2 localhost 5002 $TESTSPATH/dia-4-4-2-2-1-2.lp $TESTSPATH/dia-4-4-2-2-1-2.br $TESTSPATH/dia-4-4-2-2-1.opt >/dev/null 2>&1 &
$DMCSPATH/dmcsd 3 localhost 5003 $TESTSPATH/dia-4-4-2-2-1-3.lp $TESTSPATH/dia-4-4-2-2-1-3.br $TESTSPATH/dia-4-4-2-2-1.opt >/dev/null 2>&1 &
$DMCSPATH/dmcsd 4 localhost 5004 $TESTSPATH/dia-4-4-2-2-1-4.lp $TESTSPATH/dia-4-4-2-2-1-4.br $TESTSPATH/dia-4-4-2-2-1.opt >/dev/null 2>&1 &
time $DMCSPATH/dmcsc localhost 50014
killall dmcsd
