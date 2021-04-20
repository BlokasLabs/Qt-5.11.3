/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.18/15.4.4.18-5-2.js
 * @description Array.prototype.forEach - thisArg is Object
 */


function testcase() {
  var res = false;
  var o = new Object();
  o.res = true;
  var result;
  function callbackfn(val, idx, obj)
  {
    result = this.res;
  }

  var arr = [1];
  arr.forEach(callbackfn,o)
  if( result === true)
    return true;    

 }
runTestCase(testcase);
