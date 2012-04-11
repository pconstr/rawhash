/* Copyright 2011, Carlos Guerreiro
 * http://perceptiveconstructs.com
 * Licensed under the MIT license */

#include <cstring>
#include <v8.h>
#include <node.h>
#include <node_buffer.h>

#include <google/sparse_hash_map>
#include <google/dense_hash_map>

#include "MurmurHash3.h"

#include <map>

using google::sparse_hash_map;
using google::dense_hash_map;

using namespace std;

using namespace v8;
using namespace node;

struct RawHashKey {
  RawHashKey() : len(0), data(0) {
  }
  RawHashKey(size_t l, void* d) : len(l), data(d) {}
  size_t len;
  void* data;
};

struct BufferHasher {
  uint32_t seed;
  BufferHasher(void) : seed(1) {
  }
  BufferHasher(uint32_t s) : seed(s) {
  }
  size_t operator ()(const RawHashKey& k) const {
    uint32_t out;
    if(k.len == 0 || k.len == 0xffffffff)
      return 0;
    MurmurHash3_x86_32(k.data, k.len, seed, &out);
    return out;
  }
};

struct BufferEq {
  BufferEq(void) {
  }
  bool operator()(const RawHashKey& a, const RawHashKey& b) const {
    if(a.len != b.len)
      return false;
    if(a.len == 0 || a.len == 0xffffffff)
      return true;
    return memcmp(a.data, b.data, a.len) == 0;
  }
};

// used by MapRawHash (std::map) only
// doesn't have to deal with special values for deleted or empty
struct BufferLess {
  BufferLess(void) {
  }
  bool operator()(const RawHashKey& a, const RawHashKey& b) const {
    if(a.len == b.len)
      return memcmp(a.data, b.data, a.len) < 0;
    else
      return a.len < b.len;
  }
};

RawHashKey empty(0, 0);
RawHashKey deleted(0xffffffff, 0);

void initPostConstruction(sparse_hash_map<RawHashKey, Persistent<Value>, BufferHasher, BufferEq>& m) {
  m.set_deleted_key(deleted);
}

void initPostConstruction(dense_hash_map<RawHashKey, Persistent<Value>, BufferHasher, BufferEq>& m) {
  m.set_empty_key(empty);
  m.set_deleted_key(deleted);
}

void initPostConstruction(map<RawHashKey, Persistent<Value>, BufferLess>& m) {
}

template <class StorageT>
class RawHash: ObjectWrap {
private:
  StorageT items;

  typename StorageT::iterator access(Local<Value> v) {
    Local<Object> o = v->ToObject();
    RawHashKey pk(Buffer::Length(o), Buffer::Data(o));
    return items.find(pk);
  }
public:
  RawHash() : items() {
    initPostConstruction(items);
  }
  RawHash(uint32_t seed) : items(0, BufferHasher(seed)) {
    initPostConstruction(items);
  }

  ~RawHash() {
    typename StorageT::iterator it;

    for(it = items.begin(); it != items.end(); ++it) {
      free(it->first.data);
      it->second.Dispose();
    }
  }

  static void init(Handle < Object > target, Handle<Value> (*func)(const Arguments&), Persistent<FunctionTemplate>& ct, const char* name) {
    Local<FunctionTemplate> t = FunctionTemplate::New(func);
    ct = Persistent<FunctionTemplate>::New(t);
    ct->InstanceTemplate()->SetInternalFieldCount(1);
    Local<String> nameSymbol = String::NewSymbol(name);
    ct->SetClassName(nameSymbol);
    NODE_SET_PROTOTYPE_METHOD(ct, "set", set);
    NODE_SET_PROTOTYPE_METHOD(ct, "get", get);
    NODE_SET_PROTOTYPE_METHOD(ct, "del", del);
    NODE_SET_PROTOTYPE_METHOD(ct, "each", each);
    target->Set(nameSymbol, ct->GetFunction());
  }

  static Handle<Value> New(const Arguments& args) {
    HandleScope scope;

    if (!args.IsConstructCall()) {
      return ThrowException(Exception::TypeError(String::New("Use the new operator to create instances of this object.")));
    }

    RawHash* bh = new RawHash();
    bh->Wrap(args.This());
    return args.This();
  }

  static Handle<Value> NewWithSeed(const Arguments& args) {
    HandleScope scope;

    if (!args.IsConstructCall()) {
      return ThrowException(Exception::TypeError(String::New("Use the new operator to create instances of this object.")));
    }

    uint32_t seed = 1;
    if(args.Length() >= 1) {
      Local<Integer> s = *args[0]->ToInteger();
      if(s.IsEmpty())
        return ThrowException(Exception::Error(String::New("argument 1 must be a integer")));
      seed = s->Value();
    }
    RawHash* bh = new RawHash(seed);
    bh->Wrap(args.This());
    return args.This();
  }

  static Handle<Value> get(const Arguments& args) {
    HandleScope scope;

    if(args.Length() != 1) {
      return ThrowException(Exception::Error(String::New("get takes 1 argument")));
    }

    if(!Buffer::HasInstance(args[0]))
      return ThrowException(Exception::Error(String::New("argument 1 must be a Buffer")));

    RawHash* bh = ObjectWrap::Unwrap<RawHash>(args.This());
    typename StorageT::iterator it = bh->access(args[0]);
    if(it == bh->items.end())
      return Undefined();

    return scope.Close(it->second);
  }

  static Handle<Value> set(const Arguments& args) {
    HandleScope scope;

    if(args.Length() != 2) {
      return ThrowException(Exception::Error(String::New("set takes 2 arguments")));
    }

    if(!Buffer::HasInstance(args[0]))
      return ThrowException(Exception::Error(String::New("argument 1 must be a Buffer")));

    RawHash* bh = ObjectWrap::Unwrap<RawHash>(args.This());

    Persistent<Value> pv = Persistent<Value>::New(args[1]);
    Local<Object> o = args[0]->ToObject();

    RawHashKey pk;
    pk.len = Buffer::Length(o);
    pk.data = malloc(pk.len);
    memcpy(pk.data, Buffer::Data(o), pk.len);

    typename StorageT::iterator it = bh->items.find(pk);
    if(it != bh->items.end()) {
      it->second.Dispose();
      it->second = pv;
    } else
      bh->items[pk] = pv;

    return scope.Close(args[1]);
  }

  static Handle<Value> del(const Arguments& args) {
    HandleScope scope;

    if(args.Length() != 1) {
      return ThrowException(Exception::Error(String::New("get takes 1 argument")));
    }

    if(!Buffer::HasInstance(args[0]))
      return ThrowException(Exception::Error(String::New("argument 1 must be a Buffer")));

    RawHash* bh = ObjectWrap::Unwrap<RawHash>(args.This());
    typename StorageT::iterator it = bh->access(args[0]);
    if(it == bh->items.end())
      return scope.Close(Boolean::New(false));

    free(it->first.data);
    it->second.Dispose();
    bh->items.erase(it);

    return scope.Close(Boolean::New(true));
  }

  static Handle<Value> each(const Arguments& args) {
    if(args.Length() != 1) {
      return ThrowException(Exception::Error(String::New("each takes 1 argument")));
    }
    if (!args[0]->IsFunction()) {
      return ThrowException(Exception::TypeError(String::New("argument must be a callback function")));
    }
    // There's no ToFunction(), use a Cast instead.
    Local<Function> callback = Local<Function>::Cast(args[0]);
    Local<Value> k = Local<Value>::New(Undefined());
    Local<Value> v = Local<Value>::New(Undefined());

    const unsigned argc = 2;
    Local<Value> argv[argc] = { v };

    RawHash* bh = ObjectWrap::Unwrap<RawHash>(args.This());
    typename StorageT::iterator it;

    for(it = bh->items.begin(); it != bh->items.end(); ++it) {
      const RawHashKey& k = it->first;
      Buffer* b = Buffer::New(k.len);
      memcpy(Buffer::Data(b), k.data, k.len);
      // FIXME: consider not creating a Buffer object every time
      argv[0] = Local<Value>::New(b->handle_);
      argv[1] = *it->second;
      TryCatch tc;
      Local<Value> ret = callback->Call(Context::GetCurrent()->Global(), argc, argv);
      if(ret.IsEmpty() || ret->IsFalse())
        break;
    }

    return Undefined();
  }
};

typedef RawHash<sparse_hash_map<RawHashKey, Persistent<Value>, BufferHasher, BufferEq> > SparseRawHash;
typedef RawHash<dense_hash_map<RawHashKey, Persistent<Value>, BufferHasher, BufferEq> > DenseRawHash;
typedef RawHash<map<RawHashKey, Persistent<Value>, BufferLess> > MapRawHash;

static Persistent<FunctionTemplate> sparseRawHash_ct;
static Persistent<FunctionTemplate> denseRawHash_ct;
static Persistent<FunctionTemplate> mapRawHash_ct;

void RegisterModule(Handle<Object> target) {
  SparseRawHash::init(target, SparseRawHash::NewWithSeed, sparseRawHash_ct, "Sparse");
  DenseRawHash::init(target, DenseRawHash::NewWithSeed, denseRawHash_ct, "Dense");
  MapRawHash::init(target, MapRawHash::New, mapRawHash_ct, "Map");
}

NODE_MODULE(rawhash, RegisterModule);
