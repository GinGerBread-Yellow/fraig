/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"
#include <map>

using namespace std;
extern vector <vector<CirGate::CirGateV> *> FECgrps;
// extern bool FECisSort;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
map<CirGate::CirGateV,Var>   Cirgate2Var;
void UpdateUNSAT();
/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static void
genProofModel( SatSolver& s, GateList &dfsList)
{
   // Allocate and record variables; No Var ID for POs
	size_t n = dfsList.size();
	// Cirgate2Var.resize(n);
	for (size_t i = 0; i < n; ++i) {
			Var v = s.newVar();
			// Cirgate2Var[dfsList[i]->getID()] = v;
      Cirgate2Var.insert(pair<CirGate::CirGateV,Var>(dfsList[i],v));
	}
	for (size_t i = 0; i < n; ++i){
		if (dfsList[i]->isAig()){
			CirGate::CirGateV f1,f2;
			dfsList[i]->getfanin(f1,f2);
			s.addAigCNF(Cirgate2Var[dfsList[i]],
							Cirgate2Var[f1],f1.isInv(),
							Cirgate2Var[f2],f2.isInv());
		}
	}
}
static bool
FecSatTest(SatSolver &solver, const CirGate::CirGateV &gate1, const CirGate::CirGateV &gate2)
{
   bool result;
   // k = Solve(Gate(5) ^ !Gate(8))
   Var newV = solver.newVar();
   solver.addXorCNF(newV, Cirgate2Var[gate1], gate1.isInv(),
									Cirgate2Var[gate2], gate2.isInv());
   solver.assumeRelease();  // Clear assumptions
   solver.assumeProperty(newV, true);  // k = 1
   result = solver.assumpSolve();
   // reportResult(solver, result);
	return result;
   // cout << endl << endl << "======================" << endl;
}
static bool
FECconstTest(SatSolver &solver, const CirGate::CirGateV &gate)
{
   solver.assumeRelease();  // Clear assumptions
	solver.assumeProperty(Cirgate2Var[gate], gate.inv().isInv());
   return solver.assumpSolve();
}
/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
  bool _changeDFS = false;
  size_t n = _dfsList.size();
  CirGate::CirGateV i0,i1;
  CirGate *MergeGate;
  HashMap<HashKey, CirGate*> myhash(getHashSize(n));
  for (unsigned i = 0; i < n; ++i){
    if (_dfsList[i]->getfanin(i0,i1)){
      HashKey gate(i0,i1);
      if (myhash.query(gate,MergeGate)){
        cout << "Strashing: " << MergeGate -> getID() << " merging " << _dfsList[i]->getID() << "..." << endl;
        _dfsList[i] -> replaceSelf(MergeGate);
        delete _dfsList[i];
        --A;
        _changeDFS = true;
      }
      else{
        myhash.insert(gate,_dfsList[i]);
      }
    }
    
  }
  if (_changeDFS){
    _dfsList.clear();
    genDFSList(_dfsList);
  }
  return;
}

void
CirMgr::fraig()
{
  if (FECgrps.empty()) return;
  // else if (!FECisSort) FECsort();
  SatSolver solver;
	solver.initialize();
	::genProofModel(solver, _dfsList);
  CirGate::CirGateV first = FECgrps[0]->at(0);
	CirGate::CirGateV second;
  unsigned i = 0;
  unsigned n = FECgrps.size();
  /*
  for (unsigned s = 0, m = _dfsList.size(); s  < m; ++s){
    unsigned count = 0;
    vector<CirGate::CirGateV> * p;
    if (_dfsList[s]->hasFECs()){
      p = _dfsList[s]->getFECs();
      if (p -> at(0).isConst0())
        doConstTest(_dfsList[s]);
      else
        doNormalTest(_dfsList[s]);
      ++count;
      if (count>>10) break;
    }
  }
  */
  if (first.isConst0()){
    for (unsigned start = FECgrps[0]->size()-1; start >= 1; --start){
      second = FECgrps[i]->at(start);
		cout << "Proving "<<second.getID()<<" = "
				<< (second.isInv()? '0':'1') << "...";
		cout.flush();
      if (!::FECconstTest(solver, second)){
        cout << "UNSAT!!\n" << endl;
      }
      else
      {
			cout << "SAT!!\n" << endl;
			FECgrps[0]->erase(FECgrps[0]->begin()+start);
      second.setFECs(0);
      }
      
    }
    ++i;
  }
  unsigned size;
	while (i < n){
    first = FECgrps[i]->at(0);
    size = FECgrps[i]->size()-1;
    for (;size >=1; --size) {
      second = FECgrps[i]->at(size);
		cout << "Proving ("<<first.getID()<<", ";
        if (second.isInv()) cout << '!';
        cout << second.getID() <<")...";
		cout.flush();
      if (!::FecSatTest(solver, first, second) ){
        cout << "UNSAT!!\n" << endl;
		  --A;
      }
      else{
        cout << "SAT!!\n" << endl;
        	FECgrps[i]->erase(FECgrps[i]->begin()+size);
          second.setFECs(0);
      }
    }
    // 
		++i;
		// ++count;
		// if (count>>8){
		// 	UpdateUNSAT(last_start,i);
		// 	last_start = i;
		// 	count = 0;
		// }

	}
  UpdateUNSAT();
  destructFEC();
  Cirgate2Var.clear();
	_dfsList.clear();
	genDFSList(_dfsList);
  
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void
UpdateUNSAT(){
  CirGate::CirGateV first;
	CirGate::CirGateV second;
  for (unsigned i = 0, n = FECgrps.size(); i < n; ++i){
    first = FECgrps[i]->at(0);
    unsigned m = FECgrps[i]->size();
    if (m < 2)continue;
    for (unsigned j = 1; j < m; ++j){
      second = FECgrps[i]->at(j);
      cout << "Fraig: " << first.getID() << " merging ";
      if (second.isInv()) cout << '!';
      cout << second.getID() << "..." << endl;
      // --A;
      second = FECgrps[i]->at(j);
      if (second.isInv())
        second.gate() -> replaceSelf(first.inv());
      else
        second.gate() -> replaceSelf(first);
      delete second.gate();
    }
    
  }
  cout << "Updating by UNSAT... ";
  cout.flush();      
}