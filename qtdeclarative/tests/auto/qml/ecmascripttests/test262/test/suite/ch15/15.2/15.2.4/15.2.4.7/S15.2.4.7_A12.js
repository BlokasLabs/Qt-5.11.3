// Copyright 2011 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

/**
 * @path ch15/15.2/15.2.4/15.2.4.7/S15.2.4.7_A12.js
 * @description Let O be the result of calling ToObject passing the this value as the argument.
 * @negative
 */

Object.prototype.propertyIsEnumerable.call(undefined, 'foo');

