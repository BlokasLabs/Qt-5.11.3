/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.7/15.2.3.7-6-a-298.js
 * @description Object.defineProperties - 'O' is an Arguments object, 'P' is an array index named accessor property of 'O' but not defined in [[ParameterMap]] of 'O', test TypeError is thrown when updating the [[Get]] attribute value of 'P' which is not configurable (10.6 [[DefineOwnProperty]] step 4)
 */


function testcase() {

        var arg;

        (function fun() {
            arg = arguments;
        }());

        function get_func1() {
            return 0;
        }

        Object.defineProperty(arg, "0", {
            get: get_func1,
            enumerable: false,
            configurable: false
        });

        function get_func2() {
            return 10;
        }
        try {
            Object.defineProperties(arg, {
                "0": {
                    get: get_func2
                }
            });
            return false;
        } catch (e) {
            return (e instanceof TypeError) && accessorPropertyAttributesAreCorrect(arg, "0", get_func1, undefined, undefined, false, false);
        }
    }
runTestCase(testcase);
