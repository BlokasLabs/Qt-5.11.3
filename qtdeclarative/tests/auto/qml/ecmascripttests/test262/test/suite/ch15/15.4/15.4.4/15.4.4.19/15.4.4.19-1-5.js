/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.19/15.4.4.19-1-5.js
 * @description Array.prototype.map - applied to number primitive
 */


function testcase() {
        function callbackfn(val, idx, obj) {
            return obj instanceof Number;
        }

        try {
            Number.prototype[0] = 1;
            Number.prototype.length = 1;

            var testResult = Array.prototype.map.call(2.5, callbackfn);
            return testResult[0] === true;
        } finally {
            delete Number.prototype[0];
            delete Number.prototype.length;
        }
    }
runTestCase(testcase);
