/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.21/15.4.4.21-9-b-22.js
 * @description Array.prototype.reduce - deleting own property causes deleted index property not to be visited on an Array
 */


function testcase() {

        var accessed = false;
        var testResult = true;

        function callbackfn(accum, val, idx, obj) {
            accessed = true;
            if (idx === 1) {
                testResult = false;
            }
        }

        var arr = [1, 2, 4];

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

        arr.reduce(callbackfn, "initialValue");
        return testResult && accessed;
    }
runTestCase(testcase);
