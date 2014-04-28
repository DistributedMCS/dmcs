./new_dmcsm --port=4999 --system-size=5

./new_dmcsd --context=0 --port=5000 --system-size=5 --packsize=4 --belief-state-size=10 --kb=../../examples/context0.lp --br=../../examples/context0.br --queryplan=../../examples/context0.qp --returnplan=../../examples/context0.rp --manager=localhost:4999

./new_dmcsd --context=1 --port=5001 --system-size=5 --packsize=4 --belief-state-size=10 --kb=../../examples/context1.lp --br=../../examples/context1.br --queryplan=../../examples/context1.qp --returnplan=../../examples/context1.rp --manager=localhost:4999

./new_dmcsd --context=2 --port=5002 --system-size=5 --packsize=4 --belief-state-size=10 --kb=../../examples/context2.lp --br=../../examples/context2.br --queryplan=../../examples/context2.qp --returnplan=../../examples/context2.rp --manager=localhost:4999

./new_dmcsd --context=3 --port=5003 --system-size=5 --packsize=4 --belief-state-size=10 --kb=../../examples/context3.lp --br=../../examples/context3.br --queryplan=../../examples/context3.qp --returnplan=../../examples/context3.rp --manager=localhost:4999

./new_dmcsd --context=4 --port=5004 --system-size=5 --packsize=4 --belief-state-size=10 --kb=../../examples/context4.lp --br=../../examples/context4.br --queryplan=../../examples/context4.qp --returnplan=../../examples/context4.rp --manager=localhost:4999

./new_dmcsc --port=5000 --root=0 --signature=../../examples/client.qp --belief-state-size=10 --k1=1 --k2=4




***********************

./new_dmcsd --context=0 --port=5000 --system-size=5 --packsize=0 --belief-state-size=10 --kb=../../examples/context0.lp --br=../../examples/context0.br --queryplan=../../examples/context0.qp --returnplan=../../examples/context0.rp --manager=localhost:4999

./new_dmcsd --context=1 --port=5001 --system-size=5 --packsize=0 --belief-state-size=10 --kb=../../examples/context1.lp --br=../../examples/context1.br --queryplan=../../examples/context1.qp --returnplan=../../examples/context1.rp --manager=localhost:4999

./new_dmcsd --context=2 --port=5002 --system-size=5 --packsize=0 --belief-state-size=10 --kb=../../examples/context2.lp --br=../../examples/context2.br --queryplan=../../examples/context2.qp --returnplan=../../examples/context2.rp --manager=localhost:4999

./new_dmcsd --context=3 --port=5003 --system-size=5 --packsize=0 --belief-state-size=10 --kb=../../examples/context3.lp --br=../../examples/context3.br --queryplan=../../examples/context3.qp --returnplan=../../examples/context3.rp --manager=localhost:4999

./new_dmcsd --context=4 --port=5004 --system-size=5 --packsize=0 --belief-state-size=10 --kb=../../examples/context4.lp --br=../../examples/context4.br --queryplan=../../examples/context4.qp --returnplan=../../examples/context4.rp --manager=localhost:4999

./new_dmcsc --port=5000 --root=0 --signature=../../examples/client.qp --belief-state-size=10 --k1=0 --k2=0