/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.20/15.4.4.20-2-17.js
 * @description Array.prototype.filter applied to the Arguments object, which implements its own property get method
 */


function testcase() {

        function callbackfn(val, idx, obj) {
            return obj.length === 2;
        }

        var func = function (a, b) {
            var newArr = Array.prototype.filter.call(arguments, callbackfn);
            return newArr.length === 2;
        };

        return func(12, 11);
    }
runTestCase(testcase);
