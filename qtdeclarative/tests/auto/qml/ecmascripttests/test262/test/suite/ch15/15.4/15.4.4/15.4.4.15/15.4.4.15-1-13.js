/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.15/15.4.4.15-1-13.js
 * @description Array.prototype.lastIndexOf applied to the JSON object
 */


function testcase() {

        var targetObj = {};
        try {
            JSON[3] = targetObj;
            JSON.length = 5;
            return 3 === Array.prototype.lastIndexOf.call(JSON, targetObj);
        } finally {
            delete JSON[3];
            delete JSON.length;
        }
    }
runTestCase(testcase);
