// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * If thisArg is null or undefined, the called function is passed the global object as the this value
 *
 * @path ch15/15.3/15.3.4/15.3.4.3/S15.3.4.3_A3_T4.js
 * @description Argument at apply function is undefined
 */

Function("this.field=\"oil\"").apply(undefined);

//CHECK#1
if (this["field"] !== "oil") {
  $ERROR('#1: If thisArg is null or undefined, the called function is passed the global object as the this value');
}

