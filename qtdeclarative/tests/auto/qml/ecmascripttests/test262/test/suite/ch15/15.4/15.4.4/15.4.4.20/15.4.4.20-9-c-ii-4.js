/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.20/15.4.4.20-9-c-ii-4.js
 * @description Array.prototype.filter - k values are passed in ascending numeric order
 */


function testcase() {

        var arr = [0, 1, 2, 3, 4, 5];
        var lastIdx = 0;
        var called = 0;
        function callbackfn(val, idx, o) {
            called++;
            if (lastIdx !== idx) {
                return false;
            } else {
                lastIdx++;
                return true;
            }
        }
        var newArr = arr.filter(callbackfn);

        return newArr.length === called;
    }
runTestCase(testcase);
