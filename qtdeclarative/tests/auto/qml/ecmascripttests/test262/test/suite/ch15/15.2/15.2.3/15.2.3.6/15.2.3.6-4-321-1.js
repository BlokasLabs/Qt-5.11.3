/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.6/15.2.3.6-4-321-1.js
 * @description Object.defineProperty - 'O' is an Arguments object of a function that has formal parameters, 'P' is own accessor property of 'O', test TypeError is thrown when updating the [[Get]] attribute value of 'P' which is not configurable (10.6 [[DefineOwnProperty]] step 4)
 */


function testcase() {
        return (function (a, b, c) {
            function getFunc() {
                return "genericPropertyString";
            }
            function setFunc(value) {
                this.helpVerifyGet = value;
            }
            Object.defineProperty(arguments, "genericProperty", {
                get: getFunc,
                set: setFunc,
                configurable: false
            });
            try {
                Object.defineProperty(arguments, "genericProperty", {
                    get: function () {
                        return "overideGenericPropertyString";
                    }
                });
            } catch (e) {
                var verifyFormal = a === 1;
                return e instanceof TypeError &&
                    accessorPropertyAttributesAreCorrect(arguments, "genericProperty", getFunc, setFunc, "helpVerifyGet", false, false, false) && verifyFormal;
            }
            return false;
        }(1, 2, 3));
    }
runTestCase(testcase);
