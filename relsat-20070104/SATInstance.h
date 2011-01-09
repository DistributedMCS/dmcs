#ifndef SATInstance_h
#define SATInstance_h

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SATInstance

/////////////////
// OS Includes

//////////////
// Includes
#include "ClauseList.h"
#include "VariableSet.h"

#include "mcs/Theory.h"


/////////////
// Defines

////////////////////////
// Class Declarations

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Class Definitions

class SATInstance : public ClauseList {
public:
  SATInstance(ostream& xOutputStream_) : xOutputStream(xOutputStream_) {}
  ~SATInstance(); 
  void vOutput() const;
  void vOutputDimacs(ostream&) const;

  boolean bReadDimacs(const char* aFileName_);
  boolean bReadDimacs(istream&);

  // Primary variable file format: list of (any kind of) whitespace separated variable numbers.
  VariableSet* pReadPrimaryVariables(const char* aFileName_);
  VariableSet* pReadPrimaryVariables(istream&);

  void vMakeRandom_3SAT(int iVars_, int iClauses_);

  int iVariableCount;

  ostream& xOutputStream;

  void
  readTheory(const dmcs::TheoryPtr& theory, std::size_t sig_size);

  void
  removeLastInput();

private:
  std::size_t orig_theory_size;
  std::size_t theory_w_conflict_size;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Inlines

#endif // SATInstance_h

// Local Variables:
// mode: C++
// End:
