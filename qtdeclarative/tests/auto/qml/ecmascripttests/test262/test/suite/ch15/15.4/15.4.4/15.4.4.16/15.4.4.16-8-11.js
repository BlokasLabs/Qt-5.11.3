/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.16/15.4.4.16-8-11.js
 * @description Array.prototype.every returns true when all calls to callbackfn return true
 */


function testcase() { 
 
  var callCnt = 0;
  function callbackfn(val, idx, obj)
  {
    callCnt++;
    return true;
  }

  var arr = [0,1,2,3,4,5,6,7,8,9];
  
  if(arr.every(callbackfn) === true && callCnt === 10) 
    return true;
 }
runTestCase(testcase);
