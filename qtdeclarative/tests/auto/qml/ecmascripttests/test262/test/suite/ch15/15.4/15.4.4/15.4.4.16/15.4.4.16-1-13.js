/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.16/15.4.4.16-1-13.js
 * @description Array.prototype.every applied to the JSON object
 */


function testcase() {
        function callbackfn(val, idx, obj) {
            return ('[object JSON]' !== Object.prototype.toString.call(obj));
        }

        try {
            JSON.length = 1;
            JSON[0] = 1;
            return !Array.prototype.every.call(JSON, callbackfn);
        } finally {
            delete JSON.length;
            delete JSON[0];
        }
    }
runTestCase(testcase);
