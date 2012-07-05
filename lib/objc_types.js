
/**
 * Logic for translating a given Objective-C "type" encoding into a node-ffi
 * type.
 *
 * ### References:
 *
 *   * [Apple "Type Encoding" Docs](http://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/ObjCRuntimeGuide/Articles/ocrtTypeEncodings.html)
 *   * [node-ffi "Type List"](https://github.com/rbranson/node-ffi/wiki/Node-FFI-Tutorial#wiki-type-list)
 */

/**
 * Module exports.
 */

exports.map = map
exports.mapArray = mapArray
exports.parse = parse

/**
 * Module dependencies.
 */

var assert = require('assert')

/**
 * A map of Objective-C type encodings to node-ffi types.
 *
 * @api private
 */

var typeEncodings = {
    'c': 'char'
  , 'i': 'int32'
  , 's': 'short'
  , 'l': 'long'
  , 'q': 'longlong'
  , 'C': 'uchar'
  , 'I': 'uint32'
  , 'S': 'ushort'
  , 'L': 'ulong'
  , 'Q': 'ulonglong'
  , 'f': 'float'
  , 'd': 'double'
  , 'B': 'int8'
  , 'v': 'void'
  , '*': 'string'   // String
  , '@': 'pointer'  // id
  , '#': 'pointer'  // Class
  , ':': 'pointer'  // SEL
  , '?': 'pointer'  // Unknown, used for function pointers
}
exports.typeEncodings = typeEncodings
var DELIMS = Object.keys(typeEncodings)

/**
 * A map of the additional type info for some ObjC methods.
 *
 * @api private
 */

var methodEncodings = {
    'r': 'const'
  , 'n': 'in'
  , 'N': 'inout'
  , 'o': 'out'
  , 'O': 'bycopy'
  , 'R': 'byref'
  , 'V': 'oneway'
}
exports.methodEncodings = methodEncodings

/**
 * Used to remove and method encodings present on the type.
 * NodObjC does not use them...
 */

var methodEncodingsTest = new RegExp('^(' + Object.keys(methodEncodings).join('|') + ')')

/**
 * Maps a single Obj-C 'type' into a valid node-ffi type.
 * This mapping logic is kind of a mess...
 */

function map (type) {
  console.log('type is ')
  console.log(type)
  if (!type) throw new Error('got falsey "type" to map ('+type+'). this should NOT happen!')
  if (type instanceof Array) type = type[0] // TODO hack
  if (type.type) type = type.type
  if (isStruct(type)) return getStruct(type)
  type = type.replace(methodEncodingsTest, '')
  // if the first letter is a ^ then it's a "pointer" type
  if (type[0] === '^') return 'pointer'
  // now we can try matching from the typeEncodings map
  var rtn = typeEncodings[type]
  if (rtn) return rtn
  // last shot... try the last char? this may be a bad idea...
  rtn = typeEncodings[type[type.length-1]]
  if (rtn) return rtn
  // couldn't find the type. throw a descriptive error as to why:
  if (type[0] == '[')
    throw new Error('Array types not yet supported: ' + type)
  if (type[0] == '(')
    throw new Error('Union types not yet supported: ' + type)
  if (type[0] == 'b')
    throw new Error('Bit field types not yet supported: ' + type)
  throw new Error('Could not convert type: ' + type)
}

/**
 * Accepts an Array of ObjC return type and argument types (i.e. the result of
 * parse() below), and returns a new Array with the values mapped to valid ffi
 * types.
 */

function mapArray (types) {
  return types.map(function (type) {
    return Array.isArray(type) ? exports.mapArray(type) : exports.map(type)
  })
}

/**
 * Parses a "types string" (i.e. `'v@:'`) and returns a "types Array", where the
 * return type is the first array value, and an Array of argument types is the
 * array second value.
 */

function parse (types) {
  if (typeof types === 'string') {
    var rtn = []
      , cur = []
      , len = types.length
      , depth = 0
    for (var i=0; i<len; i++) {
      var c = types[i]

      if (depth || !/(\d)/.test(c)) {
        cur.push(c)
      }

      if (c == '{' || c == '[' || c == '(') {
        depth++
      } else if (c == '}' || c == ']' || c == ')') {
        depth--
        if (!depth)
          add()
      } else if (~DELIMS.indexOf(c) && !depth) {
        add()
      }
    }
    function add () {
      rtn.push(cur.join(''))
      cur = []
      depth = 0
    }
    assert.equal(rtn[1], '@', '_self argument expected as first arg: ' + types)
    assert.equal(rtn[2], ':', 'SEL argument expected as second arg: ' + types)
    return [ rtn[0], rtn.slice(1) ]
  } else {
    var args = types.arg
    assert.equal(args[0], '@', '_self argument expected as first arg: ' + types)
    assert.equal(args[1], ':', 'SEL argument expected as second arg: ' + types)
    return [ types.retval, args ]
  }
}

exports.isStruct = isStruct
exports.getStruct = getStruct
exports.parseStructName = parseStructName
exports.parseStruct = parseStruct

/*!
 * Module dependencies.
 */

var Struct = require('ffi').Struct
  , test = /^\{.*?\}$/
  , structs = {}

/**
 * Tests if the given arg is a `Struct` constructor or a string type encoding
 * describing a struct (then true), otherwise false.
 *
 * @api private
 */

function isStruct (type) {
  return !!type.__isStructType__ || test.test(type)
}

/**
 * Returns the struct constructor function for the given struct name or type.
 *
 *     {CGPoint="x"d"y"d}
 *
 * @api private
 */

function getStruct (type) {
  // First check if a regular name was passed in
  var rtn = structs[type];
  if (rtn) return rtn;
  // If the struct type name has already been created, return that one
  var name = exports.parseStructName(type)
  console.error('name: %s', name)
  rtn = structs[name];
  if (rtn) {
    console.error('returning cached Struct')
    return rtn;
  }
  // Next parse the type structure
  var parsed = exports.parseStruct(type);
  // Otherwise we need to create a new Struct constructor
  var props = [];
  parsed.props.forEach(function (prop) {
    props.push([ map(prop[1]), prop[0] ])
  })
  return structs[parsed.name] = Struct(props)
}

/**
 * Extracts only the name of the given struct type encoding string.
 *
 * @api private
 */

function parseStructName (struct) {
  var s = struct.substring(1, struct.length-1)
    , equalIndex = s.indexOf('=')
  if (~equalIndex)
    s = s.substring(0, equalIndex)
  return s
}

/**
 * Parses a struct type string into an Object with a `name` String and
 * a `props` Array (entries are a type string, or another parsed struct object)
*
 * @api private
 */

function parseStruct (struct) {
  var s = struct.substring(1, struct.length-1)
    , equalIndex = s.indexOf('=')
    , rtn = {
        name: s.substring(0, equalIndex)
      , props: []
    }
  s = s.substring(equalIndex+1)
  var curProp = []
    , numBrackets = 0
    , entries = []
  for (var i=0; i < s.length; i++) {
    var cur = s[i]
    switch (cur) {
      case '"':
        if (numBrackets > 0)
          curProp.push(cur)
        else
          addProp()
        break;
      case '{':
      case '[':
      case '(':
        numBrackets++
        curProp.push(cur)
break;
      case '}':
      case ']':
      case ')':
        numBrackets--
        curProp.push(cur)
        break;
      default:
        curProp.push(cur)
        break;
    }
  }
  addProp()
  function addProp () {
    entries.push(curProp.join(''))
    curProp = []
    numBrackets = 0
  }
  for (var i=1; i < entries.length; i+=2) {
    rtn.props.push([entries[i], entries[i+1]])
  }
  return rtn
}

