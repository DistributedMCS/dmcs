#include "Rule.h"
#include "PropositionalASPGrammar.h"
#include "BRGrammar.h"
#include "ParserDirector.h"
#include "LocalKBBuilder.h"
#include "BridgeRulesBuilder.h"

using namespace dmcs;

BiMapId2Name id2name;
RulesPtr rules(new Rules);
BridgeRulesPtr bridge_rules(new BridgeRules);
SignaturePtr sig(new Signature);
BridgeAtomSetPtr br(new BridgeAtomSet);

void
print_rules()
{
  std::cerr << "Local KB:" << std::endl;
  for (std::list<RulePtr>::const_iterator it = rules->begin(); it != rules->end(); ++it)
    {
      std::cerr << "Head: ";
      RulePtr r = *it;
      for (std::list<Atom>::const_iterator jt = r->first->begin(); jt != r->first->end(); ++jt)
	{
	  std::cerr << *jt << " ";
	}
      std::cerr << std::endl;

      std::cerr << "Positive body: ";
      PositiveBodyPtr pb = r->second.first;
      for (std::list<Atom>::const_iterator jt = pb->begin(); 
	   jt != pb->end(); ++jt)
	{
	  std::cerr << *jt << " ";
	}
      std::cerr << std::endl;

      std::cerr << "Negative body: ";
      NegativeBodyPtr nb = r->second.second;
      for (std::list<Atom>::const_iterator jt = nb->begin(); 
	   jt != nb->end(); ++jt)
	{
	  std::cerr << *jt << " ";
	}
      std::cerr << std::endl;
    }
}

void
print_bridge_rules()
{
  std::cerr << std::endl << "Bridge rules:" << std::endl;
  for (std::list<BridgeRulePtr>::const_iterator it = bridge_rules->begin(); it != bridge_rules->end(); ++it)
    {
      std::cerr << "Head: ";
      BridgeRulePtr r = *it;
      for (std::list<Atom>::const_iterator jt = r->first->begin(); jt != r->first->end(); ++jt)
	{
	  std::cerr << *jt << " ";
	}
      std::cerr << std::endl;

      std::cerr << "Positive body: ";
      PositiveBridgeBodyPtr pb = r->second.first;
      for (std::list<BridgeAtomPtr>::const_iterator jt = pb->begin(); 
	   jt != pb->end(); ++jt)
	{
	  BridgeAtomPtr kt = *jt;
	  std::cerr << "(" << kt->first << "," << kt->second << ") ";
	}
      std::cerr << std::endl;

      std::cerr << "Negative body: ";
      NegativeBridgeBodyPtr nb = r->second.second;
      for (std::list<BridgeAtomPtr>::const_iterator jt = nb->begin(); 
	   jt != nb->end(); ++jt)
	{
	  BridgeAtomPtr kt = *jt;
	  std::cerr << "(" << kt->first << "," << kt->second << ") ";
	}
      std::cerr << std::endl;
    }
}

void
print_signatures()
{
  std::cerr << "Signature" << std::endl;
  for (Signature::const_iterator i = sig->begin(); i != sig->end(); ++i)
    {
      std::cerr << *i << std::endl;
    }
  std::cerr << std::endl << std::endl << "Bridge Atom Set" << std::endl;

  for (BridgeAtomSet::const_iterator i = br->begin(); i != br->end(); ++i)
    {
      BridgeAtomPtr kt = *i;
      std::cerr << "(" << kt->first << "," << kt->second << ") " << std::endl;
    }
}

int main(int argc, char* argv[])
{
  if (argc != 3)
    {
      std::cerr << "Usage: " << argv[0] << " LOCALKB_FILENAME BRIDGE_RULES_FILENAME" << std::endl;
      return 1;
    }

  LocalKBBuilder<PropositionalASPGrammar> builder3(rules, id2name, sig);
  ParserDirector<PropositionalASPGrammar> parser_director3;
  parser_director3.setBuilder(&builder3);
  parser_director3.parse(argv[1]);

  BridgeRulesBuilder<BRGrammar> builder4(bridge_rules, sig, br);
  ParserDirector<BRGrammar> parser_director4;
  parser_director4.setBuilder(&builder4);
  parser_director4.parse(argv[2]);

  print_rules();

  print_bridge_rules();

  print_signatures();
}
