/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;

   public:
      iterator(vector<Data>* b, vector<Data>* p, size_t e):
            _Bucket(b), _end(p), _Element(e) { }
      iterator(const iterator& it):
            _Bucket(it._Bucket), _end(it._end), _Element(it._Element) { }
      
      const Data& operator * () const { return _Bucket -> at(_Element); }
      iterator& operator ++ () { 
         if (_Bucket -> size() > ++_Element)
            return (*this); 
         while (_Bucket != _end) {
            ++_Bucket;
            if (!_Bucket -> empty()) break;
         }  
         _Element = 0;
         return (*this);  
      }
      iterator& operator -- () { 
         if (--_Element >= 0)
            return (*this); 
         do {
            --_Bucket;
         } while (_Bucket -> empty());
         _Element = _Bucket -> size()-1;
         return (*this);  
      }
      iterator operator ++ (int) { 
         iterator li = *this;
         ++(*this);
         return li; 
      }
      iterator operator -- (int) { 
         iterator li = (*this);
         --(*this);
         return li; 
      }
      iterator& operator = (const iterator& i) { 
         _Bucket = i._Bucket;
         _end = i._end;
         _Element = i._Element;
         return *(this); 
      }
      bool operator == (const iterator& i) const { 
         return ((i._Bucket == _Bucket) && 
                  (i._Element == _Element));
      }
      bool operator != (const iterator& i) const { 
         return (!((*this) == i)); }
   private:
      vector<Data>*  _Bucket;
      vector<Data>*  _end;
      size_t         _Element;
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const { 
      size_t n = 0;
      while (n != _numBuckets){
         if (!_buckets[n].empty()) break;
         ++n;
      }
      return iterator(_buckets+n, _buckets+_numBuckets,0);}
   // Pass the end
   iterator end() const { 
         return iterator(_buckets+_numBuckets, _buckets+_numBuckets, 0); }
   // return true if no valid data
   bool empty() const { 
      return (begin() == end());}
   // number of valid data
   size_t size() const { 
      size_t s = 0;
      for (size_t i = 0; i < _numBuckets; ++i)
         s += _buckets[i].size();
      return s;
   }

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const { 
      // for ( iterator li = begin(); li != end(); ++li){
      //    if ((*li) == d) return true;
      // }
      // return false; 
      size_t n = bucketNum(d);
      for (unsigned i = 0, m = _buckets[n].size(); i < m; ++i) {
         if ( _buckets[n][i] == d ) return true;}
      return false;
   }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const { 
      // for ( iterator li = begin(); li != end(); ++li)
      //    if ((*li) == d) {
      //       d = (*li);
      //       return true;
      //    }
      size_t n = bucketNum(d);
      for (unsigned i = 0, m = _buckets[n].size(); i < m; ++i) {
         if ( _buckets[n][i] == d ) {
            d = _buckets[n][i];
            return true;}
      }
      return false;
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d) {  
      // for ( iterator li = begin(); li != end(); ++li)
      //    if ((*li) == d) {
      //       li._Bucket->at(li._Element) = d;
      //       return true;
      //    }
      size_t n = bucketNum(d);
      for (unsigned i = 0, m = _buckets[n].size(); i < m; ++i) {
         if ( _buckets[n][i] == d ) {
            _buckets[n][i] = d;
            return true;
         }
      }
      return false;
   }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d) { 
      size_t n = bucketNum(d);
      for (unsigned i = 0, m = _buckets[n].size(); i < m; ++i) {
         if ( _buckets[n][i] == d ) return false;}
      _buckets[n].push_back(d);
      return true; 
   }

   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const Data& d) { 
      // for (iterator li = begin(); li != end(); ++li){
      //    if (*li == d){
      //       swap(li._Bucket -> at(li._Element),li._Bucket -> back());
      //       li._Bucket -> pop_back();
      //       return true;
      //    }
      // }
      size_t n = bucketNum(d);
      for (unsigned i = 0, m = _buckets[n].size(); i < m; ++i) {
         if ( _buckets[n][i] == d ) {
            _buckets[n][i] = _buckets[n].back();
            _buckets[n].pop_back();
            return true;
         }
      }
      return false;
   }

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;

   size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
