#!/usr/bin/env node

/* Copyright 2011, Carlos Guerreiro
 * Licensed under the MIT license */

var assert = require('assert');
var rh = require("./rawhash");

function doTests(h) {
    var timeStart = Date.now();

    var numItems = 150000;
    var i;
    var b = new Buffer(18);
    for(i = 0; i < 18; ++i)
	b[i] = i;
    
    var i0, i1, i2;
    for(i = 0; i < numItems; ++i) {
	i0 = i & 255;
	i1 = (i >> 8) & 255;
	i2 = (i >> 16) & 255;
	b[0] = i0;
	b[1] = i1;
	b[2] = i2;
	h.set(b, {a: i0, b: i1, c: i2});
    }
    h.set(b, {a: i0, b: i1, c: i2}); // reset value
    
    var v;
    for(i = 0; i < numItems; ++i) {
	i0 = i & 255;
	i1 = (i >>  8) & 255;
	i2 = (i >> 16) & 255;
	b[0] = i0;
	b[1] = i1;
	b[2] = i2;
	v = h.get(b);;
	assert.equal(v.a, i0);
	assert.equal(v.b, i1);
	assert.equal(v.c, i2);
    }
        
    for(i = 0; i < numItems; ++i) {
	i0 = i & 255;
	i1 = (i >>  8) & 255;
	i2 = (i >> 16) & 255;
	b[0] = i0;
	b[1] = i1;
	b[2] = i2;
	assert(h.del(b));
    }

    var timeEnd = Date.now();
    console.log(h.constructor.name+ ' '+ (timeEnd - timeStart)+ ' ms');
}

doTests(new rh.Sparse(42));
doTests(new rh.Dense(42));
doTests(new rh.Map());
