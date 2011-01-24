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


  /* Organization of additional clauses:

     original theory | conflict clauses | partial ass (unit clauses) | input (unit clauses) |
                     ^                  ^                            ^                      ^
		     |                  |                            |                      |
		     |                  |                            |                      |
             orig_theory_size     size_w_conflict           size_w_partial_ass         size_w_input = _iClauseCount

   */

  void
  removeInput()
  {
    vRemoveBack(size_w_partial_ass);
  }

  void
  removePartialAss()
  {
    vRemoveBack(size_w_conflict);
  }

  void
  removeConflicts()
  {
    vRemoveBack(orig_theory_size);
  }

  void
  setOrigTheorySize(std::size_t s)
  {
    orig_theory_size = s;
  }

  void
  setSizeWConflict(std::size_t s)
  {
    size_w_conflict = s;
  }

  void
  setSizeWPartialAss(std::size_t s)
  {
    size_w_partial_ass = s;
  }

  void
  add_unit_clause(int literal);

private:
  std::size_t orig_theory_size;
  std::size_t size_w_conflict;
  std::size_t size_w_partial_ass;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Inlines

#endif // SATInstance_h

// Local Variables:
// mode: C++
// End:
