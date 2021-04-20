/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-161.js
 * @description Object.defineProperty - 'O' is an Array, 'name' is the length property of 'O', set the [[Value]] field of 'desc' to a value lesser than the existing value of length and test that indexes beyond the new length are deleted(15.4.5.1 step 3.f)
 */


function testcase() {

        var arrObj = [0, 1];
        
        Object.defineProperty(arrObj, "length", {
            value: 1
        });
        return arrObj.length === 1 && !arrObj.hasOwnProperty("1");
    }
runTestCase(testcase);
