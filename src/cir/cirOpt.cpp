/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
	vector<bool> IndfsList;
	bool change = false;
	size_t m = _gateList.size();
	IndfsList.resize(m,false);
	for (unsigned i = 0, n = _dfsList.size(); i < n; ++i)
		IndfsList[_dfsList[i]->getID()] = true;
	for (int i = m-1; i >= 0; --i){
		if (!IndfsList[i] && _gateList[i] && _gateList[i] -> sweep()){
			cout << "Sweeping: " << _gateList[i] ->getTypeStr() << '(' << _gateList[i] ->getID() << ')' << " removed..." << endl;
			if (_gateList[i]->isAig()) 
				--A;
			delete _gateList[i];
			_gateList[i] = 0;
			change = true;
		}
	}
	if (change){
		_floatList.clear();
		_unusedList.clear();
		genFnUList(_floatList,_unusedList);
	}
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
	bool _changeDFS = false;
	unsigned tmp;
	CirGate::CirGateV t = (size_t)0;
	for (unsigned i = 0, n = _dfsList.size(); i<n; ++i){
		if (_dfsList[i] -> optimize(t)){
			tmp = _dfsList[i]->getID();
			cout << "Simplifying: " << (t.getID()) << " merging ";
			if (t.isInv())cout << '!';
			cout << tmp << "..." << endl;
			_dfsList[i] -> replaceSelf(t);
			delete _dfsList[i];
			_dfsList[i] = 0;
			--A;
			_changeDFS = true;
		}
	}
	
	if (_changeDFS){
		_dfsList.clear();
		genDFSList(_dfsList);
	}


}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
