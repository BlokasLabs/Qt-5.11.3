/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.14/15.4.4.14-2-15.js
 * @description Array.prototype.indexOf - 'length' is property of the global object
 */


function testcase() {
        var targetObj = {};
        try {
            var oldLen = fnGlobalObject().length;
            fnGlobalObject().length = 2;

            fnGlobalObject()[1] = targetObj;
            if (Array.prototype.indexOf.call(fnGlobalObject(), targetObj) !== 1) {
                return false;
            }

            fnGlobalObject()[1] = {};
            fnGlobalObject()[2] = targetObj;

            return Array.prototype.indexOf.call(fnGlobalObject(), targetObj) === -1;
        } finally {
            delete fnGlobalObject()[1];
            delete fnGlobalObject()[2];
            fnGlobalObject().length = oldLen;
        }
    }
runTestCase(testcase);
