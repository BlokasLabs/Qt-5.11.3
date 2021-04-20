/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.7/15.2.3.7-6-a-157.js
 * @description Object.defineProperties - 'O' is an Array, 'P' is the length property of 'O', test the [[Value]] field of 'desc' which is less than value of the length property is defined into 'O' with deleting properties with large index named (15.4.5.1 step 3.f)
 */


function testcase() {

        var arr = [0, 1];

        Object.defineProperties(arr, {
            length: {
                value: 1
            }
        });
        return arr.length === 1 && !arr.hasOwnProperty("1") && arr[0] === 0;
    }
runTestCase(testcase);
