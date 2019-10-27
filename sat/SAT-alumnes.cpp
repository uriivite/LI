#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <map>

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
map<int, int> scores; // literal counter
vector<bool> trueClauses;
struct litOccurs {
    vector<int> t; // Claues where literal appears True
    vector<int> f; // Clauses where literal appears False
};

vector<litOccurs> occurLists;

/*
 Falta per fer:
    2) Millorar l'heuristic de decisio:
        Cal escollir bé QUINA, no és tan important si comencem amb certa o falsa
        Preferiblement escollir min(#aparicionsCertes, #aparicionsFalses)
            Es pot fer estàticament
            Es pot fer dinamicament
                ignorar les clausules que son certes
                Donar prioritat a les clàusules binaries
        Comptar les vegades score(lit) que un literal ha aparegut en un conflicte
            Es pot millorar dividint per dos score(lit) a cada iteracio, aix
 */


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

  trueClauses.resize(numClauses, false);
  // set occurLists size
  occurLists.resize(numVars);
  // init literals scores to 0
  for (int i = 0; i < numVars; i++)
  {
    scores[i] = 0;
  }
  // Read clauses
  for (uint i = 0; i < numClauses; ++i) {
    int lit;
    while (cin >> lit and lit != 0) {
        clauses[i].push_back(lit);
        // Init occurLists
        if (lit > 0)
            occurLists[lit - 1].t.push_back(i);
        else
            occurLists[(-lit) - 1].f.push_back(i);
    }
  }
}


// Retorna l'estat actual del literal: TRUE, FALSE ó UNDEF
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
      scores[lit - 1]++;
  }
  else {
      model[-lit] = FALSE;
      scores[(-lit) - 1]++;
  }
}

bool checkConflicts(const vector<int>& occurList)
{
    for (uint i = 0; i < occurList.size(); i++)
    {
        int clause = occurList[i];
        
        bool someLitTrue = false;
        int numUndefs = 0, lastLitUndef = 0;
        for (uint j = 0; not someLitTrue and j < clauses[clause].size(); j++) {
            int val = currentValueInModel(clauses[clause][j]);
            if (val == TRUE) {
                someLitTrue = true;
            }
            else if (val == UNDEF) {
                ++numUndefs; lastLitUndef = clauses[clause][j];
            }
        }
        if (not someLitTrue and numUndefs == 0) return true; // conflict! all lits false
        else if (not someLitTrue and numUndefs == 1) setLiteralToTrue(lastLitUndef);
    }
    return false;
}

bool propagateGivesConflict () {
    while ( indexOfNextLitToPropagate < modelStack.size() ) {
        int litToPropagate = modelStack[indexOfNextLitToPropagate];
        if (litToPropagate > 0) {
//             now we visit clauses where this literal appears FALSE
                if (checkConflicts(occurLists[litToPropagate - 1].f))
                    return true;
        }
        else {
//             now we visit clauses where this literal appears TRUE
                if (checkConflicts(occurLists[-litToPropagate - 1].t))
                    return true;
        }
        ++indexOfNextLitToPropagate;
    }
    return false;
}

void backtrack(){
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
    // Actually: Get literal which appears on most clauses
    uint max = 0;
    bool firstFound = false;
    for (uint i = 1; i <= numVars; ++i)
    {
        if (model[i] == UNDEF && firstFound)
        {
            if ((occurLists[i - 1].t.size() + occurLists[i - 1].f.size()) > (occurLists[max - 1].t.size() + occurLists[max - 1].f.size()))
                max = i;
        }
        else if (model[i] == UNDEF && not firstFound)
        {
            max = i;
            firstFound = true;
        }
    }
    // Objective: Get literal with better score
    // Objective: Ignore true clauses
    if (max == 0)
        return 0;
    if (occurLists[max - 1].t.size() < occurLists[max - 1].f.size())
        return max;
    else
        return -max;
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
