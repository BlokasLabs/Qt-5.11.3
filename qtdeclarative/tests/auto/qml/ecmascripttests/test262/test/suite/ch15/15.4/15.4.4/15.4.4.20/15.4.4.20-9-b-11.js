/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.20/15.4.4.20-9-b-11.js
 * @description Array.prototype.filter - deleting property of prototype causes prototype index property not to be visited on an Array
 */


function testcase() {

        function callbackfn(val, idx, obj) {
            return true;
        }
        var arr = [0, , 2];

        Object.defineProperty(arr, "0", {
            get: function () {
                delete Array.prototype[1];
                return 0;
            },
            configurable: true
        });

        try {
            Array.prototype[1] = 1;
            var newArr = arr.filter(callbackfn);
            return newArr.length === 2 && newArr[1] !== 1;
        } finally {
            delete Array.prototype[1];
        }
    }
runTestCase(testcase);
