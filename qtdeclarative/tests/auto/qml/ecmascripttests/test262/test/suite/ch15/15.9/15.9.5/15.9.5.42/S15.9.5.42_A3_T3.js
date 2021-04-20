// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * The Date.prototype.toUTCString property "length" has { ReadOnly, DontDelete, DontEnum } attributes
 *
 * @path ch15/15.9/15.9.5/15.9.5.42/S15.9.5.42_A3_T3.js
 * @description Checking DontEnum attribute
 */

if (Date.prototype.toUTCString.propertyIsEnumerable('length')) {
  $ERROR('#1: The Date.prototype.toUTCString.length property has the attribute DontEnum');
}

for(x in Date.prototype.toUTCString) {
  if(x === "length") {
    $ERROR('#2: The Date.prototype.toUTCString.length has the attribute DontEnum');
  }
}


