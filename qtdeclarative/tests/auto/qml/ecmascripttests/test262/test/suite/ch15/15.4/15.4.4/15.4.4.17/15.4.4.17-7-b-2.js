/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.17/15.4.4.17-7-b-2.js
 * @description Array.prototype.some - added properties in step 2 are visible here
 */


function testcase() {
        function callbackfn(val, idx, obj) {
            if (idx === 2 && val === "length") {
                return true;
            } else {
                return false;
            }
        }
        
        var arr = { };

        Object.defineProperty(arr, "length", {
            get: function () {
                arr[2] = "length";
                return 3;
            },
            configurable: true
        });

        return Array.prototype.some.call(arr, callbackfn);
    }
runTestCase(testcase);
