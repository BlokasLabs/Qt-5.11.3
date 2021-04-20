// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * String.prototype.slice (start, end) returns a string value(not object)
 *
 * @path ch15/15.5/15.5.4/15.5.4.13/S15.5.4.13_A2_T8.js
 * @description start is tested_string.length+1, end is 0
 */

var __string = new String("this is a string object");

//////////////////////////////////////////////////////////////////////////////
//CHECK#1
if (__string.slice(__string.length+1, 0) !== "") {
  $ERROR('#1: __string = new String("this is a string object"); __string.slice(__string.length+1, 0) === "". Actual: '+__string.slice(__string.length+1, 0) );
}
//
//////////////////////////////////////////////////////////////////////////////

