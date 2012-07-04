exports.VERSION = '0.5.5'

var Bindings = exports.Bindings = require('ffi_bindings')
exports.free = Bindings.free
exports.CallbackInfo = Bindings.CallbackInfo
exports.TYPE_SIZE_MAP = Bindings.TYPE_SIZE_MAP

exports.Struct = require('ffi_struct')
exports.FFI_TYPE = exports.Struct([
    ['size_t', 'size']
  , ['ushort', 'alignment']
  , ['ushort', 'type']
  , ['pointer','elements']
])

var Util = require('ffi_util')
exports.NON_SPECIFIC_TYPES = Util.NON_SPECIFIC_TYPES
exports.TYPE_TO_POINTER_METHOD_MAP = Util.TYPE_TO_POINTER_METHOD_MAP
exports.SIZE_TO_POINTER_METHOD_MAP = Util.SIZE_TO_POINTER_METHOD_MAP
exports.PLATFORM_LIBRARY_EXTENSIONS = Util.PLATFORM_LIBRARY_EXTENSIONS
exports.isValidParamType = Util.isValidParamType
exports.isValidReturnType = Util.isValidReturnType
exports.derefValuePtr = Util.derefValuePtr
exports.derefValuePtrFunc = Util.derefValuePtrFunc
exports.sizeOf = Util.sizeOf
exports.ffiTypeFor = Util.ffiTypeFor
exports.isStructType = Util.isStructType

exports.CIF = require('ffi_cif')
exports.ForeignFunction = require('ffi_foreign_function')
exports.DynamicLibrary = require('ffi_dynamic_library')
exports.Library = require('ffi_library')
exports.Callback = require('ffi_callback')
exports.Pointer = require('ffi_pointer')
exports.errno = require('ffi_errno')


