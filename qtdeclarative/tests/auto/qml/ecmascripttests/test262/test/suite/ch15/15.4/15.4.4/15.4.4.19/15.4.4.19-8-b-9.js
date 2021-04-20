/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.19/15.4.4.19-8-b-9.js
 * @description Array.prototype.map - deleting own property causes index property not to be visited on an Array
 */


function testcase() {

        function callbackfn(val, idx, obj) {
            if (idx === 1) {
                return false;
            } else {
                return true;
            }
        }
        var arr = [1, 2];

        Object.defineProperty(arr, "1", {
            get: function () {
                return "6.99";
            },
            configurable: true
        });

        Object.defineProperty(arr, "0", {
            get: function () {
                delete arr[1];
                return 0;
            },
            configurable: true
        });

        var testResult = arr.map(callbackfn);
        return testResult[0] === true && typeof testResult[1] === "undefined";
    }
runTestCase(testcase);
