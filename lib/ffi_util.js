var Bindings = require('ffi_bindings')

exports.TYPE_TO_POINTER_METHOD_MAP = {
    'uint8':   'UInt8'
  , 'int8':    'Int8'
  , 'uint8':   'UInt8'
  , 'int16':   'Int16'
  , 'uint16':  'UInt16'
  , 'int32':   'Int32'
  , 'uint32':  'UInt32'
  , 'int64':   'Int64'
  , 'uint64':  'UInt64'
  , 'float':   'Float'
  , 'double':  'Double'
  , 'string':  'CString'
  , 'pointer': 'Pointer'
}

exports.SIZE_TO_POINTER_METHOD_MAP = {
    1: 'Int8'
  , 2: 'Int16'
  , 4: 'Int32'
  , 8: 'Int64'
}

exports.PLATFORM_LIBRARY_EXTENSIONS = {
    'linux':  '.so'
  , 'linux2': '.so'
  , 'sunos':  '.so'
  , 'solaris':'.so'
  , 'darwin': '.dylib'
  , 'mac':    '.dylib'
  , 'win32':  '.dll'
}

// A list of types with no hard C++ methods to read/write them
exports.NON_SPECIFIC_TYPES = {
    'byte':      'Byte'
  , 'char':      'Char'
  , 'uchar':     'UChar'
  , 'short':     'Short'
  , 'ushort':    'UShort'
  , 'int':       'Int'
  , 'uint':      'UInt'
  , 'long':      'Long'
  , 'ulong':     'ULong'
  , 'longlong':  'LongLong'
  , 'ulonglong': 'ULongLong'
  , 'size_t':    'SizeT'
}

// Returns true if the passed type is a valid param type
exports.isValidParamType = function(type) {
  return exports.isStructType(type) || Bindings.FFI_TYPES[type] != undefined
}

// Returns true if the passed type is a valid return type
exports.isValidReturnType = function(type) {
  return exports.isValidParamType(type) || type == 'void'
}

exports.derefValuePtr = function(type, ptr) {
  if (!exports.isValidParamType(type)) {
    throw new Error('Invalid Type: ' + type)
  }

  if (exports.isStructType(type)) {
    return new type(ptr)
  }

  if (type == 'void') {
    return null
  }

  var dptr = ptr

  if (type == 'string') {
    dptr = ptr.getPointer()
    if (dptr.isNull()) {
      return null
    }
  }

  return dptr['get' + exports.TYPE_TO_POINTER_METHOD_MAP[type]]()
}

// Generates a derefValuePtr for a specific type
exports.derefValuePtrFunc = function(type) {
  if (!exports.isValidParamType(type)) {
    throw new Error('Invalid Type: ' + type)
  }

  if (exports.isStructType(type)) {
    return function(ptr) {
      return new type(ptr)
    }
  }

  if (type == 'void') {
    return function(ptr) { return null; }
  }

  var getf = 'get' + exports.TYPE_TO_POINTER_METHOD_MAP[type]

  if (type == 'string') {
    return function(ptr) {
      var dptr = ptr.getPointer()
      if (dptr.isNull()) {
        return null
      }
      return dptr[getf]()
    }
  } else {
    return function(ptr) {
      return ptr[getf]()
    }
  }
}

/**
 * Returns the byte size of the given type. `type` may be a string name
 * identifier or a Struct type.
 * Roughly equivalent to the C sizeof() operator.
 */

exports.sizeOf = function sizeof (type) {
  return exports.isStructType(type)
      ? type.__structInfo__.size
      : Bindings.TYPE_SIZE_MAP[type]
}

/**
 * Returns the FFI_TYPE for the given `type`. May be a `Struct` type.
 */

exports.ffiTypeFor = function ffiTypeFor (type) {
  return exports.isStructType(type)
      ? type._ffiType().ref()
      : Bindings.FFI_TYPES[type]
}

/**
 * Returns true if the given `type` is a Struct type, false otherwise.
 */

exports.isStructType = function isStructType (type) {
  return !!type.__isStructType__
}

