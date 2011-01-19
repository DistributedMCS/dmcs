/////////////////
// OS Includes
#include <fstream>
#include <iomanip>

//////////////
// Includes
#include "Clause.h"
#include "Random.h"
#include "SATInstance.h"
#include "SATSolver.h"

#include "dmcs/Log.h"

/////////////
// Defines

/////////////////////////////
// Static data initialization 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Methods

SATInstance::~SATInstance() 
{
  vDestroy(); 
}

void SATInstance::vOutputDimacs(ostream& xUseMe_) const
{
  xUseMe_ << "p cnf " << iVariableCount << ' ' << _iClauseCount << '\n';
  int i;
  for (i=0; i<_iClauseCount; i++) {
    Clause* pPrintMe = _aClause[i];
    if (!pPrintMe->bLearned()) {
      for (int j=0; j < pPrintMe->iVariableCount(); j++) {
	if (pPrintMe->iIsNegated(j))
	  xUseMe_ << "-";
	xUseMe_ << (int) (pPrintMe->eConstrainedVariable(j)+1) << " "; 
      }
      xUseMe_ << " 0\n";
    }
    xUseMe_ << flush;
  }

  xUseMe_ << "c NOTE: The following clauses are derived (non-essential):\n";
  for (i=0; i<_iClauseCount; i++) {
    Clause* pPrintMe = _aClause[i];
    if (pPrintMe->bLearned()) {
      for (int j=0; j < pPrintMe->iVariableCount(); j++) {
	if (pPrintMe->iIsNegated(j))
	  xUseMe_ << "-";
	xUseMe_ << (int) (pPrintMe->eConstrainedVariable(j)+1) << " "; 
      }
      xUseMe_ << " 0\n";
    }
    xUseMe_ << flush;
  }
}

VariableSet* SATInstance::pReadPrimaryVariables(const char* aFileName_)
{
  // Primary variable file format: list of (any kind of) whitespace separated variable numbers.
  // Call only after bReadDimacs.
  // The caller of this method is responsible for deleting the returned VariableSet.
  ifstream xInputFile(aFileName_);
  VariableSet* return_me =  pReadPrimaryVariables(xInputFile);
  xInputFile.close();
  return return_me;
}

VariableSet* SATInstance::pReadPrimaryVariables(istream& xInputFile)
{
  char buffer[5000];
  VariableID eVariableID;
  VariableSet* return_me = new VariableSet(iVariableCount);
  while (1) {
    xInputFile >> eVariableID;
    if (xInputFile.eof()) {
      return return_me;
    }
    if (eVariableID < 1 || eVariableID > iVariableCount) {
      xOutputStream << "\nError: Primary variable file contains unexpected variable id." << endl;
      return 0;
    }
    else {
      return_me->vAddVariable(eVariableID-1);
    }
  }
}

boolean SATInstance::bReadDimacs(const char* aFileName_)
{
  ifstream xInputFile(aFileName_);
  boolean return_me = bReadDimacs(xInputFile);
  xInputFile.close();
  return return_me;
}


void
SATInstance::readTheory(const dmcs::TheoryPtr& theory, std::size_t sig_size)
{
  // Transfer local theory to internal clause representation of relsat
  iVariableCount = sig_size;

  DMCS_LOG_TRACE("iVariableCount = " << iVariableCount);

  VariableSet xPositiveVariables(iVariableCount);
  VariableSet xNegativeVariables(iVariableCount);

  DMCS_LOG_DEBUG("Reading theory:");

  // travel through the theory
  for (dmcs::Theory::const_iterator it = theory->begin();
       it != theory->end(); ++it)
    {
      DMCS_LOG_DEBUG("Clause:");

      xPositiveVariables.vClear();
      xNegativeVariables.vClear();
      boolean bIgnoreMe = 0;

      // read one clause
      const dmcs::ClausePtr& cl = *it;
      for (dmcs::Clause::const_iterator jt = cl->begin(); jt != cl->end(); ++jt)
	{
	  VariableID eVar;
	  eVar = *jt;
	  assert (eVar <= iVariableCount);

	  DMCS_LOG_DEBUG(eVar);

	  if (eVar < 0)
	    {
	      if (xPositiveVariables.bHasVariable(0-(eVar+1))) 
		{
		  bIgnoreMe = 1; // Tautology
		}
	      else 
		{
		  xNegativeVariables.vAddVariable(0-(eVar+1));
		}
	    }
	  else
	    {
	      if (xNegativeVariables.bHasVariable(eVar-1)) 
		{
		  bIgnoreMe = 1; // Tautology
		}
	      else 
		{
		  xPositiveVariables.vAddVariable(eVar-1);
		}
	    }
	} // for (dmcs::Clause::const_iterator jt = ...

      if (!bIgnoreMe)
	{
	  assert(xNegativeVariables.iCount() + xPositiveVariables.iCount() > 0);
	  Clause* pNewConstraint = new Clause((VariableList&)xPositiveVariables, 
					      (VariableList&)xNegativeVariables,
					      1);
	  pNewConstraint->vSortVariableList();
	  vAddClause(pNewConstraint); // _iClauseCount is updated inside vAddClause
	}


    } // for (dmcs::Theory::const_iterator it = ...

  orig_theory_size = _iClauseCount;
  theory_w_conflict_size = orig_theory_size; // for the moment, when
					     // we haven't considered
					     // pushing conflicts
}



void
SATInstance::removeLastInput()
{
  vRemoveBack(theory_w_conflict_size);
}



void
SATInstance::add_unit_clause(int literal)
{
  VariableSet xPositiveVariables(iVariableCount);
  VariableSet xNegativeVariables(iVariableCount);
  
  assert (literal != 0);
  
  if (literal > 0)
    {
      assert (literal <= iVariableCount);
      xPositiveVariables.vAddVariable(literal-1);
    }
  else
    {
      assert (-literal <= iVariableCount);
      xNegativeVariables.vAddVariable(0-(literal+1));
    }

  Clause* pNewConstraint = new Clause((VariableList&)xPositiveVariables, 
				      (VariableList&)xNegativeVariables,
				      1);
  vAddClause(pNewConstraint); // _iClauseCount is updated inside vAddClause
}


boolean SATInstance::bReadDimacs(istream& xInputFile)
{
  xOutputStream << "c Reading instance..." << flush;
  char buffer[5000];
  VariableID eMaxVar = 0;
  int iClauseCount;
  char cCheck;

  while (1) {
    xInputFile >> cCheck;
    if (cCheck == 'c') {
      xInputFile.getline(buffer, 5000);
      continue;
    }
    else if (cCheck == 'p') {
      xInputFile >> setw(5000) >> buffer;
      xInputFile >> eMaxVar;
      xInputFile >> iClauseCount;
      break;
    }
    else {
      xOutputStream << "\nError: File not in DIMACS format?\n";
      return 0;
    }
  }
  VariableSet xPositiveVariables(eMaxVar);
  VariableSet xNegativeVariables(eMaxVar);

  iVariableCount = eMaxVar;
  int iWorkCount = 0;
  while (1) {
    if (xInputFile.eof()) {
      xOutputStream << "\nError: Unexpected end of file.\n";
      return 0;
    }
    xInputFile >> cCheck;
    if (cCheck == 'c') {
      xInputFile.getline(buffer, 5000);
      continue;
    }
    else xInputFile.putback(cCheck);

    xPositiveVariables.vClear();
    xNegativeVariables.vClear();
    boolean bIgnoreMe = 0;
    do {
      if (xInputFile.eof()) {
	xOutputStream << "\nError: Unexpected end of file.\n";
	return 0;
      }
      VariableID eVar;
      xInputFile >> eVar;
      if (eVar == 0)
	break;
      if (eVar > eMaxVar){
	xOutputStream << "\nError: some variable is numbered larger than the maximum.\n";
	return 0;
      }
      if (eVar < 0) {
	if (xPositiveVariables.bHasVariable(0-(eVar+1))) {
	  bIgnoreMe = 1;// Tautology
	}
	else {
	  xNegativeVariables.vAddVariable(0-(eVar+1));
	}
      }
      else {
	if (xNegativeVariables.bHasVariable(eVar-1)) {
	  bIgnoreMe = 1;// Tautology
	}
	else {
	  xPositiveVariables.vAddVariable(eVar-1);
	}
      }
    } while (1);

    if (!bIgnoreMe) {
      if (xNegativeVariables.iCount() + xPositiveVariables.iCount() == 0) {
	xOutputStream << "\nError: encountered a 0 length clause \n" << endl ;
	return 0;
      }
      // Silly up-casting needed to keep xlC compiler happy
      Clause* pNewConstraint = new Clause((VariableList&)xPositiveVariables, 
					  (VariableList&)xNegativeVariables,
					  1);
      pNewConstraint->vSortVariableList();
      vAddClause(pNewConstraint);
      iWorkCount++;
    }
    else iClauseCount--;
    if (iWorkCount == iClauseCount)
      break;
  }
  xOutputStream << "..done." << endl; 
  return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Protected Methods

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Methods
