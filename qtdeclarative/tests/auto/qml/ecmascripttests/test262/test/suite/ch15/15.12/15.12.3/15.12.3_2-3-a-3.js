/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.12/15.12.3/15.12.3_2-3-a-3.js
 * @description JSON.stringify converts Boolean wrapper objects returned from replacer functions to literal numbers.
 */


function testcase() {
  return JSON.stringify([42], function(k,v) {return v===42? new Boolean(false):v}) === '[false]';
  }
runTestCase(testcase);
