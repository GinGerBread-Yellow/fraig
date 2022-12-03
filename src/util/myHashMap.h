/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>

using namespace std;

// TODO: (Optionally) Implement your own HashMap and Cache classes.

//-----------------------
// Define HashMap classes
//-----------------------
// To use HashMap ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
class HashKey
{
public:
   HashKey() {}
   HashKey(const size_t &i0, const size_t &i1): _in0(i0), _in1(i1) {}

   size_t operator() () const { 
      size_t k = (_in0 < _in1)? _in1: _in0;
      k ^= (_in1 + _in0)>>2;
      return k;
   }

   bool operator == (const HashKey& k) const { 
      if (_in0 == k._in0)
         return _in1 == k._in1;
      else if (_in0 == k._in1)
         return _in1 == k._in0;
      return false; }

private:
   size_t _in0;
   size_t _in1;
};
//
template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap(size_t b=0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashMap() { reset(); }

   // [Optional] TODO: implement the HashMap<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the HashMap
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashMap<HashKey, HashData>;
   public:
      iterator(vector<HashNode>* b, vector<HashNode>* p, size_t e):
            _Bucket(b), _end(p), _Element(e) { }
      iterator(const iterator& it):
            _Bucket(it._Bucket), _end(it._end), _Element(it._Element) { }
      
      const HashNode& operator * () const { return _Bucket -> at(_Element); }
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
      vector<HashNode>  *_Bucket;
      vector<HashNode>  *_end;
      size_t            _Element;
   };

   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const { 
      size_t n = 0;
      while (n != _numBuckets){
         if (!_buckets[n].empty()) break;
         ++n;
      }
      return iterator(_buckets+n, _buckets+_numBuckets,0); }
   // Pass the end
   iterator end() const { 
      return iterator(_buckets+_numBuckets, _buckets+_numBuckets, 0); }
   // return true if no valid data
   bool empty() const { return begin() == end(); }
   // number of valid data
   size_t size() const { 
      size_t s = 0;
      for (size_t i = 0; i < _numBuckets; ++i)
         s += _buckets[i].size();
      return s;
   }

   // check if k is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const HashKey& k) const { 
      size_t n = bucketNum(k);
      for (unsigned i = 0, m = _buckets[n].size(); i < m; ++i) {
         if ( _buckets[n][i].first == k ) return true;}
      return false;
   }

   // query if k is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(const HashKey& k, HashData& d) const { 
      size_t n = bucketNum(k);
      for (unsigned i = 0, m = _buckets[n].size(); i < m; ++i) {
         if ( _buckets[n][i].first == k ) {
            d = _buckets[n][i].second;
            return true;}
      }
      return false;
   }

   // update the entry in hash that is equal to k (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const HashKey& k, HashData& d) { 
      size_t n = bucketNum(k);
      for (unsigned i = 0, m = _buckets[n].size(); i < m; ++i) {
         if ( _buckets[n][i].first == k ) {
            _buckets[n][i].second = d;
            return true;
         }
      }
      return false;
   }

   // return true if inserted d successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d) { 
      size_t n = bucketNum(k);
      for (unsigned i = 0, m = _buckets[n].size(); i < m; ++i) {
         if ( _buckets[n][i].first == k ) return false;}
      _buckets[n].push_back( HashNode(k,d) );
      return true; 
   }

   // return true if removed successfully (i.e. k is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const HashKey& k) { 
      size_t n = bucketNum(k);
      for (unsigned i = 0, m = _buckets[n].size(); i < m; ++i) {
         if ( _buckets[n][i].first == k ) {
            _buckets[n][i] = _buckets[n].back();
            _buckets[n].pop_back();
            return true;
         }
      }
      return false;}

private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//    
//    size_t operator() () const { return 0; }
//   
//    bool operator == (const CacheKey&) const { return true; }
//       
// private:
// }; 
// 
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { reset(); _size = s; _cache = new CacheNode[s]; }
   void reset() {  _size = 0; if (_cache) { delete [] _cache; _cache = 0; } }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const {
      size_t i = k() % _size;
      if (k == _cache[i].first) {
         d = _cache[i].second;
         return true;
      }
      return false;
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) {
      size_t i = k() % _size;
      _cache[i].first = k;
      _cache[i].second = d;
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
