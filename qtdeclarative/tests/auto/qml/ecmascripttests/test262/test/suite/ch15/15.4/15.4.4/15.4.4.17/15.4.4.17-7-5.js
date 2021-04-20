/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.17/15.4.4.17-7-5.js
 * @description Array.prototype.some doesn't consider newly added elements in sparse array
 */


function testcase() { 
 
  function callbackfn(val, idx, obj)
  {
    arr[1000] = 5;
    if(val < 5)
      return false;
    else 
      return true;
  }

  var arr = new Array(10);
  arr[1] = 1;
  arr[2] = 2;
  
  if(arr.some(callbackfn) === false)    
    return true;  
 
 }
runTestCase(testcase);
