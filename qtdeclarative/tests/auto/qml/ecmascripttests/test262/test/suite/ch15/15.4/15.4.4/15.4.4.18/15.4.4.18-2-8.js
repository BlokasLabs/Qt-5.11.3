/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.4/15.4.4/15.4.4.18/15.4.4.18-2-8.js
 * @description Array.prototype.forEach applied to Array-like object, 'length' is an own accessor property that overrides an inherited data property
 */


function testcase() {
        var result = false;
        function callbackfn(val, idx, obj) {
            result = (obj.length === 2);
        }

        var proto = { length: 3 };

        var Con = function () { };
        Con.prototype = proto;

        var child = new Con();

        Object.defineProperty(child, "length", {
            get: function () {
                return 2;
            },
            configurable: true
        });

        child[0] = 12;
        child[1] = 11;
        child[2] = 9;

        Array.prototype.forEach.call(child, callbackfn);
        return result;
    }
runTestCase(testcase);
