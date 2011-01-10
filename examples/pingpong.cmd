./src/dmcsd --context=1 --port=5001 --kb=examples/pingpong1.lp --br=examples/pingpong1.br --topology=examples/pingpong.opt
./src/dmcsd --context=2 --port=5002 --kb=examples/pingpong2.lp --br=examples/pingpong2.br --topology=examples/pingpong.opt
time ./src/dmcsc --hostname=localhost --port=5001 --system-size=2
