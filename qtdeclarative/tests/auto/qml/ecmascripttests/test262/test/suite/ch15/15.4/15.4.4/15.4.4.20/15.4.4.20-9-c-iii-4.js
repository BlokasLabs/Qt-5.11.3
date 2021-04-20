/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.20/15.4.4.20-9-c-iii-4.js
 * @description Array.prototype.filter - return value of callbackfn is a boolean (value is false)
 */


function testcase() {

        var accessed = false;

        function callbackfn(val, idx, obj) {
            accessed = true;
            return false;
        }

        var obj = { 0: 11, length: 1 };

        var newArr = Array.prototype.filter.call(obj, callbackfn);
        return newArr.length === 0 && accessed;
    }
runTestCase(testcase);
