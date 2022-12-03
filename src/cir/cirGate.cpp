/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;
unsigned CirGate::_GlobalRef = 0;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   // string t = getTypeStr() + "(" + to_string(_gateID) + ")";
	// if (!_symbol.empty()) t = t + '\"' + _symbol + '\"';
	// t = t + ", line " + to_string(_LineNo);
	// cout << "==================================================\n"
	// 	 << "= " << setw(47) <<  left << t << "=\n"
	// 	 << "==================================================" << endl;
   cout << "===============================================================================\n"
   	 << "= " << getTypeStr() + "(" + to_string(_gateID) + ")";
	if (!_symbol.empty()) cout <<  '\"' + _symbol + '\"';
	cout <<  ", line " << _LineNo 
       << "\n= FECs:";
	printFECs();
    cout << "\n= Value: ";
	printValue();
   	cout << "\n===============================================================================" << endl;
}

void
CirGate::reportFanin(int level)
{
	assert(level >= 0);
	setGlobalRef();
	report(level,0);
}

void
CirGate::reportFanout(int level)
{
	assert (level >= 0);
	setGlobalRef();
	reportfo(level,0);
}

void
CirGate::reportfo( int level, int indent, bool isINV)
{
	assert (level >= 0);
	for (unsigned i = 0; i < indent; ++i)
		cout << "  ";
	if (isINV) cout << "!";
	printSelf(); 
	if (!level || _fanout.empty()) {cout << "\n"; return;}
	if (!this->isGlobalRef()){
		cout << "\n";
		--level;
		++indent;
		this -> setToGlobalRef();
		for (unsigned i = 0; i < _fanout.size(); ++i){
			_fanout[i].gate() -> reportfo(level, indent, _fanout[i].isInv());
		}
	}
	else
		cout << " (*)\n";
}

void 
CirGate::delFanout(const CirGateV &f){
   for (unsigned i = 0, n = _fanout.size(); i < n; ++i){
      if (_fanout[i].isSame(f)){
         _fanout[i] = _fanout.back();
         _fanout.pop_back();
         return;
      }
   }
   cerr << "not found\n";
}

void 
CirGate::delFanin(CirGateV &f, const CirGateV &n_f){
   for (unsigned i = 0, n = _fanout.size(); i < n;++i)
      _fanout[i].gate() -> resetfanin(f, n_f.inv(_fanout[i].isInv()));
}
void 
CirGate::replaceSelf(CirGateV n){
   // cout << "replace "; printSelf(); cout << " with "; n.gate() -> printGate();
   CirGateV self = CirGateV(this);
   cutfanin(self);
   delFanin(self,n);
   n.gate()->resetfanout(n.isInv(), _fanout);
   cirMgr->setgateList(self.getID(),NULL);
   return;
}
void
CirGate::resetfanout(bool inv, vector<CirGateV> &flist){ 
   _fanout.reserve(_fanout.size()+flist.size());
   for (unsigned i = 0, n = flist.size(); i < n;++i)
      _fanout.push_back(flist[i].inv(inv));
}

/**************************************/
/*   class PIGate member functions   */
/**************************************/

void 
PIGate::dfsTraversal(GateList& dfsList){
	dfsList.push_back(this);
}
void 
PIGate::printGate() const
{
	cout << setw(4) << left << getTypeStr() << getID();
	printSymbol();
	cout << endl;
    return;
}
void
PIGate::report(int level,int indent,bool isINV)
{
	for (int i = 0; i < indent; ++i)
		cout << "  ";
	if (isINV) cout << '!';
	printSelf();
	cout << endl;
}
bool
PIGate::isFloat() const
{
	return false;
}
void
PIGate::write(ostream& outfile) const
{
	outfile << (getID()<<1) << "\n";
}
/**************************************/
/*   class POGate member functions   */
/**************************************/
void 
POGate::dfsTraversal(GateList& dfsList){
    if(!_fanin.gate()->isGlobalRef()){
    	_fanin.gate()->setToGlobalRef(); 
    	_fanin.gate()->dfsTraversal(dfsList);
    }
    dfsList.push_back(this);
}
void
POGate::printGate() const 
{
	cout << setw(4) << left << getTypeStr() << getID() << " ";
   if (_fanin.isUndef()) cout << "*";
	if (_fanin.isInv()) cout << "!";
    cout << _fanin.getID();
	printSymbol();
	cout << endl;
	return;	
}
void
POGate::report(int level, int indent,bool isINV)
{
	for (int i = 0; i < indent; ++i)
		cout << "  ";
	if (isINV) cout << '!';
	printSelf();
	if (--level >= 0){
		if (!this->isGlobalRef()) {
			cout << endl;
			_fanin.gate() -> report (level, ++indent, _fanin.isInv());
			this -> setToGlobalRef();
		}
		else
		{
			cout << " (*)" << endl;
		}
		
	}
	else{
		cout << endl;
	}
}
bool
POGate::isFloat() const{
	return _fanin.isUndef();
}
void 
POGate::connectgate(){
	CirGate* f = cirMgr->getGate(_fanin/2);
	if (!f) {
		f = new UndefGate(_fanin/2,0); cirMgr->setgateList(_fanin/2,f);}
	this -> setfanin((size_t)(void*)f + (size_t(_fanin) & NEG));
	_fanin.gate() -> setfanout((size_t)(this) + size_t(_fanin.isInv()));
}
void
POGate::write(ostream& outfile)const{
	outfile << ((_fanin.getID()<<1) + (unsigned)_fanin.isInv() ) << "\n";
}
void 
POGate::resetfanin(CirGateV &f, const CirGateV &n){
   _fanin = n;
}
/**************************************/
/*   class AigGate member functions   */
/**************************************/
void 
AigGate::printGate()const
{
    cout << setw(4) << left << getTypeStr() << getID() << " ";
	if (_fanin1.isUndef()) cout << "*";
	if (_fanin1.isInv()) cout << "!";
    cout << _fanin1.getID() << " ";
	if (_fanin2.isUndef()) cout << "*";
    if (_fanin2.isInv()) cout << "!";
    cout << _fanin2.getID() << endl;
    return;
}

void 
AigGate::dfsTraversal(GateList& dfsList){
	if(!_fanin1.gate()->isGlobalRef()){
		_fanin1.gate()->setToGlobalRef(); 
		_fanin1.gate()->dfsTraversal(dfsList);
	}
	if(!_fanin2.gate()->isGlobalRef()){
		_fanin2.gate()->setToGlobalRef(); 
		_fanin2.gate()->dfsTraversal(dfsList);
	}
	dfsList.push_back(this);
}
void
AigGate::report(int level,int indent,bool isINV)
{
	for (int i = 0; i < indent; ++i)
		cout << "  ";
	if (isINV) cout << '!';
	printSelf();
	if (--level>=0){		
		if (!this->isGlobalRef()){
			cout << endl;
			++indent;
			_fanin1.gate() -> report (level, indent, _fanin1.isInv());
			_fanin2.gate() -> report (level, indent, _fanin2.isInv());
			this -> setToGlobalRef();
		}
		else
		{
			cout << " (*)" << endl;
		}
		
	}
	else{
		cout << endl;
	}
}
bool
AigGate::isFloat() const{
	return _fanin1.isUndef() || _fanin2.isUndef();
}
void
AigGate::connectgate(){
	CirGate* f1 = cirMgr->getGate(_fanin1/2);
	if (!f1) {
		f1 = new UndefGate(_fanin1/2,0); cirMgr->setgateList(_fanin1/2,f1);}
	CirGate* f2 = cirMgr->getGate(_fanin2/2);
	if (!f2) {
		f2 = new UndefGate(_fanin2/2,0); cirMgr->setgateList(_fanin2/2,f2);}
	setfanin((size_t)(f1) + (_fanin1 & NEG),
			 (size_t)(f2) + (_fanin2 & NEG) );
	_fanin1.gate() -> setfanout((size_t)this +(size_t)(_fanin1.isInv()));
	_fanin2.gate() -> setfanout((size_t)this +(size_t)(_fanin2.isInv()));
}
void
AigGate::write(ostream& outfile) const{
	outfile << (getID()<<1) << " " 
			<< ((_fanin1.getID()<<1) + (unsigned)_fanin1.isInv()) << " "
			<< ((_fanin2.getID()<<1) + (unsigned)_fanin2.isInv()) << endl;
}


bool
AigGate::sweep(){
   // cout << "Sweeping: " << getTypeStr() << '(' << getID() << ')' << " removed\n";
   CirGateV f = CirGateV(this);
   cutfanin(f);
   if (!isNotUse()){
      delFanin(f,size_t(0));
   }
   return true;
}
void 
AigGate::resetfanin(CirGateV &f, const CirGateV &n){
   if (_fanin1.isSame(f))
      _fanin1 = n;
   else 
      _fanin2 = n;
   return;
}
void
AigGate::cutfanin(const CirGateV &f){
   if (_fanin1>>1)
      _fanin1.gate()->delFanout(f);
   if (_fanin2>>1)   
      _fanin2.gate()->delFanout(f);
}
bool 
AigGate::optimize(CirGateV &f){
   if (_fanin1.isSame(_fanin2)){
      if (_fanin1 == _fanin2){
         f = _fanin1;
      }
      else{
         f = cirMgr->getGcc();
      }
      return true;
   }
   else if (_fanin1.isConst0() || _fanin2.isConst0()){
      f = cirMgr->getGcc();
      return true;
   }
   else if (_fanin1.isConst1()){
      f = _fanin2;
      return true;
   }
   else if (_fanin2.isConst1()){
      f = _fanin1;
      return true;
   }
   else{
      return false;
   }
}
/**************************************/
/*   class ConstGate member functions   */
/**************************************/
void
ConstGate::dfsTraversal(GateList& dfsList){
   dfsList.push_back(this);
}
void 
ConstGate::printGate() const
{
    cout << setw(4) << left << getTypeStr() << '0' << endl;
    return;
}
void
ConstGate::report(int level,int indent,bool isINV)
{
	for (int i = 0; i < indent; ++i)
		cout << "  ";
	if (isINV) cout << '!';
	printSelf();
	cout << endl;
}
bool
ConstGate::isFloat() const{
	return false;
}
void 
ConstGate::write(ostream& outfile) const{
	return;
}
/**************************************/
/*   class UndefGate member functions   */
/**************************************/
void 
UndefGate::dfsTraversal(GateList& dfsList){
	return;
}
void 
UndefGate::printGate() const
{
	cout << setw(4) << left << getTypeStr() << getID() << endl;
    return;
}
void
UndefGate::report(int level,int indent,bool isINV)
{
	for (int i = 0; i < indent; ++i)
		cout << "  ";
	if (isINV) cout << '!';
	printSelf();
	cout << endl;
}
bool
UndefGate::isFloat() const{
	return false;
}
void 
UndefGate::write(ostream& outfile) const{
	return;
}
bool
UndefGate::sweep(){
   if (!isNotUse()){
      CirGateV self = CirGateV(this);
      delFanin(self,size_t(0));
   }     
   return true;
}


