/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.10/15.2.3.10-3-13.js
 * @description Object.preventExtensions - named properties cannot be added into a Function object
 */


function testcase() {
        var funObj = function () { };
        var preCheck = Object.isExtensible(funObj);
        Object.preventExtensions(funObj);

        funObj.exName = 2;
        return preCheck && !funObj.hasOwnProperty("exName");
    }
runTestCase(testcase);
