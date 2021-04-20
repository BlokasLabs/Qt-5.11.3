/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-289.js
 * @description Object.defineProperty - 'O' is an Arguments object, 'name' is own property of 'O', and is deleted afterwards, and 'desc' is data descriptor, test 'name' is redefined in 'O' with all correct attribute values (10.6 [[DefineOwnProperty]] step 3)
 */


function testcase() {
        return (function () { 
            delete arguments[0];
            Object.defineProperty(arguments, "0", {
                value: 10,
                writable: true,
                enumerable: true,
                configurable: true
            });
            return dataPropertyAttributesAreCorrect(arguments, "0", 10, true, true, true);
        }(0, 1, 2));
    }
runTestCase(testcase);
