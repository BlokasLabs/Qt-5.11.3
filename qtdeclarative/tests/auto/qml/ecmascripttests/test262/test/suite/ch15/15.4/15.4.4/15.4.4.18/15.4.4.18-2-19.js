/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.18/15.4.4.18-2-19.js
 * @description Array.prototype.forEach applied to Function object, which implements its own property get method
 */


function testcase() {
        var result = false;
        function callbackfn(val, idx, obj) {
            result = (obj.length === 2);
        }

        var fun = function (a, b) {
            return a + b;
        };
        fun[0] = 12;
        fun[1] = 11;
        fun[2] = 9;

        Array.prototype.forEach.call(fun, callbackfn);
        return result;
    }
runTestCase(testcase);
