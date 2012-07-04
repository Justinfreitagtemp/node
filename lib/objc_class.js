
/**
 * The 'id' function is essentially the "base class" for all Objective-C
 * objects that get passed around JS-land.
 */

/*!
 * Module exports.
 */

exports.wrap = wrap
exports.wrapClass = wrapClass
exports.wrapException = wrapException

/*!
 * Module dependencies.
 */


var proto = exports.proto = Object.create(Function.prototype)
  , ffi = require('ffi')
  , core  = require('objc_core')
  , types = require('objc_types')
  , SEL   = require('objc_sel')
  , assert = require('assert')
  , method = require('objc_method')
  , ivar = require('objc_ivar')
  , IMP = require('objc_imp')
  , SEL = require('objc_sel')
  , classCache = {}
  , _global = {}


/*!
 * An arbitrary "key" pointer we use for storing the JS-wrap instance reference
 * into the ObjC object's internal weak map via `objc_getAssociatedObject()`.
 */

var KEY = new ffi.Pointer(1)

/**
 * Wraps up a pointer that is expected to be a compatible Objective-C
 * object that can recieve messages. This function returns a cached version of the
 * wrapped function after the first time it is invoked on a given Pointer, using
 * Objective-C's internal association map for objects.
 *
 * @api private
 */

function wrap (pointer) {
  var rtn = null
    , p = core.objc_getAssociatedObject(pointer, KEY)
  if (p.isNull()) {
    rtn = createFunctionWrapper(pointer)
    // Store the wrapped instance internally
    var ref = new ffi.Pointer(ffi.TYPE_SIZE_MAP.Object)
    // don't call free() automatically when ref gets GC'd
    // TODO: we're gonna have to free this pointer someday!
    // XXX: use node-weak to get a callback when the wrapper is GC'd
    ref.free = false
    ref.putObject(rtn)
    core.objc_setAssociatedObject(pointer, KEY, ref, 0)
  } else {
    rtn = p.getObject()
  }
  //assert.equal(rtn.pointer.address, pointer.address)
  return rtn
}

/*!
 * The parseArgs() function is used by 'id()' and 'id.super()'.
 * You pass in an Array as the second parameter as a sort of "output variable"
 * It returns the selector that was requested.
 */

function parseArgs (argv, args) {
  var argc = argv.length
    , sel
  if (argc === 1) {
    var arg = argv[0]
    if (typeof arg === 'string') {
      // selector with no arguments
      sel = arg
    } else {
      // legacy API: an Object was passed in
      sel = []
      Object.keys(arg).forEach(function (s) {
        sel.push(s)
        args.push(arg[s])
      })
      sel.push('')
      sel = sel.join(':')
    }
  } else {
    // varargs API
    sel = []
    for (var i=0; i<argc; i+=2) {
      sel.push(argv[i])
      args.push(argv[i+1])
    }
    sel.push('')
    sel = sel.join(':')
  }
  return sel
}

/*!
 * Internal function that essentially "creates" a new Function instance wrapping
 * the given pointer. The function's implementation is the "id()" function below,
 *
 * XXX: Maybe use `Function.create()` from my `create` module here (benchmark)???
 *
 * @api private
 */

function createFunctionWrapper (pointer) {

  // This 'id' function is syntax sugar around the msgSend function attached to
  // it. 'msgSend' is expecting the selector first, an Array of args second, so
  // this function just massages it into place and returns the result.
  function id () {
    var args = []
      , sel = parseArgs(arguments, args)
    return msgSend(sel, args)
  }

  // Set the "type" on the pointer. This is used by 'ref()' and 'unref()'.
  pointer._type = '@'
  // Save a reference to the pointer for use by the prototype functions
  id.pointer = pointer
  // Morph into a MUTANT FUNCTION FREAK!!1!
  id.__proto__ = proto
  return id
}


/**
 * A very important function that *does the message sending* between
 * Objective-C objects. When you do `array('addObject', anObject)`, this
 * `msgSend` function is the one that finally gets called to do the dirty work.
 *
 * This function accepts a String selector as the first argument, and an Array
 * of (wrapped) values that get passed to the the message. This function takes
 * care of unwrapping the passed in arguments and wrapping up the result value,
 * if necessary.
 */

proto.msgSend = function msgSend (sel, args) {
  var types = this._getTypes(sel, args)
    , argTypes = types[1]
    , msgSendFunc = core.get_objc_msgSend(types)
    , unwrappedArgs = core.unwrapValues([this, sel].concat(args), argTypes)
    , rtn

  try {
    rtn = msgSendFunc.apply(null, unwrappedArgs)
  } catch (e) {
    if (!e.hasOwnProperty('stack')) {
      e = wrapException(e)
    }
    throw e
  }
  // Process the return value into a wrapped value if needed
  return core.wrapValue(rtn, types[0])
}

/**
 * Like regular message sending, but invokes the method implementation on the
 * object's "superclass" instead. This is the equivalent of what happens when the
 * Objective-C compiler encounters the `super` keyword:
 *
 * ``` objectivec
 * self = [super init];
 * ```
 *
 * To do the equivalent using NodObjC you call `super()`, as shown here:
 *
 * ``` js
 * self = self.super('init')
 * ```
 */

proto.super = function super_ () {
  var args = []
    , sel = parseArgs(arguments, args)
  return this.msgSendSuper(sel, args)
}

/*!
 * Struct used by msgSendSuper().
 */

var objc_super = ffi.Struct([
    ['pointer', 'receiver']
  , ['pointer', 'class']
])

/**
 * Calls `objc_msgSendSuper()` on the underlying Objective-C object.
 */

proto.msgSendSuper = function msgSendSuper (sel, args) {

  var os = new objc_super
  os.receiver = this.pointer
  os.class = this.getClass().getSuperclass().pointer

  var types = this._getTypes(sel, args)
    , argTypes = types[1]
    , msgSendSuperFunc = core.get_objc_msgSendSuper(types)
    , unwrappedArgs = core.unwrapValues([os, sel].concat(args), argTypes)
    , rtn

  try {
    rtn = msgSendSuperFunc.apply(null, unwrappedArgs)
  } catch (e) {
    if (!e.hasOwnProperty('stack')) {
      e = wrapException(e)
    }
    throw e
  }
  // Process the return value into a wrapped value if needed
  return core.wrapValue(rtn, types[0])
}

/**
 * Accepts a SEL and queries the current object for the return type and
 * argument types for the given selector. If current object does not implment
 * that selector, then check the superclass, and repeat recursively until
 * a subclass that responds to the selector is found, or until the base class
 * is found.
 *
 * TODO: Just merge this logic with `msgSend()`? It's not used anywhere else
 *
 * @api private
 */

proto._getTypes = function getTypes (sel, args) {
  var c = this.getClass()
    , t = c._getTypesClass(sel, this.isClass)
  if (!t) {
    // Unknown selector being send to object. This *may* still be valid, we
    // assume all args are type 'id' and return is 'id'.
    t = [ '@', [ '@', ':', ].concat(args.map(function () { return '@' })) ]
  }
  return t
}

/**
 * Retrieves the wrapped Class instance for this object.
 */

proto.getClass = function getClass () {
  return wrapClass(core.object_getClass(this.pointer))
}

/**
 * Calls 'object_getClassName()' on this object.
 */

proto.getClassName = function getClassName () {
  return core.object_getClassName(this.pointer)
}

/**
 * Dynamically changes the object's Class.
 */

proto.setClass = function setClass (newClass) {
  return wrapClass(core.object_setClass(this.pointer, newClass.pointer))
}

/**
 * Walks up the inheritance chain and returns an Array of Strings of
 * superclasses.
 */

proto.ancestors = function ancestors () {
  var rtn = []
    , c = this.getClass()
  while (c) {
    rtn.push(c.getName())
    c = c.getSuperclass()
  }
  return rtn
}

/**
 * Getter/setter function for instance variables (ivars) of the object,
 * If just a name is passed in, then this function gets the ivar current value.
 * If a name and a new value are passed in, then this function sets the ivar.
 */

proto.ivar = function ivar (name, value) {
  // TODO: Add support for passing in a wrapped Ivar instance as the `name`
  if (arguments.length > 1) {
    // setter
    var ivar = this.isClass
             ? this.getClassVariable(name)
             : this.getClass().getInstanceVariable(name)
      , unwrapped = core.unwrapValue(value, ivar.getTypeEncoding())
    return core.object_setIvar(this.pointer, ivar.pointer, unwrapped)
  } else {
    // getter
    var ptr = new ffi.Pointer(ffi.TYPE_SIZE_MAP.pointer)
      , ivar = core.object_getInstanceVariable(this.pointer, name, ptr)
    return core.wrapValue(ptr.getPointer(), core.ivar_getTypeEncoding(ivar))
  }
}

/**
 * Returns an Array of Strings of the names of the ivars that the current object
 * contains. This function can iterate through the object's superclasses
 * recursively, if you specify a `maxDepth` argument.
 */

proto.ivars = function ivars (maxDepth, sort) {
  var rtn = []
    , c = this.getClass()
    , md = maxDepth || 1
    , depth = 0
  while (c && depth++ < md) {
    var is = c.getInstanceVariables()
      , i = is.length
    while (i--) {
      if (!~rtn.indexOf(is[i])) rtn.push(is[i])
    }
    c = c.getSuperclass()
  }
  return sort === false ? rtn : rtn.sort()
}

/**
 * Returns an Array of Strings of the names of methods that the current object
 * will respond to. This function can iterate through the object's superclasses
 * recursively, if you specify a `maxDepth` number argument.
 */

proto.methods = function methods (maxDepth, sort) {
  var rtn = []
    , c = this.getClass()
    , md = maxDepth || 1
    , depth = 0
  while (c && depth++ < md) {
    var ms = c.getInstanceMethods()
      , i = ms.length
    while (i--) {
      if (!~rtn.indexOf(ms[i])) rtn.push(ms[i])
    }
    c = c.getSuperclass()
  }
  return sort === false ? rtn : rtn.sort()
}

/**
 * Returns a **node-ffi** pointer pointing to this object. This is a convenience
 * function for methods that take pointers to objects (i.e. `NSError**`).
 *
 * @return {Pointer} A pointer to this object.
 */

proto.ref = function ref () {
  var ptr = this.pointer.ref()
  return ptr
}

/**
 * The overidden `toString()` function proxies up to the real Objective-C object's
 * `description` method. In Objective-C, this is equivalent to:
 *
 * ``` objectivec
 * [[id description] UTF8String]
 * ```

proto.toString = function toString () {
  return this('description')('UTF8String')
}

 */
/*!
 * Custom inspect() function for `util.inspect()`.

proto.inspect = function inspect () {
  return this.toString()
}

 */
/**
 * The `Class` class is a subclass of `id`. Instances of `Class` wrap an
 * Objective C *"Class"* instance.
 *
 * You can retrieve `Class` instances by getting a reference to a global class
 * (i.e. `$.NSObject`), or by other methods/functions that return `Class`
 * instances normally (i.e. `$.NSClassFromString($('NSObject'))`).
 */

/*!
 * Module exports.
 */

exports.getClass = getClass

/**
 * Gets a wrapped Class instance based off the given name.
 * Also takes care of returning a cached version when available.
 *
 * @param {String} className The class name to load.
 * @return {Class} A `Class` instance wrapping the desired Objective C *"Class"*.
 * @api private
 */

function getClass (className) {
  var rtn = classCache[className]
  if (rtn) {
  } else {
    var pointer = core.objc_getClass(className)
    rtn = wrapClass(pointer, className)
  }
  return rtn
}

/**
 * Wraps the given *pointer*, which should be an Objective-C Class, and returns
 * a `Class` instance.
 *
 * @param {Pointer} pointer The Class pointer to wrap.
 * @param {String} className An optional class name to cache the Class wrapper with.
 * @return {Class} A `Class` instance wrapping the given *pointer*.
 * @api private
 */

function wrapClass (pointer, className) {
  var w = wrap(pointer)
  w.__proto__ = proto
  pointer._type = '#'
  // optionally cache when a class name is given
  if (className) {
    classCache[className] = w
  }
  return w
}

// Flag used by id#msgSend()
proto.isClass = true

/**
 * Creates a subclass of this class with the given name and optionally a
 * number of extra bytes that will be allocated with each instance. The
 * returned `Class` instance should have `addMethod()` and `addIvar()` called on
 * it as needed before use, and then `register()` when you're ready to use it.
 */

proto.extend = function extend (className, extraBytes) {
  var c = core.objc_allocateClassPair(this.pointer, className, extraBytes || 0)
  if (c.isNull()) {
    throw new Error('New Class could not be allocated: ' + className)
  }
  return wrapClass(c, className)
}

/**
 * Calls objc_registerClassPair() on the class pointer.
 * This must be called on the class *after* all 'addMethod()' and 'addIvar()'
 * calls are made, and *before* the newly created class is used for real.
 */

proto.register = function register () {
  core.objc_registerClassPair(this.pointer)
  _global[this.getName()] = this
  return this
}

/**
 * Adds a new Method to the Class. Instances of the class (even already existing
 * ones) will have the ability to invoke the method. This may be called at any
 * time on any class.
 */

proto.addMethod = function addMethod (selector, type, func) {
  var parsed = types.parse(type)
    , selRef = SEL.toSEL(selector)
    , funcPtr = IMP.createWrapperPointer(func, parsed)
  // flatten the type
  var typeStr = parsed[0] + parsed[1].join('')
  if (!core.class_addMethod(this.pointer, selRef, funcPtr, typeStr)) {
    throw new Error('method "' + selector + '" was NOT sucessfully added to Class: ' + this.getName())
  }
  return this
}

/**
 * Adds an Ivar to the Class. Instances of the class will contain the specified
 * instance variable. This MUST be called after .extend() but BEFORE .register()
 */

proto.addIvar = function addIvar (name, type, size, alignment) {
  if (!size) {
    // Lookup the size of the type when needed
    var ffiType = types.map(type)
    size = ffi.TYPE_SIZE_MAP[ffiType]
  }
  if (!alignment) {
    // Also set the alignment when needed. This formula is from Apple's docs:
    //   For variables of any pointer type, pass log2(sizeof(pointer_type)).
    alignment = Math.log(size) / Math.log(2)
  }
  if (!core.class_addIvar(this.pointer, name, size, alignment, type)) {
    throw new Error('ivar "' + name + '" was NOT sucessfully added to Class: ' + this.getName())
  }
  return this
}

/**
 * Adds a `Protocol` to the list of protocols that this class "conforms to"
 * (a.k.a "implements"). Usually, an implementation object is passed in that
 * defines the Protocol's defined methods onto the class.
 */

proto.addProtocol = function addProtocol (protocolName, impl) {
  var informal = require('objc_import').informal_protocols[protocolName]
    , formal = core.objc_getProtocol(protocolName)

  console.error(core.copyMethodDescriptionList(formal, 1, 1))
  console.error(core.copyMethodDescriptionList(formal, 0, 0))
  console.error(core.copyMethodDescriptionList(formal, 1, 0))
  console.error(core.copyMethodDescriptionList(formal, 0, 1))
}

proto._getSuperclassPointer = function getSuperclassPointer () {
  return core.class_getSuperclass(this.pointer)
}

proto.getName = function getName () {
  return core.class_getName(this.pointer)
}

proto.isMetaClass = function isMetaClass () {
  return !!core.class_isMetaClass(this.pointer)
}

proto.getInstanceSize = function getInstanceSize () {
  return core.class_getInstanceSize(this.pointer)
}

proto.getIvarLayout = function getIvarLayout () {
  return core.class_getIvarLayout(this.pointer)
}

/**
 * Get's a Class instance's superclass. If the current class is a base class,
 * then this will return null.
 */

proto.getSuperclass = function getSuperclass () {
  var superclassPointer = this._getSuperclassPointer()
  if (superclassPointer.isNull()) return null
  return wrapClass(superclassPointer)
}

proto.setSuperclass = function setSuperclass (superclass) {
  return wrapClass(this._setSuperclassPointer(superclass.pointer))
}

proto._setSuperclassPointer = function setSuperclassPointer (superclassPointer) {
  return core.class_setSuperclass(this.pointer, superclassPointer)
}

proto.getInstanceVariable = function getInstanceVariable (name) {
  return ivar.wrap(this._getInstanceVariable(name))
}

proto._getInstanceVariable = function _getInstanceVariable (name) {
  return core.class_getInstanceVariable(this.pointer, name)
}

proto.getClassVariable = function getClassVariable (name) {
  return ivar.wrap(this._getClassVariable(name))
}

proto._getClassVariable = function _getClassVariable (name) {
  return core.class_getClassVariable(this.pointer, name)
}

proto.getInstanceMethod = function getInstanceMethod (sel) {
  return method.wrap(this._getInstanceMethod(SEL.toSEL(sel)))
}

proto._getInstanceMethod = function _getInstanceMethod (selPtr) {
  return core.class_getInstanceMethod(this.pointer, selPtr)
}

proto.getClassMethod = function getClassMethod (sel) {
  return method.wrap(this._getClassMethod(SEL.toSEL(sel)))
}

proto._getClassMethod = function _getClassMethod (selPtr) {
  return core.class_getClassMethod(this.pointer, selPtr)
}

proto._getTypesClass = function getTypesClass (sel, isClass) {
  //console.error('_getTypesClass: %s, isClass: %d', sel, isClass)
  var method = this['get'+(isClass ? 'Class' : 'Instance')+'Method'](sel)
  return method ? method.getTypes() : null
}

proto.getVersion = function getVersion () {
  return core.class_getVersion(this.pointer)
}

proto.setVersion = function setVersion (v) {
  return core.class_setVersion(this.pointer, v)
}

/**
 * Returns an Array of the class variables this Class has. Superclass variables
 * are not included.
 */

proto.getClassVariables = function getClassVariables () {
  return core.copyIvarList(this._getClassPointer())
}

/**
 * Returns an Array of the instance variables this Class has. Superclass
 * variables are not included.
 */

proto.getInstanceVariables = function getInstanceVariables () {
  return core.copyIvarList(this.pointer)
}

/**
 * Returns an Array of all the class methods this Class responds to.
 * This function returns the raw, unsorted result of copyMethodList().
 */

proto.getClassMethods = function getClassMethods () {
  // getClassPointer() on a Class actually gets a pointer to the metaclass
  return core.copyMethodList(this._getClassPointer())
}

/**
 * Returns an Array of all the instance methods an instance of this Class will
 * respond to.
 * This function returns the raw, unsorted result of copyMethodList().
 */

proto.getInstanceMethods = function getInstanceMethods () {
  return core.copyMethodList(this.pointer)
}

/**
 * Allocates a new pointer to this type. The pointer points to `nil` initially.
 * This is meant for creating a pointer to hold an NSError*, and pass a ref()
 * to it into a method that accepts an 'error' double pointer.
 * XXX: Tentative API - name will probably change
 */

proto.createPointer = function createPointer () {
  var ptr = core.Pointer.alloc('pointer', core.Pointer.NULL)
  ptr._type = '@'
  return ptr
}

/*!
 * toString() override.
  */
proto.toString = function toString () {
  return '[Class: ' + this.getName() + ']'
}

proto.inspect = function inspect () {
  // yellow
  return '\033[33m' + this.toString() + '\033[39m'
}

/**
 * Wraps a `Pointer` that should be an Objective-C `NSException` instance.
 *
 * @api private
 */

var Exception = require('vm').runInNewContext('Error')
  , exceptionProto = exports.exceptionProto = Exception.prototype

exceptionProto.toString = function toString () {
  return this('name') + ': ' + this('reason')
}

function wrapException (pointer) {
  var w = Class.wrap(pointer)
  w.__proto__ = exceptionProto
  // `name` is non-configurable on Functions, so don't bother
  w.message = String(w('reason'))
  Error.captureStackTrace(w, wrap)
  return w
}

