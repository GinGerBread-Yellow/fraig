/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
int ROW = 1<<6;
bool ischange = false;
vector <vector<CirGate::CirGateV> *> FECgrps;
// bool FECisSort = false;
typedef HashMap<CirGate::CirPattern, vector<CirGate::CirGateV>* > FECmap;
typedef vector<CirGate::CirGateV>     fecpair;
enum SimErrorType {
  ERR_LENG,
  ERR_SIG
};
/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned errleng;
static unsigned inputnum;
static char errsig;
static string errstr;
static bool
SimError(SimErrorType err){
  switch (err) {
    case ERR_LENG:
      cerr << "Error: Pattern(" << errstr << ") length(" 
            << errleng << ") does not match the number of inputs(" 
            << inputnum << ") in a circuit!!\n" << endl;
      break;
    case ERR_SIG:
      cerr << "Error: Pattern(" << errstr 
            << ") contains a non-0/1 character(‘"<< errsig << "’).\n" << endl;
      break; 
  }
  return false;
}
static void
CollectValid(FECmap &myhash)
{
  // fecpair * delgrp;
  for (FECmap::iterator li = myhash.begin(); li != myhash.end(); ++li)
  {
    if((*li).second->size() <= 1)
      delete (*li).second;
    else
      FECgrps.push_back((*li).second);
  }
}
/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
  vector<size_t> patterns;
  patterns.resize(I);
  size_t times = 0;
  size_t Sim_times = genMagicTime(), totaltimes = 0;
  //char buff[64][I];
  vector<string> strbuff;
  string s;
  s.resize(I);
  strbuff.resize(ROW,s);
  // cout << "Total #FEC Group = 0" << endl;
  while (times < Sim_times){
    genRanSim(strbuff,patterns);
    Sim(patterns);
    writeSim(strbuff);
    resetFEC();
    if (ischange){
      ischange = false;
      times = 0;
    }
    else if (totaltimes > (2<<(Sim_times/2)))
      ++times;
    ++totaltimes;
  }
  FECsort();
  cout << (totaltimes<<6) << " patterns simulated." << endl;
  //FECisSort = false;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
  vector<size_t> patterns;
  patterns.resize(I);
  //char buff[64][I];
  vector<string> strbuff;
  string t;
  t.resize(I,'0');
  strbuff.resize(ROW,t);
  size_t p_count = 0;
  inputnum = I;
  cout << '\n';
  while(readSimfile(patternFile,patterns,strbuff)){
    Sim(patterns);
    writeSim(strbuff);
    p_count += ROW;
    resetFEC(); 
  }
  ROW = (1 << 6);
  RenewFECs();
  FECsort();
  cout << p_count << " patterns simulated." << endl;
  
  
  //FECisSort = false;
}


/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void
CirMgr::Sim(const vector<size_t>& patterns)
{
  for (unsigned i = 0; i < I; ++i){
    _piList[i]->setP(patterns[i]);
  }
  for (unsigned i = 0, n = _dfsList.size(); i < n; ++i){
    _dfsList[i] -> genP();
  }
  // for (unsigned i = 0; i < O; ++i){
  //   _poList[i] -> simulate();
  // }
}
bool
CirMgr::readSimfile(ifstream& patternfile, vector<size_t>& patterns, vector<string>& buff)
{
  unsigned k = 0;
  if (!patternfile) return false;
  while (patternfile >> errstr){
    errleng = errstr.size();
    if (errleng != I)
      return SimError(ERR_LENG);
    for (unsigned index = 0; index < I; ++index){
      if (errstr[index] != '0' && errstr[index] != '1'){
        errsig = errstr[index];
        return SimError(ERR_SIG);
      }
    }
    for (unsigned input = 0; input < I; ++input){
      patterns[input] <<= 1;
      patterns[input] |= (errstr[input] -'0');
    }
    buff[k] = errstr;
    ++k;
    if (k == 64)break;
  }
  if (!k) return false;
  else if (k < 64){
    ROW = k;
  }
  return true;
}

size_t
CirMgr::genMagicTime(){
  size_t n = (_dfsList.size());
  if (!(n >> 10)) return 13;
  n >>= 13;
  size_t a = 15;
  while (n != 0) {
    n >>= 1;
    a+=2;
  }
  return a;
}

void 
CirMgr::writeSim(vector<string>& buff){
  if (!_simLog) return;
  char out[ROW][O];
  size_t tmp;
  for (unsigned o = 0; o < O; ++o){
    tmp = _poList[o] -> getP();
    // tmp >>= (64-ROW);
    unsigned j = ROW;
    do {
      --j;
      out[j][o] = (tmp & (size_t)1) + '0';
      tmp >>= 1;
    }while ( j != 0);
  }
  // cout << buff << endl;
  for (unsigned i = 0; i < ROW; ++i){
    (*_simLog) << buff[i];
    _simLog->put(' ');
    _simLog->write(out[i],O);
    _simLog->put('\n');
  }
}
void
CirMgr::genRanSim(vector<string>& buff, vector<size_t>& patterns){
  size_t tmp;
  const size_t ref = 1;
  for (unsigned i = 0; i < I; ++i){
    tmp = rnGen(INT_MAX);
    tmp <<= 2;
    tmp |= ~(tmp << 31);
    tmp |= rnGen(5);
    patterns[i] = tmp;
    for (int j = ROW-1; j >= 0;--j){  
      buff[j][i] = ((tmp & ref) + '0');
      tmp >>= 1;
    }
  }
  
}
void
CirMgr::resetFEC(){ 
  if (FECgrps.empty()){
    initFEC();
    return;
  }
  unsigned n = FECgrps.size();
  unsigned change = 0;
  // FECmap myhash(getHashSize(FECgrps.size()*2));
  for (unsigned i = 0; i < n; ++i){
    fecpair *grp;
    size_t m = FECgrps[i]->size();
    FECmap myhash(getHashSize(m));
    for (unsigned j = 0; j < m; ++j){
      CirGate::CirGateV f = FECgrps[i]->at(j);
      CirGate::CirPattern p = f.fgetP();
      if (myhash.query(p, grp)){
        grp -> push_back(f);
      }
      else{
        grp = new fecpair;
        grp -> push_back(f);
        myhash.insert(p,grp);
        ++change;
      }
    }
    delete FECgrps[i];
    CollectValid(myhash);
  }
  // FECgrps.clear();
  // FECgrps.reserve(change);
  // CollectValid(myhash);
  FECgrps.erase(FECgrps.begin(),FECgrps.begin()+n);
  size_t new_n = FECgrps.size();
  if (new_n != n)
    ischange = true;
  cout << "Total #FEC Group = " << new_n << endl;
}


void
CirMgr::initFEC(){
  CirGate::CirGateV gate = _const0;
  fecpair* grp = new fecpair;
  grp -> push_back(gate);
  size_t n = _dfsList.size();
  FECmap myhash(getHashSize(n));
  myhash.insert(0,grp);
  for (unsigned i = 0; i < n; ++i){
    if (_dfsList[i]->isAig()){
      gate = _dfsList[i];
      CirGate::CirPattern p = gate.fgetP();
      if (myhash.query(p, grp)){
        grp -> push_back(gate);
      }
      else if (myhash.query(p.inv(),grp)){
        grp -> push_back(gate.inv());
      }
      else{
        grp = new fecpair;
        grp -> push_back(gate);
        myhash.insert(p.inv(),grp);
      }
    }
  }
  CollectValid(myhash);
  cout << "Total #FEC Group = " << FECgrps.size() << endl;
}

void
CirMgr::destructFEC(){
  for (unsigned i = 0, n = FECgrps.size(); i < n; ++i){
    // for (unsigned j = 0, m = FECgrps[i] -> size(); j < m; ++j){
    //   cout << "erase" << endl;
    //   FECgrps[i] -> at(j).setFECs(0);
    // }
    delete FECgrps[i];
  }
  FECgrps.clear();
  cout << "Total #FEC Group = 0" << endl;
  // FECisSort = false;
}

void
CirMgr::RenewFECs(){
  for (unsigned i = 0, n = FECgrps.size(); i < n; ++i){
    for (unsigned j = 0, m = FECgrps[i]->size(); j < m; ++j){
      FECgrps[i]-> at(j).setFECs(FECgrps[i]);
    }
  }
}