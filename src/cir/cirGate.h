/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
#define NEG 0x1

public:
	CirGate(): _pattern(0){}
	CirGate( unsigned gateID, unsigned lineno): 
			_ref(0), _gateID(gateID), _LineNo(lineno), _pattern(0) {}
	virtual ~CirGate() {}
	
	class CirPattern
	{
	public:
		CirPattern() {}
		CirPattern(size_t p): _value(p) {}
		bool operator == (const CirPattern &c) { return (_value == c._value);}
		// bool operator != (const size_t &c) { return (_value != c);}
		CirPattern operator ^ (const CirPattern &c) { return (_value ^ c._value);}
		CirPattern operator & (const CirPattern &c) { return (_value & c._value);}
		CirPattern& inv() { _value = ~(_value);return *this;}
		CirPattern& operator <<= (int i) { _value <<= i; return (*this);}
		operator size_t () const { return _value; }
		size_t operator() () const { 
			// return (_value ^ (_value << 10)) >> 13;
			return _value | (_value << 23);
		}
	private:
		size_t _value;
	};

	class CirGateV 
	{

	public:
		CirGateV( size_t gid = 0): _gateV(gid) { }
		CirGateV( CirGate* g, size_t phase = 0): _gateV(size_t(g) + phase) { } 
		CirGateV( const CirGateV& g): _gateV(g._gateV) { }

		CirGate* gate() const {
			return (CirGate*)(_gateV & ~size_t(NEG)); 
		}
		operator size_t () const { return _gateV; }
		/*
		CirGateV operator + (size_t sign) { return _gateV + sign;}
		CirGateV operator - (size_t sign) { return _gateV - sign;}
		*/
		CirGateV inv(const bool s = true) const {return  _gateV ^ size_t(s);}
		CirGateV& Inv() { _gateV ^= size_t(NEG); return (*this);}
		CirGateV& operator = (const CirGateV &g) { _gateV = g._gateV; return *this;}
		bool operator == (const CirGateV &c) { return (this->_gateV == c._gateV);}
		bool operator < (const CirGateV &c) const{ return (getID() < c.getID()); }
		bool isSame(const CirGateV& c) const { return (this->_gateV & ~size_t(NEG)) == (c._gateV & ~size_t(NEG));}
		bool isInv() const { return (_gateV & NEG); }
		bool isUndef() const { return gate()->undef();}
		bool isAig() const { return gate()->isAig();}
		bool isConst0() const { return isConst() && !isInv();}
		bool isConst1() const { return isConst() && isInv();}
		unsigned getID() const{ return gate()->getID();}
		CirGate::CirPattern fgetP() const{ 
			return (isInv())? gate()->getP().inv() : gate()->getP();
		}
		// bool P_changed() const{
		// 	return gate()->simulate();
		// }
		void setFECs(vector<CirGateV>* const &p){
			gate()->setFECs(p);
		}
	private:
		size_t _gateV; 
		bool isConst() const { return gate()->isConst();}
	};

	// Basic access methods
	virtual string getTypeStr() const = 0;
	virtual bool isAig() const { return false; }
	virtual bool isPI() const { return false; }
	virtual bool isFloat() const = 0;
	virtual bool isNotUse() const {return _fanout.empty();}
	virtual bool undef() const { return false;}
	virtual bool isConst() const {return false;}
	virtual void report(int level,int indent,bool isINV = false) = 0;
	virtual void connectgate() {return;}
	virtual void write(ostream& outfile) const = 0;


	// Printing functions
	virtual void printGate() const {}
	void reportGate() const;
	void reportFanin(int level);
	void reportFanout(int level);

	// traverse
	bool isGlobalRef() { return (_ref == _GlobalRef); }
	void setToGlobalRef(){ _ref = _GlobalRef; }
	static void setGlobalRef() { _GlobalRef++; }

	virtual void dfsTraversal(GateList&) = 0;

	unsigned getID() const{ return _gateID; }
	unsigned getLineNo() const { return _LineNo; }
	void setfanout(size_t p){ _fanout.push_back(p); }
	void resetfanout(bool inv, vector<CirGateV>& flist);
	void setSymbol(const string& s){ _symbol = s;} 
	virtual void writeSymbol(ostream& outfile,const unsigned& ind) const{ return;}
	// sweep function
	virtual bool sweep() { return false; }
	// delete fanout f from this->_fanout
	void delFanout(const CirGateV &f);
	// for (_fanout[i]) change fanin from f to n
	void delFanin(CirGateV &f, const CirGateV &n);
	// mainly for Aig
	virtual void resetfanin(CirGateV &f, const CirGateV &n){ return; }
	virtual void cutfanin(const CirGateV &f){return;}
	// optimize function, return f is the gate that can replace itself
	virtual bool optimize(CirGateV &f) { return false; }
	void replaceSelf(CirGateV t);
	virtual bool getfanin(CirGateV &i0, CirGateV &i1) const{ return false;}

	// void setP(const size_t& p){ _pattern = p; return;}
	void setP(const CirPattern& p){_pattern = p;}
	virtual void genP(){return;}
	CirPattern getP() const{ return _pattern;}
	virtual void setFECs(vector<CirGateV> * const &ptr){return;}
	virtual void printFECs() const{return;}
	// virtual bool simulate() = 0;


private:
	static unsigned   	_GlobalRef;
	unsigned          	_ref;
	unsigned          	_gateID;
	unsigned         	_LineNo;
	CirPattern			_pattern;
	vector<CirGateV>  	_fanout;
	string				_symbol;

protected:
	void printSelf() const{
		cout << getTypeStr() << " " << _gateID ;
		return;
	}
	void reportfo(int level, int indent, bool isINV = false);

	bool isSymbol() const{
		return (!_symbol.empty());
	}
	void printSymbol() const { 
		if (!_symbol.empty()) 
			cout << " (" << _symbol << ")"; 
	}
	string getSymbol() const{
		return _symbol;
	}
	void printValue() const{
		string tmp;
		CirPattern p = 1;
		unsigned i = 0;
		while (p != 0){
			if ((i >> 3)){ i = 0; tmp += '_';}
			tmp += (_pattern & p)? '1':'0';
			p <<= 1;
			++i;
		}
		cout << tmp;
	}
};

class PIGate : public CirGate
{
public:  
	PIGate(unsigned gateID,unsigned lineno): CirGate(gateID,lineno){}
	void printGate() const;
	void dfsTraversal(GateList&);
	void report(int level,int indent,bool isINV = false) ;
	bool isFloat() const;
	bool isPI() const { return true; }
	string getTypeStr() const { return "PI";}
	void write(ostream& outfile) const;
	void writeSymbol(ostream& outfile, const unsigned& ind) const{ 
		if (isSymbol())
			outfile << "i" << (ind) << " " << getSymbol() << "\n";}
	// bool simulate(){
	// 	return true;
	// }
private:
};

class POGate : public CirGate
{
  	friend class CirMgr;
public:  

	POGate( unsigned gateID, unsigned lineno):
			CirGate(gateID,lineno){}
	
	void printGate() const;
	void dfsTraversal(GateList&);
	void report(int level,int indent,bool isINV = false);
	bool isFloat() const;
	bool isNotUse() const{ return false;}
	void connectgate();
	string getTypeStr() const { return "PO";}
	void write(ostream& outfile) const;
	
	// CirGateV getfanin() const{ return _fanin;}
	void setfanin(size_t f){ _fanin = f;}
	void writeSymbol(ostream& outfile,const unsigned& ind) const{ 
		if (isSymbol())
			outfile << "o" << (ind) << " " << getSymbol() << "\n";
	}

	void resetfanin(CirGateV &f, const CirGateV &n);
	void genP(){ setP(_fanin.fgetP());}
	// bool simulate(){ 
	// 	if (_fanin.P_changed()){return genP();}
	// 	return false;
	// }
	
private:
	CirGateV  _fanin;	
};

class AigGate : public CirGate
{
public: 
	AigGate( unsigned gateID, unsigned lineno): CirGate( gateID,lineno) {}
	void printGate()const;
	void dfsTraversal(GateList&);
	void report(int level,int indent,bool isINV = false) ;
	void connectgate();
	
	string getTypeStr() const { return "AIG";}
	bool isAig() const{ return true;}
	bool isFloat() const;
	void write(ostream& outfile) const;

	bool sweep();
	bool getfanin(CirGateV &i0, CirGateV &i1)const {i0 = _fanin1; i1 = _fanin2; return true;}
	void setfanin(size_t f1, size_t f2){ _fanin1 = f1; _fanin2 = f2;}
	void resetfanin(CirGateV &f, const CirGateV &n);
	void cutfanin(const CirGateV &f);
	bool optimize(CirGateV &f);
	void genP(){ return setP(_fanin1.fgetP() & _fanin2.fgetP());}
	// bool simulate(){
	// 	if (!_fanin1.P_changed() && !_fanin2.P_changed())
	// 		return false;
	// 	return genP();
	// }
	void setFECs(vector<CirGateV> * const &ptr){FECs = ptr;}
	void printFECs() const{
		if (!FECs) return;
		unsigned n = FECs ->size();
		unsigned self = 0;
		bool inv;
		for (; self < n; ++self){
			if (FECs->at(self).gate() == this) 
			{
				inv = FECs->at(self).isInv();
				break;}
		}
		for (unsigned i = 0; i < n; ++i){
			if (i == self) continue;
			cout << ' ';
			if (inv ^ FECs->at(i))
				cout << '!';
			cout << FECs->at(i).getID();
		}
		cout << endl;
	}
private:
	CirGateV  _fanin1;
	CirGateV  _fanin2;
	vector<CirGateV> * FECs;
	
};

class ConstGate : public CirGate
{
public:  
	ConstGate(unsigned gateID,unsigned lineno): 
			CirGate(gateID,lineno){}
	void printGate() const;
	void dfsTraversal(GateList&);
	void report(int level,int indent,bool isINV = false) ;
	bool isFloat() const;
	bool isNotUse() const{ return false;}
	bool isConst() const {return true;}
	string getTypeStr() const { return "CONST";}
	void write(ostream& outfile) const;
	// bool simulate(){return false;}
private:

};
class UndefGate : public CirGate
{
public:  
	UndefGate(unsigned gateID,unsigned lineno): CirGate(gateID,lineno){}
	void printGate() const;
	void dfsTraversal(GateList&);
	void report(int level,int indent,bool isINV = false) ;
	bool isFloat() const;
	bool isNotUse() const{ return false;}
	bool undef() const{ return true;}
	string getTypeStr() const { return "UNDEF";}
	void write(ostream& outfile) const;
	bool sweep();
	// bool simulate(){return false;}


private:
};

struct FecSortCirV
{
   bool operator() (const CirGate::CirGateV& m1, const CirGate::CirGateV& m2) const {
      return m1 < m2;
   }
};
struct FecSortCirVec
{
   bool operator() (vector<CirGate::CirGateV> * const &m1, vector<CirGate::CirGateV> * const&m2) const {
      return m1->at(0) < m2->at(0);
   }
};


#endif // CIR_GATE_H
