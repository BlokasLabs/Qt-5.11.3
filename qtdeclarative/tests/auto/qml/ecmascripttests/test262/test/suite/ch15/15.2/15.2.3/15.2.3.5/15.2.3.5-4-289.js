/// Copyright (c) 2012 Ecma International.  All rights reserved. 
/// Ecma International makes this code available under the terms and conditions set
/// forth on http://hg.ecmascript.org/tests/test262/raw-file/tip/LICENSE (the 
/// "Use Terms").   Any redistribution of this code must retain the above 
/// copyright and this notice and otherwise comply with the Use Terms.
/**
 * @path ch15/15.2/15.2.3/15.2.3.5/15.2.3.5-4-289.js
 * @description Object.create - one property in 'Properties' is an Arguments object which implements its own [[Get]] method to access the 'set' property (8.10.5 step 8.a)
 */


function testcase() {
        var argObj = (function () { return arguments; })();

        var data = "data";

        argObj.set = function (value) {
            data = value;
        };

        var newobj = Object.create({}, {
            prop: argObj
        });

        var hasProperty = newobj.hasOwnProperty("prop");

        newobj.prop = "overrideData";

        return hasProperty && data === "overrideData";
    }
runTestCase(testcase);
