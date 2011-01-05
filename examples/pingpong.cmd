./dmcsd --context=1 --port=5001 --kb=tests/pingpong1.lp --br=tests/pingpong1.br --topology=tests/pingpong.opt
./dmcsd --context=2 --port=5002 --kb=tests/pingpong2.lp --br=tests/pingpong2.br --topology=tests/pingpong.opt
time ./dmcsc --hostname=localhost --port=5001 --system-size=2
