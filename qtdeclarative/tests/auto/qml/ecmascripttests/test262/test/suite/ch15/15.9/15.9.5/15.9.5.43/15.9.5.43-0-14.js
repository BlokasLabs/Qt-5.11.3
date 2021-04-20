/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.9/15.9.5/15.9.5.43/15.9.5.43-0-14.js
 * @description Date.prototype.toISOString - when value of year is -Infinity Date.prototype.toISOString throw the RangeError
 */


function testcase() {
        var date = new Date(-Infinity, 1, 70, 0, 0, 0);

        try {
            date.toISOString();
        } catch (ex) {
            return ex instanceof RangeError;
        }
    }
runTestCase(testcase);
