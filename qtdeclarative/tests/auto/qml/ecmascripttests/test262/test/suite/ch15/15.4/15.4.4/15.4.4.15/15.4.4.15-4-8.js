/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.15/15.4.4.15-4-8.js
 * @description Array.prototype.lastIndexOf returns -1 if 'length' is 0 (length is an empty array)
 */


function testcase() {

  
  // objects inherit the default valueOf method of the Object object;
  // that simply returns the itself. Since the default valueOf() method
  // does not return a primitive value, ES next tries to convert the object
  // to a number by calling its toString() method and converting the
  // resulting string to a number.
  //
  // The toString( ) method on Array converts the array elements to strings,
  // then returns the result of concatenating these strings, with commas in
  // between. An array with no elements converts to the empty string, which
  // converts to the number 0. If an array has a single element that is a
  // number n, the array converts to a string representation of n, which is
  // then converted back to n itself. If an array contains more than one element,
  // or if its one element is not a number, the array converts to NaN.
 var i = Array.prototype.lastIndexOf.call({length: [ ]}, 1);
  
  if (i === -1) {
    return true;
  }
 }
runTestCase(testcase);
