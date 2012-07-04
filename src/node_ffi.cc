#include "node_ffi.h"

#include "node_buffer.h"

namespace ffi {

///////////////

void FFI::InitializeStaticFunctions(Handle<Object> target) {
  Local<Object> o = Object::New();

  // atoi and abs here for testing purposes
  o->Set(String::NewSymbol("atoi"), Pointer::WrapPointer((unsigned char *)atoi));
  // Windows has multiple `abs` signatures, so we need to manually disambiguate
  int (*absPtr)(int)(abs);
  o->Set(String::NewSymbol("abs"), Pointer::WrapPointer((unsigned char *)absPtr));

  // dl functions used by the DynamicLibrary JS class
  o->Set(String::NewSymbol("dlopen"), Pointer::WrapPointer((unsigned char *)dlopen));
  o->Set(String::NewSymbol("dlclose"), Pointer::WrapPointer((unsigned char *)dlclose));
  o->Set(String::NewSymbol("dlsym"), Pointer::WrapPointer((unsigned char *)dlsym));
  o->Set(String::NewSymbol("dlerror"), Pointer::WrapPointer((unsigned char *)dlerror));

  target->Set(String::NewSymbol("StaticFunctions"), o);
}

///////////////

void FFI::InitializeBindings(Handle<Object> target) {

  target->Set(String::NewSymbol("free"), FunctionTemplate::New(Free)->GetFunction());
  target->Set(String::NewSymbol("prepCif"), FunctionTemplate::New(FFIPrepCif)->GetFunction());
  target->Set(String::NewSymbol("strtoul"), FunctionTemplate::New(Strtoul)->GetFunction());
  target->Set(String::NewSymbol("POINTER_SIZE"), Integer::New(sizeof(unsigned char *)));
  target->Set(String::NewSymbol("FFI_TYPE_SIZE"), Integer::New(sizeof(ffi_type)));

  bool hasObjc = false;
#if __OBJC__ || __OBJC2__
  hasObjc = true;
#endif
  target->Set(String::NewSymbol("HAS_OBJC"), Boolean::New(hasObjc), static_cast<PropertyAttribute>(ReadOnly|DontDelete));

  Local<Object> smap = Object::New();
  smap->Set(String::NewSymbol("byte"),      Integer::New(sizeof(unsigned char)));
  smap->Set(String::NewSymbol("int8"),      Integer::New(sizeof(int8_t)));
  smap->Set(String::NewSymbol("uint8"),     Integer::New(sizeof(uint8_t)));
  smap->Set(String::NewSymbol("int16"),     Integer::New(sizeof(int16_t)));
  smap->Set(String::NewSymbol("uint16"),    Integer::New(sizeof(uint16_t)));
  smap->Set(String::NewSymbol("int32"),     Integer::New(sizeof(int32_t)));
  smap->Set(String::NewSymbol("uint32"),    Integer::New(sizeof(uint32_t)));
  smap->Set(String::NewSymbol("int64"),     Integer::New(sizeof(int64_t)));
  smap->Set(String::NewSymbol("uint64"),    Integer::New(sizeof(uint64_t)));
  smap->Set(String::NewSymbol("char"),      Integer::New(sizeof(char)));
  smap->Set(String::NewSymbol("uchar"),     Integer::New(sizeof(unsigned char)));
  smap->Set(String::NewSymbol("short"),     Integer::New(sizeof(short)));
  smap->Set(String::NewSymbol("ushort"),    Integer::New(sizeof(unsigned short)));
  smap->Set(String::NewSymbol("int"),       Integer::New(sizeof(int)));
  smap->Set(String::NewSymbol("uint"),      Integer::New(sizeof(unsigned int)));
  smap->Set(String::NewSymbol("long"),      Integer::New(sizeof(long)));
  smap->Set(String::NewSymbol("ulong"),     Integer::New(sizeof(unsigned long)));
  smap->Set(String::NewSymbol("longlong"),  Integer::New(sizeof(long long)));
  smap->Set(String::NewSymbol("ulonglong"), Integer::New(sizeof(unsigned long long)));
  smap->Set(String::NewSymbol("float"),     Integer::New(sizeof(float)));
  smap->Set(String::NewSymbol("double"),    Integer::New(sizeof(double)));
  smap->Set(String::NewSymbol("pointer"),   Integer::New(sizeof(unsigned char *)));
  smap->Set(String::NewSymbol("string"),    Integer::New(sizeof(char *)));
  smap->Set(String::NewSymbol("size_t"),    Integer::New(sizeof(size_t)));
  // Size of a Persistent handle to a JS object
  smap->Set(String::NewSymbol("Object"),    Integer::New(sizeof(Persistent<Object>)));

  Local<Object> ftmap = Object::New();
  ftmap->Set(String::NewSymbol("void"),     Pointer::WrapPointer((unsigned char *)&ffi_type_void));
  ftmap->Set(String::NewSymbol("byte"),     Pointer::WrapPointer((unsigned char *)&ffi_type_uint8));
  ftmap->Set(String::NewSymbol("int8"),     Pointer::WrapPointer((unsigned char *)&ffi_type_sint8));
  ftmap->Set(String::NewSymbol("uint8"),    Pointer::WrapPointer((unsigned char *)&ffi_type_uint8));
  ftmap->Set(String::NewSymbol("uint16"),   Pointer::WrapPointer((unsigned char *)&ffi_type_uint16));
  ftmap->Set(String::NewSymbol("int16"),    Pointer::WrapPointer((unsigned char *)&ffi_type_sint16));
  ftmap->Set(String::NewSymbol("uint32"),   Pointer::WrapPointer((unsigned char *)&ffi_type_uint32));
  ftmap->Set(String::NewSymbol("int32"),    Pointer::WrapPointer((unsigned char *)&ffi_type_sint32));
  ftmap->Set(String::NewSymbol("uint64"),   Pointer::WrapPointer((unsigned char *)&ffi_type_uint64));
  ftmap->Set(String::NewSymbol("int64"),    Pointer::WrapPointer((unsigned char *)&ffi_type_sint64));
  ftmap->Set(String::NewSymbol("uchar"),    Pointer::WrapPointer((unsigned char *)&ffi_type_uchar));
  ftmap->Set(String::NewSymbol("char"),     Pointer::WrapPointer((unsigned char *)&ffi_type_schar));
  ftmap->Set(String::NewSymbol("ushort"),   Pointer::WrapPointer((unsigned char *)&ffi_type_ushort));
  ftmap->Set(String::NewSymbol("short"),    Pointer::WrapPointer((unsigned char *)&ffi_type_sshort));
  ftmap->Set(String::NewSymbol("uint"),     Pointer::WrapPointer((unsigned char *)&ffi_type_uint));
  ftmap->Set(String::NewSymbol("int"),      Pointer::WrapPointer((unsigned char *)&ffi_type_sint));
  ftmap->Set(String::NewSymbol("float"),    Pointer::WrapPointer((unsigned char *)&ffi_type_float));
  ftmap->Set(String::NewSymbol("double"),   Pointer::WrapPointer((unsigned char *)&ffi_type_double));
  ftmap->Set(String::NewSymbol("pointer"),  Pointer::WrapPointer((unsigned char *)&ffi_type_pointer));
  ftmap->Set(String::NewSymbol("string"),   Pointer::WrapPointer((unsigned char *)&ffi_type_pointer));
  ftmap->Set(String::NewSymbol("size_t"),   Pointer::WrapPointer((unsigned char *)&ffi_type_pointer));

  // libffi is weird when it comes to long data types (defaults to 64-bit), so we emulate here, since
  // some platforms have 32-bit longs and some platforms have 64-bit longs.
  if (sizeof(long) == 4) {
    ftmap->Set(String::NewSymbol("ulong"),    Pointer::WrapPointer((unsigned char *)&ffi_type_uint32));
    ftmap->Set(String::NewSymbol("long"),     Pointer::WrapPointer((unsigned char *)&ffi_type_sint32));
  } else if (sizeof(long) == 8) {
    ftmap->Set(String::NewSymbol("ulong"),    Pointer::WrapPointer((unsigned char *)&ffi_type_uint64));
    ftmap->Set(String::NewSymbol("long"),     Pointer::WrapPointer((unsigned char *)&ffi_type_sint64));
  }

  // Let libffi handle "long long"
  ftmap->Set(String::NewSymbol("ulonglong"),Pointer::WrapPointer((unsigned char *)&ffi_type_ulong));
  ftmap->Set(String::NewSymbol("longlong"), Pointer::WrapPointer((unsigned char *)&ffi_type_slong));

  target->Set(String::NewSymbol("FFI_TYPES"), ftmap);
  target->Set(String::NewSymbol("TYPE_SIZE_MAP"), smap);
}

Handle<Value> FFI::Free(const Arguments &args) {
  HandleScope scope;

  Pointer *p = ObjectWrap::Unwrap<Pointer>(args[0]->ToObject());
  free(p->GetPointer());
  return Undefined();
}

/**
 * Hard-coded `stftoul` binding, for the benchmarks.
 */

Handle<Value> FFI::Strtoul(const Arguments &args) {
  HandleScope scope;

  Pointer *middle = ObjectWrap::Unwrap<Pointer>(args[1]->ToObject());
  char buf[128];
  args[0]->ToString()->WriteUtf8(buf);

  unsigned long val = strtoul(buf, (char **)middle->GetPointer(), args[2]->Int32Value());

  return scope.Close(Integer::NewFromUnsigned(val));
}

/**
 * Function that creates and returns an `ffi_cif` pointer from the given return
 * value type and argument types.
 */

Handle<Value> FFI::FFIPrepCif(const Arguments& args) {
  HandleScope scope;

  unsigned int nargs;
  Pointer *rtype, *atypes, *cif;
  ffi_status status;

  if (args.Length() != 3) {
    return THROW_ERROR_EXCEPTION("prepCif() requires 3 arguments!");
  }

  nargs = args[0]->Uint32Value();
  rtype = ObjectWrap::Unwrap<Pointer>(args[1]->ToObject());
  atypes = ObjectWrap::Unwrap<Pointer>(args[2]->ToObject());

  cif = new Pointer(NULL);
  cif->Alloc(sizeof(ffi_cif));

  status = ffi_prep_cif(
      (ffi_cif *)cif->GetPointer(),
      FFI_DEFAULT_ABI,
      nargs,
      (ffi_type *)rtype->GetPointer(),
      (ffi_type **)atypes->GetPointer());

  if (status != FFI_OK) {
    delete cif;
    return THROW_ERROR_EXCEPTION("ffi_prep_cif() returned error.");
  }

  return scope.Close(Pointer::WrapInstance(cif));
}

Persistent<FunctionTemplate> CallbackInfo::callback_template;
pthread_t CallbackInfo::g_mainthread;
pthread_mutex_t CallbackInfo::g_queue_mutex;
std::queue<ThreadedCallbackInvokation *> CallbackInfo::g_queue;
uv_async_t CallbackInfo::g_async;


CallbackInfo::CallbackInfo(Handle<Function> func, void *closure, void *code) {
  m_function = Persistent<Function>::New(func);
  m_closure = closure;
  this->code = code;
}

CallbackInfo::~CallbackInfo() {
  ffi_closure_free(m_closure);
  m_function.Dispose();
}

void CallbackInfo::DispatchToV8(CallbackInfo *self, void *retval, void **parameters) {
  HandleScope scope;

  Handle<Value> argv[2];
  argv[0] = Pointer::WrapPointer((unsigned char *)retval);
  argv[1] = Pointer::WrapPointer((unsigned char *)parameters);

  TryCatch try_catch;

  self->m_function->Call(self->m_this, 2, argv);

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }
}

void CallbackInfo::WatcherCallback(uv_async_t *w, int revents) {
  pthread_mutex_lock(&g_queue_mutex);

  while (!g_queue.empty()) {
    ThreadedCallbackInvokation *inv = g_queue.front();
    g_queue.pop();

    DispatchToV8(inv->m_cbinfo, inv->m_retval, inv->m_parameters);
    inv->SignalDoneExecuting();
  }

  pthread_mutex_unlock(&g_queue_mutex);
}

void CallbackInfo::Initialize(Handle<Object> target) {
  HandleScope scope;

  if (callback_template.IsEmpty()) {
    callback_template = Persistent<FunctionTemplate>::New(MakeTemplate());
  }

  Handle<FunctionTemplate> t = callback_template;

  target->Set(String::NewSymbol("CallbackInfo"), t->GetFunction());

  // initialize our threaded invokation stuff
  g_mainthread = pthread_self();
  uv_async_init(uv_default_loop(), &g_async, CallbackInfo::WatcherCallback);
  pthread_mutex_init(&g_queue_mutex, NULL);

  // allow the event loop to exit while this is running
  //uv_unref(uv_default_loop());
}

Handle<Value> CallbackInfo::New(const Arguments& args) {
  HandleScope scope;

  if (args.Length() < 2) {
    return ThrowException(String::New("Not enough arguments."));
  }
  Pointer *cif = ObjectWrap::Unwrap<Pointer>(args[0]->ToObject());
  Local<Function> callback = Local<Function>::Cast(args[1]);
  ffi_closure *closure;
  ffi_status status;
  void *code;

  closure = (ffi_closure *)ffi_closure_alloc(sizeof(ffi_closure), &code);

  if (!closure) {
    return ThrowException(String::New("ffi_closure_alloc() Returned Error"));
  }

  CallbackInfo *self = new CallbackInfo(callback, closure, code);

  status = ffi_prep_closure_loc(
    closure,
    (ffi_cif *)cif->GetPointer(),
    Invoke,
    (void *)self,
    code
  );

  if (status != FFI_OK) {
    delete self;
    return ThrowException(String::New("ffi_prep_closure() Returned Error"));
  }

  self->Wrap(args.This());
  self->m_this = args.This();

  return scope.Close(args.This());
}

Handle<FunctionTemplate> CallbackInfo::MakeTemplate() {
  HandleScope scope;

  Handle<FunctionTemplate> t = FunctionTemplate::New(New);

  Local<ObjectTemplate> inst = t->InstanceTemplate();
  inst->SetInternalFieldCount(1);
inst->SetAccessor(String::NewSymbol("pointer"), GetPointer);

  return scope.Close(t);
}

void CallbackInfo::Invoke(ffi_cif *cif, void *retval, void **parameters, void *user_data) {
  CallbackInfo *self = (CallbackInfo *)user_data;

  // are we executing from another thread?
  if (pthread_equal(pthread_self(), g_mainthread)) {
    DispatchToV8(self, retval, parameters);
  } else {
    // create a temporary storage area for our invokation parameters
    ThreadedCallbackInvokation *inv = new ThreadedCallbackInvokation(self, retval, parameters);

    // push it to the queue -- threadsafe
    pthread_mutex_lock(&g_queue_mutex);
    g_queue.push(inv);
    pthread_mutex_unlock(&g_queue_mutex);

    // send a message to our main thread to wake up the WatchCallback loop
    uv_async_send(&g_async);

    // wait for signal from calling thread
    inv->WaitForExecution();

    delete inv;
  }
}

Handle<Value> CallbackInfo::GetPointer(Local<String> name, const AccessorInfo& info) {
  HandleScope scope;

  CallbackInfo *self = ObjectWrap::Unwrap<CallbackInfo>(info.Holder());
  Handle<Value> ptr = Pointer::WrapPointer((unsigned char *)self->m_closure);
  return scope.Close(ptr);
}

ForeignCaller::ForeignCaller() {
}

ForeignCaller::~ForeignCaller() {
}

Persistent<FunctionTemplate> ForeignCaller::foreign_caller_template;

Handle<FunctionTemplate> ForeignCaller::MakeTemplate() {
  HandleScope scope;
  Handle<FunctionTemplate> t = FunctionTemplate::New(New);

  Local<ObjectTemplate> inst = t->InstanceTemplate();
  inst->SetInternalFieldCount(1);

  return scope.Close(t);
}

void ForeignCaller::Initialize(Handle<Object> target) {
  HandleScope scope;

  if (foreign_caller_template.IsEmpty()) {
    foreign_caller_template = Persistent<FunctionTemplate>::New(MakeTemplate());
  }

  Handle<FunctionTemplate> t = foreign_caller_template;

  NODE_SET_PROTOTYPE_METHOD(t, "exec", Exec);

  target->Set(String::NewSymbol("ForeignCaller"), t->GetFunction());
}

Handle<Value> ForeignCaller::New(const Arguments& args) {
  HandleScope     scope;
  ForeignCaller   *self = new ForeignCaller();

  if (args.Length() != 5) {
return THROW_ERROR_EXCEPTION("new ForeignCaller() requires 5 arguments!");
  }

  Pointer *cif    = ObjectWrap::Unwrap<Pointer>(args[0]->ToObject());
  Pointer *fn     = ObjectWrap::Unwrap<Pointer>(args[1]->ToObject());
  Pointer *fnargs = ObjectWrap::Unwrap<Pointer>(args[2]->ToObject());
  Pointer *res    = ObjectWrap::Unwrap<Pointer>(args[3]->ToObject());

  self->m_cif     = (ffi_cif *)cif->GetPointer();
  self->m_fn      = (void (*)(void))fn->GetPointer();
  self->m_res     = (void *)res->GetPointer();
  self->m_fnargs  = (void **)fnargs->GetPointer();
  self->m_async   = args[4]->BooleanValue();

  self->Wrap(args.This());
  return scope.Close(args.This());
}

Handle<Value> ForeignCaller::Exec(const Arguments& args) {
  HandleScope     scope;
  ForeignCaller   *self = ObjectWrap::Unwrap<ForeignCaller>(args.This());

  if (self->m_async) {
    AsyncCallParams *p = new AsyncCallParams();

    // cuter way of doing this?
    p->cif  = self->m_cif;
    p->ptr  = self->m_fn;
    p->res  = self->m_res;
    p->args = self->m_fnargs;

    // get the events.EventEmitter constructor
    Local<Object> global = Context::GetCurrent()->Global();
    Local<Object> events = global->Get(String::NewSymbol("process"))->ToObject();
    Local<Function> emitterConstructor = Local<Function>::Cast(events->Get(String::NewSymbol("EventEmitter")));

    // construct a new EventEmitter object
    p->emitter = Persistent<Object>::New(emitterConstructor->NewInstance());

    uv_work_t *req = new uv_work_t;
req->data = p;
    uv_queue_work(uv_default_loop(), req, ForeignCaller::AsyncFFICall, ForeignCaller::FinishAsyncFFICall);

    return scope.Close(p->emitter);
  } else {
#if __OBJC__ || __OBJC2__
    @try {
#endif
      ffi_call(
          self->m_cif,
          self->m_fn,
          self->m_res,
          self->m_fnargs
        );
#if __OBJC__ || __OBJC2__
    } @catch (id ex) {
      return ThrowException(Pointer::WrapPointer((unsigned char *)ex));
    }
#endif
  }

  return Undefined();
}

/**
 * Called on the thread pool.
 */

void ForeignCaller::AsyncFFICall(uv_work_t *req) {
  AsyncCallParams *p = (AsyncCallParams *)req->data;
  ffi_call(p->cif, p->ptr, p->res, p->args);
}

/**
 * Called after the AsyncFFICall function completes on the thread pool.
 * This gets run on the loop thread.
 */

void ForeignCaller::FinishAsyncFFICall(uv_work_t *req) {
HandleScope scope;

  AsyncCallParams *p = (AsyncCallParams *)req->data;
  Local<Value> argv[1];

  argv[0] = Local<Value>::New(String::New("success"));

  // get a reference to the 'emit' function
  Local<Value> emitVal = p->emitter->Get(String::NewSymbol("emit"));
  Local<Function> emit = Local<Function>::Cast(emitVal);

  TryCatch try_catch;

  // emit a success event
  emit->Call(p->emitter, 1, argv);

  // dispose of our persistent handle to the EventEmitter object
  p->emitter.Dispose();

  // free up our memory (allocated in FFICall)
  delete p;
  delete req;

  if (try_catch.HasCaught()) {
    FatalException(try_catch);
  }
}

ThreadedCallbackInvokation::ThreadedCallbackInvokation(CallbackInfo *cbinfo, void *retval, void **parameters) {
  m_cbinfo = cbinfo;
  m_retval = retval;
  m_parameters = parameters;

  pthread_mutex_init(&m_mutex, NULL);
  pthread_cond_init(&m_cond, NULL);
  //uv_ref(uv_default_loop()); // hold the event loop open while this is executing
}

ThreadedCallbackInvokation::~ThreadedCallbackInvokation() {
//  uv_unref(uv_default_loop());
  pthread_cond_destroy(&m_cond);
  pthread_mutex_destroy(&m_mutex);
}

void ThreadedCallbackInvokation::SignalDoneExecuting() {
  pthread_mutex_lock(&m_mutex);
  pthread_cond_signal(&m_cond);
  pthread_mutex_unlock(&m_mutex);
}

void ThreadedCallbackInvokation::WaitForExecution() {
  pthread_mutex_lock(&m_mutex);
  pthread_cond_wait(&m_cond, &m_mutex);
  pthread_mutex_unlock(&m_mutex);
}

Pointer::Pointer(unsigned char *ptr) {
  this->origPtr = ptr;
  this->m_ptr = ptr;
  this->m_allocated = 0;
  this->doFree = false;
  //fprintf(stderr, "Creating new Pointer %p\n", this->m_ptr);
}

Pointer::~Pointer() {
  if (this->doFree) {
    //fprintf(stderr, "Pointer destructor called on ALLOCATED area: %p\n", this->m_ptr);
    free(this->origPtr);
  }
}

Persistent<FunctionTemplate> Pointer::pointer_template;

Handle<FunctionTemplate> Pointer::MakeTemplate() {
  HandleScope scope;
  Handle<FunctionTemplate> t = FunctionTemplate::New(New);
  t->SetClassName(String::NewSymbol("Pointer"));

  Local<ObjectTemplate> inst = t->InstanceTemplate();
  inst->SetInternalFieldCount(1);
  inst->SetAccessor(String::NewSymbol("address"), GetAddress);
  inst->SetAccessor(String::NewSymbol("allocated"), GetAllocated);
  inst->SetAccessor(String::NewSymbol("free"), GetFree, SetFree);

  return scope.Close(t);
}

void Pointer::Initialize(Handle<Object> target) {
  HandleScope scope;

  if (pointer_template.IsEmpty()) {
    pointer_template = Persistent<FunctionTemplate>::New(MakeTemplate());
  }
  Handle<FunctionTemplate> t = pointer_template;

  NODE_SET_PROTOTYPE_METHOD(t, "seek", Seek);
  NODE_SET_PROTOTYPE_METHOD(t, "putUInt8", PutUInt8);
  NODE_SET_PROTOTYPE_METHOD(t, "getUInt8", GetUInt8);
  NODE_SET_PROTOTYPE_METHOD(t, "putInt8", PutInt8);
  NODE_SET_PROTOTYPE_METHOD(t, "getInt8", GetInt8);
  NODE_SET_PROTOTYPE_METHOD(t, "putInt16", PutInt16);
  NODE_SET_PROTOTYPE_METHOD(t, "getInt16", GetInt16);
  NODE_SET_PROTOTYPE_METHOD(t, "putUInt16", PutUInt16);
  NODE_SET_PROTOTYPE_METHOD(t, "getUInt16", GetUInt16);
  NODE_SET_PROTOTYPE_METHOD(t, "putInt32", PutInt32);
  NODE_SET_PROTOTYPE_METHOD(t, "getInt32", GetInt32);
  NODE_SET_PROTOTYPE_METHOD(t, "putUInt32", PutUInt32);
  NODE_SET_PROTOTYPE_METHOD(t, "getUInt32", GetUInt32);
  NODE_SET_PROTOTYPE_METHOD(t, "putInt64", PutInt64);
  NODE_SET_PROTOTYPE_METHOD(t, "getInt64", GetInt64);
  NODE_SET_PROTOTYPE_METHOD(t, "putUInt64", PutUInt64);
  NODE_SET_PROTOTYPE_METHOD(t, "getUInt64", GetUInt64);
  NODE_SET_PROTOTYPE_METHOD(t, "putFloat", PutFloat);
  NODE_SET_PROTOTYPE_METHOD(t, "getFloat", GetFloat);
  NODE_SET_PROTOTYPE_METHOD(t, "putDouble", PutDouble);
  NODE_SET_PROTOTYPE_METHOD(t, "getDouble", GetDouble);
  NODE_SET_PROTOTYPE_METHOD(t, "_putPointer", PutPointerMethod);
  NODE_SET_PROTOTYPE_METHOD(t, "getPointer", GetPointerMethod);
  NODE_SET_PROTOTYPE_METHOD(t, "getObject", GetObject);
  NODE_SET_PROTOTYPE_METHOD(t, "putObject", PutObject);
  NODE_SET_PROTOTYPE_METHOD(t, "putCString", PutCString);
  NODE_SET_PROTOTYPE_METHOD(t, "getCString", GetCString);
  NODE_SET_PROTOTYPE_METHOD(t, "isNull", IsNull);
  NODE_SET_PROTOTYPE_METHOD(t, "toBuffer", ToBuffer);

  target->Set(String::NewSymbol("Pointer"), t->GetFunction());
}

unsigned char *Pointer::GetPointer() {
  return this->m_ptr;
}

void Pointer::MovePointer(int bytes) {
  this->m_ptr += bytes;
}

Handle<Value> Pointer::Alloc(size_t bytes) {
  if (!this->m_allocated && bytes > 0) {
    this->m_ptr = (unsigned char *)malloc(bytes);
    this->origPtr = this->m_ptr;
    //fprintf(stderr, "malloc()'d %p\n", this->m_ptr);

    if (this->m_ptr != NULL) {
      this->m_allocated = bytes;

      // Any allocated Pointer gets free'd by default
      // This can be changed in JS-land with `free`
      this->doFree = true;
    } else {
      return THROW_ERROR_EXCEPTION("malloc(): Could not allocate Memory");
    }
  }
  return Undefined();
}

/**
 * Sentinel Object used to determine when Pointer::New() is being called from
 * JS-land or from within a Pointer::WrapInstance() call.
 */

static Persistent<Value> SENTINEL;

Handle<Object> Pointer::WrapInstance(Pointer *inst) {
  HandleScope scope;
  Persistent<Value> WRAP_ARGS[] = { SENTINEL };

  Local<Object> obj = pointer_template->GetFunction()->NewInstance(1, WRAP_ARGS);
  inst->Wrap(obj);
  return scope.Close(obj);
}

Handle<Object> Pointer::WrapPointer(unsigned char *ptr) {
  return WrapInstance(new Pointer(ptr));
}

Handle<Value> Pointer::New(const Arguments& args) {
  HandleScope scope;

  int argc = args.Length();

  if (argc < 1) {
    return THROW_ERROR_EXCEPTION("new Pointer() requires at least 1 argument");
  }

  Handle<Value> arg0 = args[0];
  if (!arg0->StrictEquals(SENTINEL)) {
    Pointer *self = new Pointer(NULL);
    unsigned int sz = arg0->Uint32Value();
    self->Alloc(sz);

    if (argc >= 2) {
      // Second argument specifies whether or not to call `free()` on the
      // allocated buffer when this Pointer gets garbage collected
      self->doFree = args[1]->BooleanValue();
    }
    self->Wrap(args.This());
  }

  return scope.Close(args.This());
}

Handle<Value> Pointer::GetAddress(Local<String> name, const AccessorInfo& info) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(info.Holder());
  Handle<Value>   ret;

  ret = Number::New((size_t)self->GetPointer());

  return scope.Close(ret);
}

Handle<Value> Pointer::GetAllocated(Local<String> name, const AccessorInfo& info) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(info.Holder());
Handle<Value>   ret = Integer::New(self->m_allocated);
  return scope.Close(ret);
}

Handle<Value> Pointer::GetFree(Local<String> name, const AccessorInfo& info) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(info.Holder());
  Handle<Value>   ret = Boolean::New(self->doFree);
  return scope.Close(ret);
}

void Pointer::SetFree(Local<String> property, Local<Value> value, const AccessorInfo &info) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(info.Holder());
  Handle<Boolean> val = value->ToBoolean();
  self->doFree = val->BooleanValue();
}


Handle<Value> Pointer::Seek(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  Handle<Value>   ret;

  if (args.Length() > 0 && args[0]->IsNumber()) {
    size_t offset = args[0]->IntegerValue();
    ret = WrapPointer(static_cast<unsigned char *>(self->GetPointer()) + offset);
  }
  else {
    return THROW_ERROR_EXCEPTION("Must specify an offset");
  }

  return scope.Close(ret);
}

Handle<Value> Pointer::PutUInt8(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  if (args.Length() >= 1 && args[0]->IsNumber()) {
int64_t val = args[0]->IntegerValue();

    if (val >= UINT8_MIN && val <= UINT8_MAX) {
      uint8_t cvt = (uint8_t)val;
      memcpy(ptr, &cvt, sizeof(uint8_t));
    }
    else {
      return THROW_ERROR_EXCEPTION("Value out of Range.");
    }
  }
  if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
    self->MovePointer(sizeof(uint8_t));
  }

  return Undefined();
}

Handle<Value> Pointer::GetUInt8(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(uint8_t));
  }

  return scope.Close(Integer::New(*ptr));
}

Handle<Value> Pointer::PutInt8(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  if (args.Length() >= 1 && args[0]->IsNumber()) {
    int64_t val = args[0]->IntegerValue();

    if (val >= INT8_MIN && val <= INT8_MAX) {
      int8_t cvt = (int8_t)val;
      memcpy(ptr, &cvt, sizeof(int8_t));
    }
else {
      return THROW_ERROR_EXCEPTION("Value out of Range.");
    }
  }
  if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
    self->MovePointer(sizeof(int8_t));
  }

  return Undefined();
}

Handle<Value> Pointer::GetInt8(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr  = self->GetPointer();
  int8_t          val   = *((int8_t *)ptr);

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(int8_t));
  }

  return scope.Close(Integer::New(val));
}


Handle<Value> Pointer::PutInt16(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  // TODO: Exception handling here for out of range values
  if (args.Length() >= 1 && args[0]->IsNumber()) {
    int64_t val = args[0]->IntegerValue();

    if (val >= INT16_MIN && val <= INT16_MAX) {
      int16_t cvt = (int16_t)val;
      memcpy(ptr, &cvt, sizeof(int16_t));
    }
    else {
      return THROW_ERROR_EXCEPTION("Value out of Range.");
    }

 }
  if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
    self->MovePointer(sizeof(int16_t));
  }

  return Undefined();
}

Handle<Value> Pointer::GetInt16(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();
  int16_t          val = *((int16_t *)ptr);

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(int16_t));
  }

  return scope.Close(Integer::New(val));
}

Handle<Value> Pointer::PutUInt16(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  // TODO: Exception handling here for out of range values
  if (args.Length() >= 1 && args[0]->IsNumber()) {
    int64_t val = args[0]->IntegerValue();

    if (val >= UINT16_MIN && val <= UINT16_MAX) {
      uint16_t cvt = (uint16_t)val;
      memcpy(ptr, &cvt, sizeof(uint16_t));
    }
    else {
      return THROW_ERROR_EXCEPTION("Value out of Range.");
    }
  }
  if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
    self->MovePointer(sizeof(uint16_t));
  }
 return Undefined();
}

Handle<Value> Pointer::GetUInt16(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();
  unsigned short  val = *((uint16_t *)ptr);

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(uint16_t));
  }

  return scope.Close(Integer::New(val));
}

Handle<Value> Pointer::PutInt32(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  if (args.Length() >= 1 && args[0]->IsNumber()) {
    int64_t val = args[0]->IntegerValue();

    if (val >= INT32_MIN && val <= INT32_MAX) { // XXX: Will this ever be false?
      memcpy(ptr, &val, sizeof(int32_t));
    }
    else {
      return THROW_ERROR_EXCEPTION("Value out of Range.");
    }
  }
  if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
    self->MovePointer(sizeof(int32_t));
  }

  return Undefined();
}

Handle<Value> Pointer::GetInt32(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
unsigned char   *ptr = self->GetPointer();
  int32_t         val = *((int32_t *)ptr);

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(int32_t));
  }

  return scope.Close(Integer::New(val));
}

Handle<Value> Pointer::PutUInt32(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  if (args.Length() >= 1 && args[0]->IsNumber()) {
    int64_t val = args[0]->IntegerValue();

    if (val >= UINT32_MIN && val <= UINT32_MAX) { // XXX: Will this ever be false?
      memcpy(ptr, &val, sizeof(uint32_t));
    }
    else {
      return THROW_ERROR_EXCEPTION("Value out of Range.");
    }
  }
  if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
    self->MovePointer(sizeof(uint32_t));
  }

  return Undefined();
}

Handle<Value> Pointer::GetUInt32(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();
  uint32_t        val = *((uint32_t *)ptr);

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(uint32_t));
  }

  return scope.Close(Integer::NewFromUnsigned(val));
}

Handle<Value> Pointer::PutInt64(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  // Have to do this because strtoll doesn't set errno to 0 on success :(
  errno = 0;

  if (args.Length() >= 1) {
    if (args[0]->IsNumber() || args[0]->IsString()) {
      int64_t val;

      if (args[0]->IsNumber()) {
        val = args[0]->IntegerValue();
      }
      else { // assumed args[0]->IsString() from condition above
        String::Utf8Value str(args[0]->ToString());
        val = STR_TO_INT64(*str);
      }

      if (errno != ERANGE && (val >= INT64_MIN && val <= INT64_MAX)) {
        memcpy(ptr, &val, sizeof(int64_t));
      }
      else {
        return THROW_ERROR_EXCEPTION("Value out of Range.");
}
    }
  }
  if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
    self->MovePointer(sizeof(int64_t));
  }

  return Undefined();
}

Handle<Value> Pointer::GetInt64(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();
  int64_t         val = *((int64_t *)ptr);
  char            buf[INTEGER_CONVERSION_BUFFER_SIZE];

  memset(buf, 0, INTEGER_CONVERSION_BUFFER_SIZE);
  snprintf(buf, INTEGER_CONVERSION_BUFFER_SIZE, "%lld", val);

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(int64_t));
  }

  return scope.Close(String::New(buf));
}

Handle<Value> Pointer::PutUInt64(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  // Have to do this because strtoull doesn't set errno to 0 on success :(
  errno = 0;

  if (args.Length() >= 1) {
    if (args[0]->IsNumber() || args[0]->IsString()) {
      uint64_t val;

      // Convert everything to a string because it's easier this way
      String::Utf8Value str(args[0]->ToString());
      val = STR_TO_UINT64(*str);

      if ((*str)[0] != '-' && errno != ERANGE && val <= UINT64_MAX) {
        memcpy(ptr, &val, sizeof(uint64_t));
      } else {
        return THROW_ERROR_EXCEPTION("Value out of Range.");
      }
    }
  }
  if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
    self->MovePointer(sizeof(uint64_t));
  }

  return Undefined();
}

Handle<Value> Pointer::GetUInt64(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();
  uint64_t        val = *((uint64_t *)ptr);
  char            buf[INTEGER_CONVERSION_BUFFER_SIZE];

  memset(buf, 0, INTEGER_CONVERSION_BUFFER_SIZE);
  snprintf(buf, INTEGER_CONVERSION_BUFFER_SIZE, "%llu", val);

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(uint64_t));
  }
return scope.Close(String::New(buf));
}

Handle<Value> Pointer::PutFloat(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  if (args.Length() >= 1 && args[0]->IsNumber()) {
    float val = args[0]->NumberValue();
    memcpy(ptr, &val, sizeof(float));
  }
  if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
    self->MovePointer(sizeof(float));
  }

  return Undefined();
}

Handle<Value> Pointer::GetFloat(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();
  float           val = *((float *)ptr);

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(float));
  }

  return scope.Close(Number::New((double)val));
}

Handle<Value> Pointer::PutDouble(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  if (args.Length() >= 1 && args[0]->IsNumber()) {
    double val = args[0]->NumberValue();
    memcpy(ptr, &val, sizeof(double));
  }
  if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
    self->MovePointer(sizeof(double));
  }

  return Undefined();
}

Handle<Value> Pointer::GetDouble(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();
  double          val = *((double *)ptr);

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(double));
  }

  return scope.Close(Number::New(val));
}

Handle<Value> Pointer::PutPointerMethod(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  if (args.Length() >= 1) {
    if (args[0]->IsNull()) {
      *((unsigned char **)ptr) = NULL;
    }
    else {
Pointer *obj = ObjectWrap::Unwrap<Pointer>(args[0]->ToObject());
      *((unsigned char **)ptr) = obj->GetPointer();
    }
    //printf("Pointer::PutPointerMethod: writing pointer %p at %p\n", *((unsigned char **)ptr), ptr);

    if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
      self->MovePointer(sizeof(unsigned char *));
    }
  }

  return Undefined();
}

Handle<Value> Pointer::GetPointerMethod(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();
  unsigned char   *val = *((unsigned char **)ptr);

  //printf("Pointer::GetPointerMethod: got %p from %p\n", val, ptr);

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(unsigned char *));
  }

  return scope.Close(WrapPointer(val));
}

Handle<Value> Pointer::PutObject(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());

  if (args.Length() >= 1) {
    Local<Value> obj = args[0];
    *reinterpret_cast<Persistent<Value>*>(self->GetPointer()) = Persistent<Value>::New(obj);

    if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
      self->MovePointer(sizeof(Persistent<Value>));
    }
  }

  return Undefined();
}

Handle<Value> Pointer::GetObject(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();
  Persistent<Value> rtn = *reinterpret_cast<Persistent<Value>*>(ptr);

  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(sizeof(Persistent<Value>));
  }

  return scope.Close(rtn);
}

Handle<Value> Pointer::PutCString(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  unsigned char   *ptr = self->GetPointer();

  if (args.Length() >= 1 && args[0]->IsString()) {
    args[0]->ToString()->WriteUtf8((char *)ptr);

    if (args.Length() == 2 && args[1]->IsBoolean() && args[1]->BooleanValue()) {
      self->MovePointer(args[0]->ToString()->Utf8Length() + 1);
    }
  }

  return Undefined();
}

Handle<Value> Pointer::GetCString(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  char            *val = (char *)self->GetPointer();

  Local<String> rtn = String::New(val);
  if (args.Length() == 1 && args[0]->IsBoolean() && args[0]->BooleanValue()) {
    self->MovePointer(rtn->Utf8Length() + 1);
  }

  return scope.Close(rtn);
}

Handle<Value> Pointer::IsNull(const Arguments& args) {
  HandleScope     scope;
  Pointer         *self = ObjectWrap::Unwrap<Pointer>(args.This());
  return scope.Close(Boolean::New(self->GetPointer() == NULL));
}

// Callback that gets invoked when the Buffer returned from ToBuffer is being freed
void Pointer::unref_pointer_callback(char *data, void *hint) {
  Pointer *p = static_cast<Pointer *>(hint);
  //printf("Unref()ing pointer\n");
  p->Unref();
}

Handle<Value> Pointer::ToBuffer(const Arguments& args) {
  HandleScope scope;
  Pointer *self = ObjectWrap::Unwrap<Pointer>(args.This());

  // Defaults to the size of the allocated Buffer area, but can be explicitly
  // specified as the first argument.
  unsigned int sz = self->m_allocated;
  if (args.Length() >= 1) {
    sz = args[0]->Uint32Value();
  }

  Buffer *slowBuffer = Buffer::New((char *)self->GetPointer(), (size_t)sz, unref_pointer_callback, self);

  // increase the reference count for this Pointer
  self->Ref();

  return scope.Close(slowBuffer->handle_);
}

void init(Handle<Object> target) {
  HandleScope scope;

  SENTINEL = Persistent<Object>::New(Object::New());

  Pointer::Initialize(target);
  FFI::InitializeBindings(target);
  FFI::InitializeStaticFunctions(target);
  CallbackInfo::Initialize(target);
  ForeignCaller::Initialize(target);
}

} // namespace ffi

NODE_MODULE(node_ffi, ffi::init);
