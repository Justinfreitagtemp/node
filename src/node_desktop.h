#ifndef __NODE_DESKTOP_H__
#define __NODE_DESKTOP_H__

#include <node.h>
#include <v8.h>

#include <string>

namespace node_desktop {

class Baton {
  public:
    v8::Persistent<v8::Function> callback;
    std::string *error;

    Baton(v8::Local<v8::Value> callback) {
      this->callback =
        v8::Persistent<v8::Function>::New(v8::Local<v8::Function>::Cast(callback));
      error = NULL;
    }

    ~Baton() {
      callback.Dispose();
      delete error;
    }
};

static v8::Handle<v8::Value> InitDesktop(const v8::Arguments &args);
static void DoingInitDesktop(uv_work_t *request);
static void AfterInitDesktop(uv_work_t *request);

void init(v8::Handle<v8::Object> target);

} // namespace node_desktop

#endif
