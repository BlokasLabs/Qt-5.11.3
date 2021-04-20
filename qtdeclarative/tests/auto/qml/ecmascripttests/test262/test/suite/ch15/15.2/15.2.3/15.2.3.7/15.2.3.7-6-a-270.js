/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.7/15.2.3.7-6-a-270.js
 * @description Object.defineProperties - 'O' is an Array, 'P' is generic own data property of 'O', test TypeError is thrown when updating the [[Value]] attribute value of 'P' which is defined as unwritable and non-configurable (15.4.5.1 step 5)
 */


function testcase() {

        var arr = [];

        Object.defineProperty(arr, "property", {
            value: 12
        });

        try {
            Object.defineProperties(arr, {
                "property": {
                    value: 36
                }
            });
            return false;
        } catch (ex) {
            return (ex instanceof TypeError) && dataPropertyAttributesAreCorrect(arr, "property", 12, false, false, false);
        }
    }
runTestCase(testcase);
