/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.19/15.4.4.19-8-c-i-28.js
 * @description Array.prototype.map - element changed by getter on previous iterations is observed on an Array
 */


function testcase() {

        var preIterVisible = false;
        var arr = [];

        function callbackfn(val, idx, obj) {
            if (idx === 0) {
                return val === 11;
            } else if (idx === 1) {
                return val === 9;
            } else {
                return false;
            }
        }

        Object.defineProperty(arr, "0", {
            get: function () {
                preIterVisible = true;
                return 11;
            },
            configurable: true
        });

        Object.defineProperty(arr, "1", {
            get: function () {
                if (preIterVisible) {
                    return 9;
                } else {
                    return 11;
                }
            },
            configurable: true
        });

        var testResult = arr.map(callbackfn);

        return testResult[0] === true && testResult[1] === true;
    }
runTestCase(testcase);
