/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() { _gateList.push_back(_const0); }
    ~CirMgr() {
        reset();
    }

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { 
      // CirGate* tmp = NULL;
      // if (gid < _gateList.size()) tmp = _gateList[gid];
      // return tmp;
      return _gateList.at(gid);
   }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

   void genDFSList(GateList&) const;
   void genAigList(GateList&) const;
   void setgateList(size_t ind, CirGate* p){
      _gateList[ind] = p;
   }

   static CirGate* getGcc() { return _const0;}
private:
   static CirGate     *_const0;
   ofstream           *_simLog;
   unsigned M;
   unsigned I;
   unsigned L;
   unsigned O;
   unsigned A;
   GateList _piList;
   GateList _poList;
   GateList _gateList;
   GateList _dfsList;
   GateList _floatList;
   GateList _unusedList;
   // GateList _aigList;

   void reset();
   void readHeader(ifstream&);
   void readInput(ifstream&);
   void readOutput(ifstream&);
   void readAig(ifstream&);
   void readSymbol(ifstream&);
   void readComment(ifstream&);
   void connect();
   void genFnUList(GateList&,GateList&) const;  


   void Sim(const vector<size_t>&);
   bool readSimfile(ifstream &, vector<size_t>&, vector<string>&);
   size_t genMagicTime();
   void genRanSim(vector<string>&, vector<size_t>&);
   void writeSim(vector<string>&);
   void resetFEC();
   void initFEC();
   void destructFEC();
   void FECsort() const;
   // void UpdateUNSAT();
   void RenewFECs();
   void genProofModel(SatSolver&);
};

#endif // CIR_MGR_H
