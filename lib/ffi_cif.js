var Bindings = require('ffi_bindings')
  , Util = require('ffi_util')
  , Pointer = require('ffi_pointer')

/**
 * CIF provides a JS interface for the libffi "callback info" (CIF) structure.
 * TODO: Deprecate this class. Turn this into a simple function that returns the
 *       CIF pointer.
 */

function CIF (rtype, types) {

  if (!Util.isValidReturnType(rtype)) {
    throw new Error('Invalid Return Type: ' + rtype)
  }

  var numArgs = types.length

  this._argtypesptr = new Pointer(types.length * Bindings.FFI_TYPE_SIZE)
  this._rtypeptr = Util.ffiTypeFor(rtype)

  var tptr = this._argtypesptr.clone()

  for (var i=0; i<numArgs; i++) {
    var typeName = types[i]

    if (!Util.isValidParamType(typeName)) {
      throw new Error('Invalid Type: ' + typeName)
    }

    var ffiType = Util.ffiTypeFor(typeName)
    tptr.putPointer(ffiType, true)
  }

  this.pointer = Bindings.prepCif(numArgs, this._rtypeptr, this._argtypesptr)
}
module.exports = CIF

CIF.prototype.getPointer = function () { return this.pointer }
