/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-73.js
 * @description Object.defineProperty - both desc.writable and name.writable are boolean values with the same value (8.12.9 step 6)
 */


function testcase() {

        var obj = {};

        Object.defineProperty(obj, "foo", { writable: false});

        Object.defineProperty(obj, "foo", { writable: false });
        return dataPropertyAttributesAreCorrect(obj, "foo", undefined, false, false, false);
    }
runTestCase(testcase);
