/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-3-86.js
 * @description Object.defineProperty - 'Attributes' is a Function object which implements its own [[Get]] method to access the 'configurable' property (8.10.5 step 4.a)
 */


function testcase() {
        var obj = {};

        var funObj = function (a, b) {
            return a + b;
        };

        funObj.configurable = true;

        Object.defineProperty(obj, "property", funObj);

        var beforeDeleted = obj.hasOwnProperty("property");

        delete obj.property;

        var afterDeleted = obj.hasOwnProperty("property");

        return beforeDeleted === true && afterDeleted === false;
    }
runTestCase(testcase);
