
/**
 * Logic for importing a Framework into the node process.
 *
 * "Importing" a framework is a multi-step process:
 *
 *   1. `resolve()` the absolute path of the given framework name.
 *   1. Load the framework's binary `dylib` file into the process.
 *   1. Usually locate the `BridgeSupport` files for the framework and process.
 *   1. Define any new class getters for newly loaded Objective-C classes.
 */

/*!
 * Module exports.
 */

exports.import = importFramework

/*!
 * The Framework PATH. Equivalent to the -F switch with gcc.
 */

PATH = [
    '/System/Library/Frameworks'
  , '/System/Library/PrivateFrameworks'
]

/*!
 * Module dependencies.
 */

var fs = require('fs')
  , ffi = require('ffi')
  , path = require('path')
  , core = require('objc_core')
  , _class = require('objc_class')
  , basename = path.basename
  , SUFFIX = '.framework'
  , _global = require('objc_global')
  , cache = {}

/**
 * Accepts a single framework name and imports it into the current node process.
 * `framework` may be a relative (singular) framework name, or a path (relative or
 * absolute) to a Framework directory.
 *
 *     $.NSObject   // undefined
 *
 *     $.import('Foundation')
 *
 *     $.NSObject   // [Class: NSObject]
 *
 * @param {String} framework The framework name or path to load.
 */

function importFramework (framework, skip) {
  framework = resolve(framework)

  var shortName = basename(framework, SUFFIX)

  // Check if the framework has already been loaded
  var fw = cache[shortName]
  if (fw) {
    return
  }

  // Load the main framework binary file
  var frameworkPath = path.join(framework, shortName)
    , lib = new ffi.DynamicLibrary(frameworkPath)

  fw = {
      lib: lib
    , name: shortName
    , basePath: framework
    , binaryPath: frameworkPath
  }

  // cache before loading bridgesupport files
  cache[shortName] = fw

  // Parse the BridgeSupport file and inline dylib, for the C functions, enums,
  // and other symbols not introspectable at runtime.
  bridgesupport(fw)

  // Iterate through the loaded classes list and define "setup getters" for them.
  if (!skip) {
    var classes = core.getClassList()
    classes.forEach(function (c) {
      if (c in _global) return
      _global.__defineGetter__(c, function () {
        var clazz = _class.getClass(c)
        delete _global[c]
        return _global[c] = clazz
      })
    })
  }

}

/**
 * Accepts a single framework name and resolves it into an absolute path
 * to the base directory of the framework.
 *
 * In most cases, you will not need to use this function in your code.
 *
 *     $.resolve('Foundation')
 *     //  '/System/Library/Frameworks/Foundation.framework'
 *
 * @param {String} framework The framework name or path to resolve.
 * @return {String} The resolved framework path.
 */

function resolve (framework) {
  // strip off a trailing slash if present
  if (framework[framework.length-1] == '/') {
    framework = framework.slice(0, framework.length-1)
  }
  // already absolute, return as-is
  if (~framework.indexOf('/')) return framework
  var i = 0
    , l = PATH.length
    , rtn = null
  for (; i<l; i++) {
    rtn = path.join(PATH[i], framework + SUFFIX)
    if (fs.existsSync(rtn)) {
      return rtn
    }
  }
  throw new Error('Could not resolve framework: ' + framework)
}

/**
 * This module takes care of loading the BridgeSupport XML files for a given
 * framework, and parsing the data into the given framework object.
 *
 * ### References:
 *
 *  * [`man 5 BridgeSupport`](http://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man5/BridgeSupport.5.html)
 *  * [BridgeSupport MacOS Forge website](http://bridgesupport.macosforge.org)
 */

/*!
 * Module exports.
 */

exports.classes = {}
exports.informal_protocols = {}

/*!
 * Module dependencies.
 */

var xml2js = require('xml2js')
  , assert = require('assert')
  , method = require('objc_method')
  , types = require('objc_types')
  , DY_SUFFIX = '.dylib'
  , BS_SUFFIX = '.bridgesupport'


/*!
 * Architecture-specific functions that return the Obj-C type or value from one
 * of these BridgeSupport XML nodes.
 */

var getType
  , getValue
if (process.arch == 'x64') {
  // 64-bit specific functions
  getType = function (node) {
    var v = node.type64 || node.type
    return v
  }
  getValue = function (node) {
    var v = node.value64 || node.value
    return v
  }
} else {
  // 32-bit / ARM specific functions
  getType = function (node) {
    return node.type
  }
  getValue = function (node) {
    return node.value
  }
}

/**
 * Attempts to retrieve the BridgeSupport files for the given framework.
 * It synchronously reads the contents of the bridgesupport files and parses
 * them in order to add the symbols that the Obj-C runtime functions cannot
 * determine.
 */

function bridgesupport (fw) {

  var bridgeSupportDir = path.join(fw.basePath, 'Resources', 'BridgeSupport')
    , bridgeSupportXML = path.join(bridgeSupportDir, fw.name + BS_SUFFIX)
    , bridgeSupportDylib = path.join(bridgeSupportDir, fw.name + DY_SUFFIX)

  // If there's no BridgeSupport file, then bail...
  if (!fs.existsSync(bridgeSupportXML)) {
    return
 }

  // Load the "inline" dylib if it exists
  if (fs.existsSync(bridgeSupportDylib)) {
    fw.inline = new ffi.DynamicLibrary(bridgeSupportDylib)
  }

  var contents = fs.readFileSync(bridgeSupportXML, 'utf8')
  var parser = new xml2js.Parser({mergeAttrs: true, explicitRoot: false})
  parser.parseString(contents, function (error, result) {
    //console.log(require('util').inspect(result, false, 10));
    for (var name in result) {
      switch (name) {
        case 'text':
          break;
        case 'depends_on':
          result['depends_on'].forEach(function(node) {
            importFramework(node.path, true)
          })
          break;
        case 'class':
          result['class'].forEach(function(node) {
            exports.classes[node.name] = node
          })
          break;
        case 'string_constant':
          result['string_constant'].forEach(function(node) {
            _global[node.name] = getValue(node)
          })
          break;
        case 'enum':
          result['enum'].forEach(function(node) {
            _global[node.name] = Number(getValue(node))
          })
          break;
        case 'struct':
          // TODO: Remove the try/catch when all the Struct formats are supported
          //       Still need Array and Union support.
          try {
            result['struct'].forEach(function(node) {
              _global[node.name] = types.getStruct(getType(node))
            })
          } catch (e) {
          }
          break;
        case 'field':
          break;
        case 'cftype':
          break;
        case 'constant':
          result['constant'].forEach(function(node) {
            defineConstant(node, fw)
          })
          break;
        case 'function':
          result['function'].forEach(function(node) {
            defineFunction(node, fw)
          })
          break;
        case 'opaque':
          break;
        case 'informal_protocol':
          result['informal_protocol'].forEach(function(node) {
            exports.informal_protocols[node.name] = node
          })
          break;
        case 'function_alias':
          break;
        case 'version':
          break
        default:
          throw new Error('unkown tag: '+ node.name)
          break;
      }
    }
  })
}


/**
 * Sets up a <constant> tag onto the global exports.
 * These start out as simple JS getters, so that the underlying
 * symbol pointer can be lazy-loaded on-demand.
 */

function defineConstant (node, fw) {
  var name = node.name
    , type = getType(node)
  _global.__defineGetter__(name, function () {
    var ptr = fw.lib.get(name) // TODO: Cache the pointer after the 1st call
    ptr._type = '^' + type
    var val = ptr.deref()
    return val
  })
}


/**
 * Sets up a <function> tag onto the global exports.
 * These start out as simple JS getters, so that the underlying
 * function pointer can be lazy-loaded on-demand.
 */

function defineFunction (node, fw) {
  var name = node.name
  _global.__defineGetter__(name, function () {
    //console.error(require('util').inspect(a, true, 10))
    // TODO: Handle 'variadic' arg functions (NSLog), will require
    //       a "function generator" to get a Function from the passed
    //       in args (and guess at the types that were passed in...)
    if (node.inline  === 'true') {
      assert.ok(fw.inline, name+', '+fw.name+': declared inline but could not find inline dylib!')
    }
    var ptr = (node.inline ? fw.inline : fw.lib).get(name)
      , unwrapper = method.createUnwrapperFunction(ptr, node)
    unwrapper.info = node
    delete _global[name]
    return _global[name] = unwrapper
  })
}

