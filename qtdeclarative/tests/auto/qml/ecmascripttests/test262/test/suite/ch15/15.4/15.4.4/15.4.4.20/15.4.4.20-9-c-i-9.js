/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.20/15.4.4.20-9-c-i-9.js
 * @description Array.prototype.filter - element to be retrieved is own accessor property on an Array-like object
 */


function testcase() {

        function callbackfn(val, idx, obj) {
            return (idx === 0) && (val === 11);
        }

        var obj = { 10: 10, length: 20 };

        Object.defineProperty(obj, "0", {
            get: function () {
                return 11;
            },
            configurable: true
        });

        var newArr = Array.prototype.filter.call(obj, callbackfn);
        return newArr.length === 1 && newArr[0] === 11;
    }
runTestCase(testcase);
