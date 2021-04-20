/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.17/15.4.4.17-7-b-12.js
 * @description Array.prototype.some - deleting own property with prototype property causes prototype index property to be visited on an Array-like object
 */


function testcase() {
        function callbackfn(val, idx, obj) {
            if (idx === 1 && val === 1) {
                return true;
            } else {
                return false;
            }
        }
        var arr = { 0: 0, 1: 111, 2: 2, length: 10 };
        
        Object.defineProperty(arr, "0", {
            get: function () {
                delete arr[1];
                return 0;
            },
            configurable: true
        });

        try {
            Object.prototype[1] = 1;
            return Array.prototype.some.call(arr, callbackfn);
        } finally {
            delete Object.prototype[1];
        }
    }
runTestCase(testcase);
