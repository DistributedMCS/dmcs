#include <iostream>
#include "QueryPlan.h"

using namespace dmcs;

int main()
{
  QueryPlan qp;
  //std::istringstream
    //    my_input("digraph G { 0 [sigma=\"a0 b0 c0\", hostname=\"localhost\", port=\"5001\"] 1;\n 0 -> 1 [interface=\"2 3\"] }");

  std::istringstream
  my_input("digraph G { graph [name = \"0 0 0 0 \"] 0 [hostname=\"\", port=\"\", sigma=\"(a1 1 1 1),(b1 1 2 2),(c1 1 3 3),(d0 0 4 4)\"]; 1 [hostname=\"\", port=\"\", sigma=\"(a0 0 1 1),(b0 0 2 2),(c0 0 3 3),(d0 0 4 4)\"]; 0->1 [interface=\"0 0 1 \"];}");

  std::istringstream iss("(a0 1 1 1),(b1 2 2 2)");
  Signature sig;
  iss >> sig;
  std::cerr << sig << std::endl;

  // std::istringstream
  //   my_input("digraph G { graph [name = \"0 0 0 0 \"] 0 [hostname=\"(a0 0 1 1)\", port=\"\", sigma=\"\"]; 1 [hostname=\"\", port=\"\", sigma=\"(a0 0 1 1),(b0 0 2 2),(c0 0 3 3),(d0 0 4 4)\"]; 0->1 [interface=\"0 0 1 \"];}");




  qp.read_graph(my_input);
  std::string fn = "diamond-4-4-2-2-1.opt";
  //qp.read_graph(fn);

  std::cerr << "Write graph..." << std::endl;

  BeliefStatePtr V1(new BeliefState(4, 0));
  qp.putGlobalV(V1);
  qp.write_graph(std::cerr);

  Signature sig1;

  // we are in ctx 1 and populate \Sigma_1 = "a1 b1 c1 d1"
  sig1.insert(Symbol("a0",0,1,1));
  sig1.insert(Symbol("b0",0,2,2));
  sig1.insert(Symbol("c0",0,3,3));
  sig1.insert(Symbol("d0",0,4,4));

  qp.add_neighbor(1, 2);
  qp.putSignature(1, sig1);
  qp.putSignature(2, sig1);


  BeliefStatePtr V(new BeliefState(3));
  (*V)[2] = 1;

  qp.putInterface(1, 2, V);

  std::cerr << "Update graph" << std::endl;
  qp.write_graph(std::cerr);

  std::cerr << "Finish writing" << std::endl;
  //BeliefStatePtr T = qp.getInterface(1, 2);
  //std::cerr << T.belief_state_ptr->belief_state[1] << std::endl;
  std::cerr << qp.getInterface(1, 2) << std::endl;
  std::cerr << V << std::endl;
}
