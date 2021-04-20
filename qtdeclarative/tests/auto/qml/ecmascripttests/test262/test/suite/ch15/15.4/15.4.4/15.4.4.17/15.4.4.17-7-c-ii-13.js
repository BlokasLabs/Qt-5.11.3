/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.17/15.4.4.17-7-c-ii-13.js
 * @description Array.prototype.some - callbackfn that uses arguments object to get parameter value
 */


function testcase() {

        function callbackfn() {
            return arguments[2][arguments[1]] === arguments[0];
        }

        return [9, 12].some(callbackfn);
    }
runTestCase(testcase);
