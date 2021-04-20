// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * length property has the attributes {DontDelete}
 *
 * @path ch15/15.5/15.5.5/S15.5.5.1_A3.js
 * @description Checking if deleting the length property of String fails
 */

var __str__instance = new String("globglob");

//////////////////////////////////////////////////////////////////////////////
//CHECK#1
if (!(__str__instance.hasOwnProperty("length"))) {
  $ERROR('#1: var __str__instance = new String("globglob"); __str__instance.hasOwnProperty("length") return true. Actual: '+__str__instance.hasOwnProperty("length"));
}
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//CHECK#2
if (delete __str__instance === true) {
  $ERROR('#2: var __str__instance = new String("globglob"); delete __str__instance !== true');
}
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//CHECK#3
if (!(__str__instance.hasOwnProperty("length"))) {
  $ERROR('#3: var __str__instance = new String("globglob"); delete __str__instance; __str__instance.hasOwnProperty("length") return true. Actual: '+__str__instance.hasOwnProperty("length"));
}
//
//////////////////////////////////////////////////////////////////////////////

