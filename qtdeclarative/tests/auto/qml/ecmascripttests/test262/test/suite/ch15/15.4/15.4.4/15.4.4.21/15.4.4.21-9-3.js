/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.21/15.4.4.21-9-3.js
 * @description Array.prototype.reduce doesn't visit deleted elements in array after the call
 */


function testcase() { 
 
  function callbackfn(prevVal, curVal, idx, obj)  
  {
    delete arr[3];
    delete arr[4];
    return prevVal + curVal;    
  }

  var arr = ['1',2,3,4,5];
  if(arr.reduce(callbackfn) === "123"  )    // two elements deleted
    return true;  
  
 }
runTestCase(testcase);
