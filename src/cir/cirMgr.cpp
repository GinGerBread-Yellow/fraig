/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;
CirGate* CirMgr::_const0 = new ConstGate(0,0);
extern vector <vector<CirGate::CirGateV> *> FECgrps;
// extern bool FECisSort;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
/*static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}
*/

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   ifstream fin = ifstream(fileName);
   if (!fin) return false;
   readHeader(fin);
   readInput(fin);
   readOutput(fin);
   readAig(fin);
   readSymbol(fin);
   readComment(fin);
   connect();
   fin.close();
   genDFSList(_dfsList);
   genFnUList(_floatList,_unusedList);
   return true;
}


/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout  << "\nCircuit Statistics" << "\n"
         << "==================" << "\n"
         << "  PI" << setw(12) << right << I << "\n"
         << "  PO" << setw(12) << right << O << "\n"
         << "  AIG" << setw(11) << right << A << "\n"
         << "------------------" << "\n"
         << "  Total" << setw(9) << right << I+O+A << endl;
   return;
}
void
CirMgr::printNetlist() const
{
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGate();
   }
}


void
CirMgr::printFECPairs() const
{
   
   CirGate::CirGateV f;
   if (FECgrps.empty()) return;
   for (unsigned i = 0, n = FECgrps.size(); i < n; ++i){
      cout << '[' << i << ']';
      for (unsigned j = 0, m = FECgrps[i]->size(); j < m; ++j){
         f = FECgrps[i]->at(j);
         cout << ' ';
         if (f.isInv())cout << '!';
         cout << f.getID();
      }
      cout << endl;
   }
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
   GateList writeList;
   // GateList inList;
   vector<unsigned> inList;
   GateList aigList;
   CirGate::setGlobalRef();
   unsigned id = g -> getID(), m = id;
   g->setToGlobalRef();
   g->dfsTraversal(writeList);
   unsigned n = writeList.size();
   for (unsigned i=0;i<n;++i){
      if (writeList[i]->isPI())
         inList.push_back(writeList[i]->getID());
      else if (writeList[i] -> isAig()){
         aigList.push_back(writeList[i]);
         if (m < writeList[i]->getID())
            m = writeList[i]->getID();
      }
   }
   unsigned k = inList.size();
   unsigned l = aigList.size();
   sort(inList.begin(),inList.end());
   outfile << "aag " << m << " " <<  k << " 0 1 " << l << endl;
   for (unsigned i=0;i<k;++i){
      _gateList[inList[i]]->write(outfile);
   }
   outfile << (id << 1) << endl;
   for (unsigned i=0;i<l;++i){
      aigList[i]->write(outfile);
   }
   for (unsigned i = 0; i < k; ++i){
      _gateList[inList[i]] -> writeSymbol(outfile,i);
   }
   outfile << "o0 " << id << endl;
   outfile << "c\nWrite gate (" << id << ") by ChingChieh Huang" << endl;
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (unsigned i=0;i<I;++i){
      cout << " "<< _piList[i]->getID();
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (unsigned i=0;i<O;++i){
      cout << " "<<_poList[i]->getID();
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   
   if (!_floatList.empty()){
      cout << "Gates with floating fanin(s):";
      for (unsigned i = 0; i < _floatList.size(); ++i){
         cout << " "<<_floatList[i] -> getID();
      }
      cout << endl;
   }
   if (!_unusedList.empty()){
      cout << "Gates defined but not used  :";
      for (unsigned i = 0; i < _unusedList.size(); ++i){
         cout << " "<<_unusedList[i] -> getID();
      }
      cout << endl;
   }
   
}

void
CirMgr::writeAag(ostream& outfile) const
{
   GateList aiglist;
   size_t aig = 0;
   size_t n = _dfsList.size();
   aiglist.reserve(n-I-O);
   for(unsigned i=0; i < n;++i){
      if ( _dfsList[i]->isAig()){
         ++aig;
         aiglist.push_back(_dfsList[i]);
      }
   }
   outfile << "aag" << " " << M << " " << I << " " << L << " " << O << " " << aig << "\n";
   for (unsigned i = 0; i < I; ++i){
      _piList[i] -> write(outfile);
   }
   for (unsigned i = 0; i < O; ++i){
      _poList[i] -> write(outfile);
      
   }
   for(unsigned i=0; i< aig;++i){
      aiglist[i]->write(outfile);
   }
   for (unsigned i = 0; i < I; ++i){
      _piList[i] -> writeSymbol(outfile,i);
   }
   for (unsigned i = 0; i < O; ++i){
      _poList[i] -> writeSymbol(outfile,i);
   }
   outfile << "c\nAag out by ChingChieh Huang" << endl;

}

void
CirMgr::readHeader(ifstream& fin){
   string aag;
   fin >> aag >> M >> I >> L >> O >> A;
   _piList.reserve(I);
   _poList.reserve(O);
   unsigned size = M+O+1;
   _gateList.resize(size,NULL);
   ++lineNo;
   return ;
      
}
void
CirMgr::readInput(ifstream& fin){
   size_t tmp;
   for(int i=0 ; i < I; ++i){
      fin >> tmp;
      CirGate *p = new PIGate(tmp/2,++lineNo);
      _piList.push_back(p);
      _gateList[tmp/2] = p;
   }


}
void
CirMgr::readOutput(ifstream& fin){
   size_t tmp;
   for(int i=1 ; i <= O; ++i){
      fin >> tmp;
      CirGate* p = new POGate(M+i,++lineNo);
      dynamic_cast < POGate* > (p) -> setfanin(tmp);
      _poList.push_back(p);
      _gateList[M+i] = p;
   }
}
void
CirMgr::readAig(ifstream& fin){
   size_t tmp1,tmp2,tmp3;
   for(int i=0 ; i < A; ++i){
      fin >> tmp1 >> tmp2 >> tmp3;
      CirGate* p = new AigGate(tmp1/2,++lineNo);
      dynamic_cast < AigGate* > (p) -> setfanin(tmp2,tmp3);
      _gateList[tmp1/2] = p;
   }
}

void
CirMgr::readSymbol(ifstream& fin){
   string tmp;
   int index;
   
   while (fin >> tmp){
      if (tmp == "c") break;
      char i = tmp[0];
      tmp = tmp.substr(1,tmp.size()-1);
      myStr2Int(tmp, index);
      fin >> tmp;
      if (i == 'i')
         _gateList[++index] -> setSymbol(tmp);
      else if (i == 'o')
         _gateList[M + index + 1] -> setSymbol(tmp); 
   }
}
void
CirMgr::readComment(ifstream& fin){
   return;
}
void
CirMgr::connect(){
   for (int i = 1; i < _gateList.size(); ++i){
      if (_gateList[i])
         _gateList[i] -> connectgate();
   }
   return;
}

void 
CirMgr::genDFSList(GateList& dfsList) const{
   CirGate::setGlobalRef();
   for(unsigned i=0; i<O; ++i){
      _poList[i]->setToGlobalRef();
      _poList[i]->dfsTraversal(dfsList);
   }
}

void
CirMgr::reset(){
   for (unsigned i=1; i<_gateList.size();++i){
     if(_gateList[i]) delete _gateList[i];
   }
   _gateList.clear();
   lineNo = 0;
}
void 
CirMgr::genFnUList(GateList& floatList, GateList& unusedList)const{
   for (unsigned i=0; i< _gateList.size(); ++i){
      if (!_gateList[i]) continue;
      if (_gateList[i] -> isFloat())
         floatList.push_back(_gateList[i]);
      if (_gateList[i] -> isNotUse())
         unusedList.push_back(_gateList[i]);
   }
}
 
void
CirMgr::genAigList(GateList& aigList) const {
	for (unsigned i=0; i<_dfsList.size();++i){
		if (_dfsList[i]->isAig())
			aigList.push_back(_dfsList[i]);
	}
}
void
CirMgr::FECsort() const{
   FecSortCirV fcv;
   for (unsigned i = 0, n = FECgrps.size(); i < n; ++i){
		::sort(FECgrps[i]->begin(),FECgrps[i]->end(),fcv);
   	if (FECgrps[i]->at(0).isInv()){
			for (unsigned j = 0, m = FECgrps[i]->size(); j < m; ++j)
				FECgrps[i]->at(j).Inv();
		}
   }
   ::sort(FECgrps.begin(),FECgrps.end(),FecSortCirVec());
}
