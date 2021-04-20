/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-124.js
 * @description Object.defineProperty - 'O' is an Array, 'name' is the length property of 'O', the [[Value]] field of 'desc' is absent, test updating the [[Writable]] attribute of the length property from true to false (15.4.5.1 step 3.a.i)
 */


function testcase() {

        var arrObj = [];

        Object.defineProperty(arrObj, "length", {
            writable: false
        });
        return dataPropertyAttributesAreCorrect(arrObj, "length", 0, false, false, false);
    }
runTestCase(testcase);
