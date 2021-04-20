/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.17/15.4.4.17-7-c-ii-3.js
 * @description Array.prototype.some immediately returns true if callbackfn returns true
 */


function testcase() { 
 
  var callCnt = 0;
  function callbackfn(val, idx, obj)
  {
    callCnt++;
    if(idx > 5)   
      return true;
    else
      return false;
  }

  var arr = [0,1,2,3,4,5,6,7,8,9];
  
  if(arr.some(callbackfn) === true && callCnt === 7) 
    return true;
 }
runTestCase(testcase);
