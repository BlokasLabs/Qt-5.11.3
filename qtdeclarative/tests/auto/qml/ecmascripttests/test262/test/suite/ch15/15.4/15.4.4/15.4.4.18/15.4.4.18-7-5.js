/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.18/15.4.4.18-7-5.js
 * @description Array.prototype.forEach visits deleted element in array after the call when same index is also present in prototype
 */


function testcase() { 
 
  var callCnt = 0;
  function callbackfn(val, idx, obj)
  {
    delete arr[4];
    callCnt++;
  }

  Array.prototype[4] = 5;

  var arr = [1,2,3,4,5];
  arr.forEach(callbackfn)
  delete Array.prototype[4];
  if( callCnt === 5)    
      return true;  
  
 }
runTestCase(testcase);
