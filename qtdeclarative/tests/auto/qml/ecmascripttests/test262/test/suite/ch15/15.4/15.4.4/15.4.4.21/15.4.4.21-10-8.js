/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.21/15.4.4.21-10-8.js
 * @description Array.prototype.reduce doesn't visit expandos
 */


function testcase() {

  var callCnt = 0;
  function callbackfn(prevVal, curVal,  idx, obj)
  {
    callCnt++;
    return curVal;
  }
  var srcArr = ['1','2','3','4','5'];
  srcArr["i"] = 10;
  srcArr[true] = 11;
  srcArr.reduce(callbackfn);

  if(callCnt == 4)
  {
    return true;
  }

 }
runTestCase(testcase);
