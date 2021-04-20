/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.14/15.4.4.14-3-19.js
 * @description Array.prototype.indexOf - value of 'length' is an Object which has an own toString method.
 */


function testcase() {

        // objects inherit the default valueOf() method from Object
        // that simply returns itself. Since the default valueOf() method
        // does not return a primitive value, ES next tries to convert the object
        // to a number by calling its toString() method and converting the
        // resulting string to a number.

        var obj = {
            1: true,
            2: 2,

            length: {
                toString: function () {
                    return '2';
                }
            }
        };

        return Array.prototype.indexOf.call(obj, true) === 1 &&
            Array.prototype.indexOf.call(obj, 2) === -1;
    }
runTestCase(testcase);
