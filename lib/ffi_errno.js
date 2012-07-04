
/**
 * Implementation of errno. This is a #define :/
 * On Linux, it's a global variable with the symbol `errno`,
 * but on Darwin it's a method execution called `__error`.
 */

var ForeignFunction = require('ffi_foreign_function')
  , DynamicLibrary = require('ffi_dynamic_library')
  , errnoPtr = null

if (process.platform == 'darwin' || process.platform == 'mac') {
  var __error = new DynamicLibrary().get('__error')
  errnoPtr = ForeignFunction(__error, 'pointer', [])
} else if (process.platform == 'win32') {
  var _errno = new DynamicLibrary('msvcrt.dll').get('_errno')
  errnoPtr = ForeignFunction(_errno, 'pointer', [])
} else {
  var errnoGlobal = new DynamicLibrary().get('errno');
  errnoPtr = function () { return errnoGlobal }
}

function errno () {
  return errnoPtr().getInt32()
}
module.exports = errno
