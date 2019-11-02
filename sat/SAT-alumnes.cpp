#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>

using namespace std;

#define UNDEF -1
#define TRUE 1
#define FALSE 0

uint numVars;
uint numClauses;
vector<vector<int> > clauses;
vector<int> model;
vector<int> modelStack;
uint indexOfNextLitToPropagate;
uint decisionLevel;

// added

// occurs lists
struct litOccurs {
    vector<vector<int>* > t; // Point to clauses where literal appears True
    vector<vector<int>* > f; // Point to clauses where literal appears False
};
vector<litOccurs> occurLists;

// used by heuristic
vector<double> scores;

uint nConflicts;

void readClauses( ){
  // Skip comments
  char c = cin.get();
  while (c == 'c') {
    while (c != '\n') c = cin.get();
    c = cin.get();
  }
  
  // Read "cnf numVars numClauses"
  string aux;
  cin >> aux >> numVars >> numClauses;
  clauses.resize(numClauses);
  
  // init new data structures
  occurLists.resize(numVars+1);
  scores.resize(numVars + 1, 0.0);

  // Read clauses
  for (uint i = 0; i < numClauses; ++i) {
    int lit;
    while (cin >> lit and lit != 0) {
        clauses[i].push_back(lit);
        // Init occurLists
        if (lit > 0) {
            occurLists[lit].t.push_back(&clauses[i]);
        }
        else {
            occurLists[(-lit)].f.push_back(&clauses[i]);
        }
    }
  }
  nConflicts = 0;
}

int currentValueInModel(int lit){
  if (lit >= 0) return model[lit];
  else {
    if (model[-lit] == UNDEF) return UNDEF;
    else return 1 - model[-lit];
  }
}


void setLiteralToTrue(int lit) {
  modelStack.push_back(lit);
  if (lit > 0) {
      model[lit] = TRUE;
  }
  else {
      model[-lit] = FALSE;
  }
}

void incrementScores(const vector<int>& clause)
{
    for (uint i = 0; i < clause.size(); ++i)
    {
        scores[abs(clause[i])]+=1;
    }
}

bool checkConflicts(const vector<vector<int>* >& occurList)
{
    for (uint i = 0; i < occurList.size(); i++)
    {
        vector<int> clause = *occurList[i];
        bool someLitTrue = false;
        int numUndefs = 0, lastLitUndef = 0;
        for (uint j = 0; not someLitTrue and j < clause.size(); j++) {
            int val = currentValueInModel(clause[j]);
            if (val == TRUE) {
                someLitTrue = true;
            }
            else if (val == UNDEF) {
                ++numUndefs; lastLitUndef = clause[j];
            }
        }
        if (not someLitTrue and numUndefs == 0) {
            // increment score of all lits that appears in this clause
            incrementScores(clause);
            return true; // conflict! all lits false
        }
        else if (not someLitTrue and numUndefs == 1) {
            setLiteralToTrue(lastLitUndef);
        }
    }
    return false;
}

bool propagateGivesConflict () {
    while ( indexOfNextLitToPropagate < modelStack.size() ) {
        int litToPropagate = modelStack[indexOfNextLitToPropagate];
        if (litToPropagate > 0) {
//             now we visit clauses where this literal appears FALSE
                if (checkConflicts(occurLists[litToPropagate].f))
                    return true;
        }
        else {
//             now we visit clauses where this literal appears TRUE
                if (checkConflicts(occurLists[-litToPropagate].t))
                    return true;
        }
        ++indexOfNextLitToPropagate;
    }
    return false;
}

void backtrack(){
    ++nConflicts;
    if (nConflicts % 1000 == 0)
    {
        // we halve lits values
        for (uint i = 1; i <= numVars; i++)
        {
            scores[i] /= 2.0;
        }
    }
    uint i = modelStack.size() -1;
    int lit = 0;
    while (modelStack[i] != 0){ // 0 is the DL mark
        lit = modelStack[i];
        model[abs(lit)] = UNDEF;
        modelStack.pop_back();
        --i;
    }
    // at this point, lit is the last decision
    modelStack.pop_back(); // remove the DL mark
    --decisionLevel;
    indexOfNextLitToPropagate = modelStack.size();
    setLiteralToTrue(-lit);  // reverse last decision
}

// Heuristic for finding the next decision literal:
int getNextDecisionLiteral() {
    double maxScore = 0.0;
    int maxScoreLit = 0;
    for (uint i = 1; i <= numVars; ++i)
    {
        // check only undefined lits
        if (model[i] == UNDEF)
        {
            if (scores[i] >= maxScore)
            {
                maxScore = scores[i];
                maxScoreLit = i;
            }
        }
    }
    if (maxScoreLit == 0 || occurLists[maxScoreLit].t.size() < occurLists[maxScoreLit].f.size())
        return maxScoreLit;
    return -maxScoreLit;
}

void checkmodel(){
  for (uint i = 0; i < numClauses; ++i){
    bool someTrue = false;
    for (uint j = 0; not someTrue and j < clauses[i].size(); ++j)
      someTrue = (currentValueInModel(clauses[i][j]) == TRUE);
    if (not someTrue) {
      cout << "Error in model, clause is not satisfied:";
      for (uint j = 0; j < clauses[i].size(); ++j) cout << clauses[i][j] << " ";
      cout << endl;
      exit(1);
    }
  }  
}


int main() { 
  readClauses(); // reads numVars, numClauses and clauses
  model.resize(numVars+1,UNDEF);
  indexOfNextLitToPropagate = 0;
  decisionLevel = 0;
  
  // Take care of initial unit clauses, if any
  for (uint i = 0; i < numClauses; ++i)
    if (clauses[i].size() == 1) {
      int lit = clauses[i][0];
      int val = currentValueInModel(lit);
      if (val == FALSE) {cout << "UNSATISFIABLE" << endl; return 10;}
      else if (val == UNDEF) setLiteralToTrue(lit);
    }
  
  // DPLL algorithm
  while (true) {
    while ( propagateGivesConflict() ) {
      if ( decisionLevel == 0) { cout << "UNSATISFIABLE" << endl; return 10; }
      backtrack();
    }
    int decisionLit = getNextDecisionLiteral();
    if (decisionLit == 0) { checkmodel(); cout << "SATISFIABLE" << endl; return 20; }
    // start new decision level:
    modelStack.push_back(0);  // push mark indicating new DL
    ++indexOfNextLitToPropagate;
    ++decisionLevel;
    
    setLiteralToTrue(decisionLit);    // now push decisionLit on top of the mark
  }
}
