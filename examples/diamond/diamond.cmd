./dmcsd --logging=../../log4cxx.cfg --context=1 --port=5001 --kb=tests/diamond1.lp --br=tests/diamond1.br --topology=tests/diamond.opt
./dmcsd --logging=../../log4cxx.cfg --context=2 --port=5002 --kb=tests/diamond2.lp --br=tests/diamond2.br --topology=tests/diamond.opt
./dmcsd --logging=../../log4cxx.cfg --context=3 --port=5003 --kb=tests/diamond3.lp --br=tests/diamond3.br --topology=tests/diamond.opt
./dmcsd --logging=../../log4cxx.cfg --context=4 --port=5004 --kb=tests/diamond4.lp --br=tests/diamond4.br --topology=tests/diamond.opt
time ./dmcsc --hostname=localhost --port=5001 --system-size=4
