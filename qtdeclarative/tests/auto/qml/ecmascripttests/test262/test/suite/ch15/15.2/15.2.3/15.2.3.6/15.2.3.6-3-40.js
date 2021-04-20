/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-3-40.js
 * @description Object.defineProperty - 'Attributes' is an RegExp object that uses Object's [[Get]] method to access the 'enumerable' property (8.10.5 step 3.a)
 */


function testcase() {
        var obj = {};
        var accessed = false;

        var regObj = new RegExp();
        regObj.enumerable = true;

        Object.defineProperty(obj, "property", regObj);

        for (var prop in obj) {
            if (prop === "property") {
                accessed = true;
            }
        }

        return accessed;
    }
runTestCase(testcase);
