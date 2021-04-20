/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.17/15.4.4.17-3-19.js
 * @description Array.prototype.some - value of 'length' is an Object which has an own toString method
 */


function testcase() {
        function callbackfn1(val, idx, obj) {
            return val > 10;
        }

        function callbackfn2(val, idx, obj) {
            return val > 11;
        }

        var toStringAccessed = false;
        var obj = {
            0: 9,
            1: 11,
            2: 12,

            length: {
                toString: function () {
                    toStringAccessed = true;
                    return '2';
                }
            }
        };

        return Array.prototype.some.call(obj, callbackfn1) &&
            !Array.prototype.some.call(obj, callbackfn2) && toStringAccessed;
    }
runTestCase(testcase);
