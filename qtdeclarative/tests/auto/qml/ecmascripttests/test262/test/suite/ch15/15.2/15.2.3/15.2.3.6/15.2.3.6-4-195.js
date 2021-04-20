/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-195.js
 * @description Object.defineProperty - 'O' is an Array, 'name' is an array index named property, 'name' is an inherited accessor property (15.4.5.1 step 4.c)
 */


function testcase() {
        try {
            Object.defineProperty(Array.prototype, "0", {
                get: function () {
                    return 11;
                },
                configurable: true
            });

            var arrObj = [];

            function getFunc() {
                return arrObj.helpVerifySet;
            }
            function setFunc(value) {
                arrObj.helpVerifySet = value;
            }

            Object.defineProperty(arrObj, "0", {
                get: getFunc,
                set: setFunc,
                configurable: false
            });

            arrObj[0] = 13;

            return accessorPropertyAttributesAreCorrect(arrObj, "0", getFunc, setFunc, "helpVerifySet", false, false);
        } finally {
            delete Array.prototype[0];
        }
    }
runTestCase(testcase);
