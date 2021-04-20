/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.18/15.4.4.18-2-1.js
 * @description Array.prototype.forEach - 'length' is own data property on an Array-like object
 */


function testcase() {
        var result = false;
        function callbackfn(val, idx, obj) {
            result = (obj.length === 2);
        }

        var obj = {
            0: 12,
            1: 11,
            2: 9,
            length: 2
        };

        Array.prototype.forEach.call(obj, callbackfn);
        return result;
    }
runTestCase(testcase);
