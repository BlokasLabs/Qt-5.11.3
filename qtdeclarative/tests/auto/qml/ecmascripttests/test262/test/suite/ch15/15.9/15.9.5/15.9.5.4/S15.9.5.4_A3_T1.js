// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * The Date.prototype.toTimeString property "length" has { ReadOnly, DontDelete, DontEnum } attributes
 *
 * @path ch15/15.9/15.9.5/15.9.5.4/S15.9.5.4_A3_T1.js
 * @description Checking ReadOnly attribute
 */

x = Date.prototype.toTimeString.length;
Date.prototype.toTimeString.length = 1;
if (Date.prototype.toTimeString.length !== x) {
  $ERROR('#1: The Date.prototype.toTimeString.length has the attribute ReadOnly');
}


