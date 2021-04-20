// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * The length property of Array.prototype is 0
 *
 * @path ch15/15.4/15.4.3/15.4.3.1/S15.4.3.1_A5.js
 * @description Array.prototype.length === 0
 */

//CHECK#1
if (Array.prototype.length !== 0) {
  $ERROR('#1.1: Array.prototype.length === 0. Actual: ' + (Array.prototype.length));
} else {
  if (1 / Array.prototype.length !== Number.POSITIVE_INFINITY) {
    $ERROR('#1.2: Array.prototype.length === +0. Actual: -' + (Array.prototype.length));
  }
} 


