/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.16/15.4.4.16-1-9.js
 * @description Array.prototype.every applied to Function object
 */


function testcase() {
        function callbackfn(val, idx, obj) {
            return !(obj instanceof Function);
        }

        var obj = function (a, b) {
            return a + b;
        };
        obj[0] = 11;
        obj[1] = 9;

        return !Array.prototype.every.call(obj, callbackfn);
    }
runTestCase(testcase);
