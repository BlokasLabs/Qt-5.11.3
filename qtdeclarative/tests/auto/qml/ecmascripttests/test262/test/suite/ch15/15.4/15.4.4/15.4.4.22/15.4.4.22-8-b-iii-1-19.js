/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.22/15.4.4.22-8-b-iii-1-19.js
 * @description Array.prototype.reduceRight - element to be retrieved is own accessor property without a get function that overrides an inherited accessor property on an Array-like object
 */


function testcase() {

        var testResult = false;
        function callbackfn(prevVal, curVal, idx, obj) {
            if (idx === 1) {
                testResult = (typeof prevVal === "undefined");
            }
        }

        try {
            Object.prototype[2] = 2;

            var obj = { 0: 0, 1: 1, length: 3 };
            Object.defineProperty(obj, "2", {
                set: function () { },
                configurable: true
            });

            Array.prototype.reduceRight.call(obj, callbackfn);
            return testResult;
        } finally {
            delete Object.prototype[2];
        }

    }
runTestCase(testcase);
