/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.4/15.2.3.4-4-b-6.js
 * @description Object.getOwnPropertyNames - elements of the returned array are configurable
 */


function testcase() {
        var obj = { "a": "a" };

        var result = Object.getOwnPropertyNames(obj);

        var beforeDeleted = (result.hasOwnProperty("0"));
        delete result[0];
        var afterDeleted = (result.hasOwnProperty("0"));

        return beforeDeleted && !afterDeleted;
    }
runTestCase(testcase);
