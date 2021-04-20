/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.14/15.2.3.14-3-1.js
 * @description Object.keys returns the standard built-in Array containing own enumerable properties
 */


function testcase() {
  var o = { x: 1, y: 2};

  var a = Object.keys(o);
  if (a.length === 2 &&
      a[0] === 'x' &&
      a[1] === 'y') {
    return true;
  }
 }
runTestCase(testcase);
