#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>

using namespace boost;
using namespace dmcs;
using namespace dmcs::generator;

struct sigma_t
{
  typedef vertex_property_tag kind;
};

struct interface_t
{
  typedef edge_property_tag kind;
};

typedef property < vertex_name_t, std::string,
		   property < sigma_t, std::string > > vertex_p;

typedef property<interface_t, std::string> edge_p;

typedef adjacency_list < vecS, vecS, directedS, vertex_p, edge_p> QueryPlan;


template <class Name>
class query_plan_writer {
public:
  query_plan_writer(Name name_, const std::string& label_) 
    : label(label_), name(name_)
  { }

  template <class VertexOrEdge>
  void operator()(std::ostream& out, const VertexOrEdge& v) const {
    out << "[" << label << "=\"" << name[v] << "\"]";
  }
private:
  std::string label;
  Name name;
};

template < class Name >
query_plan_writer<Name>
make_query_plan_writer(Name n, const std::string& l)
{
  return query_plan_writer<Name>(n, l);
}


int main()
{

// Construct an empty graph and prepare the dynamic_property_maps.

QueryPlan qp(0);
dynamic_properties dp;

 property_map<QueryPlan, vertex_name_t>::type name = get(vertex_name, qp);
 dp.property("node_id", name);

 property_map<QueryPlan, sigma_t>::type sigma = get(sigma_t(), qp);
 dp.property("sigma", sigma);

 property_map<QueryPlan, interface_t>::type interface = get(interface_t(), qp);
 dp.property("interface", interface);

 // Sample graph as an std::istream;
 std::istringstream
   gvgraph("digraph G { 0 1 [sigma=\"a1 b1 c1\"] ; 0 -> 1 [interface=\"a2 b3\"] }");
 
 read_graphviz(gvgraph,qp,dp,"node_id");
 /*
  QueryPlan qp1;//(5);
 dynamic_properties dp;
 add_edge(1, 2, qp1);
 add_edge(1, 3, qp1);
 add_edge(2, 4, qp1);
 add_edge(3, 4, qp1);

 property_map<QueryPlan, vertex_name_t>::type name = get(vertex_name, qp1);
 dp.property("node_id", name);

 property_map<QueryPlan, sigma_t>::type sigma1 = get(sigma_t(), qp1);
 property_map<QueryPlan, interface_t>::type interface1 = get(interface_t(), qp1);

 put(sigma1, 1, "a1 b1 c1");
 put(sigma1, 2, "a2 b2 c2");
 put(sigma1, 3, "a3 b3 c3");
 sigma1[4] = "a4 b4 c4";


 graph_traits<QueryPlan>::edge_iterator ei, ei_end;
 for (tie(ei, ei_end) = edges(qp1); ei != ei_end; ++ei)
   {
     interface1[*ei] = "a2 b2";
   }

 write_graphviz(std::cout, qp1, 
		make_query_plan_writer(sigma1, "sigma"), 
		make_query_plan_writer(interface1, "interface"));

*/
}
