/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.18/15.4.4.18-7-8.js
 * @description Array.prototype.forEach - no observable effects occur if len is 0
 */


function testcase() {
        var accessed = false;

        function callbackfn(val, idx, obj) {
            accessed = true;
        }

        var obj = { 0: 11, 1: 12, length: 0 };

        Array.prototype.forEach.call(obj, callbackfn);
        return !accessed;
    }
runTestCase(testcase);
