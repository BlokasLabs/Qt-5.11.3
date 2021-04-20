/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.17/15.4.4.17-7-c-ii-7.js
 * @description Array.prototype.some - unhandled exceptions happened in callbackfn terminate iteration
 */


function testcase() {

        var accessed = false;

        function callbackfn(val, idx, obj) {
            if (idx > 0) {
                accessed = true;
            }
            if (idx === 0) {
                throw new Error("Exception occurred in callbackfn");
            }
            return false;
        }

        var obj = { 0: 9, 1: 100, 10: 11, length: 20 };

        try {
            Array.prototype.some.call(obj, callbackfn);
            return false;
        } catch (ex) {
            return ex instanceof Error && !accessed;
        }
    }
runTestCase(testcase);
