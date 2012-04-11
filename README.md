rawhash
-------

[![Build Status](https://secure.travis-ci.org/pconstr/rawhash.png)](http://travis-ci.org/pconstr/rawhash)

An experimental binary friendly alternative to using a hash as a key:value cache, for [node.js](http://www.nodejs.org).

Keys are binary [Buffer](http://nodejs.org/docs/v0.6.5/api/buffers.html) objects rather than strings. Values are arbitrary objects.

`rawhash` is built on [google-sparsehash](http://code.google.com/p/google-sparsehash) (not included) and [murmurhash3](http://code.google.com/p/smhasher/) (included).

Install
-------

get google-sparsehash:

*  on Debian/Ubuntu:  `apt-get install libsparsehash-dev`
*  on OS X: `brew install google-sparsehash` or `port install google-sparsehash`

or get the latest version from the [google\-sparsehash project](http://code.google.com/p/google-sparsehash/downloads/list)

then install rawhash itself:

`npm install rawhash`

Usage
-----

```javascript
var rh = require('rawhash');
var k = new Buffer(6);
var h = new rh.Sparse();
h.set(k, {a:1, b:2});
console.log(h.get(k));
h.each(function(k, v) {
  console.log(k, v);
});
h.del(k);
```

There are 3 kinds of hashes:

*  `Sparse` is slower, but uses less memory, uses [sparse\_hash\_map<>](http://google-sparsehash.googlecode.com/svn/trunk/doc/sparse_hash_map.html)
*  `Dense` is faster, but uses more memory, uses [dense\_hash\_map<>](http://google-sparsehash.googlecode.com/svn/trunk/doc/dense_hash_map.html)
*  `Map` is usually somewhere between `Sparse` and `Dense`, uses the STL's [map<>](http://www.sgi.com/tech/stl/Map.html) which is actually ordered and supports range queries - not exposed in `Map`

`Sparse` and `Dense` take an optional argument to seed `murmurhash3`.

```javascript
var h = new rh.Dense(42);
```

Performance
-----------

This is largely TBD

On a synthetic test (see `./perf/`) with 150K sets, gets and deletes (very similar to `./test.js`) on a low-end MBP, this is how rawhash compares with using a Javascript hash:

<pre>
Sparse 509 ms
Dense  330 ms
Map    463 ms
{}     754 ms
</pre>

License
-------

(The MIT License)

Copyright (c) 2011 Carlos Guerreiro, [perceptiveconstructs.com](http://perceptiveconstructs.com)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
