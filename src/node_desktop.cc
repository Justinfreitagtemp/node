#include "node_desktop.h"
#include "Desktop.h"

#define TYPE_ERROR(message) \
  v8::Exception::TypeError(String::New(message))
#define THROW_TYPE_ERROR(message) \
  ThrowException(TYPE_ERROR(message))
#define BAD_ARGS \
  TYPE_ERROR("Bad argument/s")
#define THROW_BAD_ARGS \
  THROW_TYPE_ERROR("Bad argument/s")

namespace node_desktop {

using namespace v8;
using namespace node;

Handle<Value> InitDesktop(const Arguments &args) {
  HandleScope scope;

  desktopInit();
  uv_work_t *request = new uv_work_t;
  request->data = new Baton(args[0]);
  uv_queue_work(uv_default_loop(), request, DoingInitDesktop, AfterInitDesktop);

  return scope.Close(Undefined());
}

void DoingInitDesktop(uv_work_t *request) {
  //desktopInit();
}

void AfterInitDesktop(uv_work_t *request) {
  Baton *baton = static_cast<Baton *>(request->data);

  Handle<Value> argv[1] = {Null()};
  TryCatch try_catch;
  baton->callback->Call(Context::GetCurrent()->Global(), 1, argv);
  if (try_catch.HasCaught()) FatalException(try_catch);

  delete baton;
  delete request;
}

void init(Handle<Object> target) {
  NODE_SET_METHOD(target, "initDesktop", InitDesktop);
}

} // namespace node_desktop

NODE_MODULE(node_desktop, node_desktop::init);

