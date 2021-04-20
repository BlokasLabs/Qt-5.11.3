/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.7/15.2.3.7-6-a-262.js
 * @description Object.defineProperties - 'O' is an Array, 'P' is an array index named property that already exists on 'O' is accessor property and 'desc' is accessor descriptor, test updating multiple attribute values of 'P'  (15.4.5.1 step 4.c)
 */


function testcase() {

        var arr = [];

        function get_fun() {
            return 36;
        }
        function set_fun(value) {
            arr.setVerifyHelpProp = value;
        }
        Object.defineProperty(arr, "0", {
            get: function () {
                return 12;
            },
            set: set_fun,
            enumerable: true,
            configurable: true
        });

        try {
            Object.defineProperties(arr, {
                "0": {
                    get: get_fun,
                    enumerable: false,
                    configurable: false
                }
            });
            return accessorPropertyAttributesAreCorrect(arr, "0", get_fun, set_fun, "setVerifyHelpProp", false, false);
        } catch (ex) {
            return false;
        }
    }
runTestCase(testcase);
