#include "node_desktop.h"
#include "Desktop.h"

#include <node.h>
#include <v8.h>
#include <pthread.h>

#define TYPE_ERROR(message) \
  v8::Exception::TypeError(String::New(message))
#define THROW_TYPE_ERROR(message) \
  ThrowException(TYPE_ERROR(message))
#define BAD_ARGS \
  TYPE_ERROR("Bad argument/s")
#define THROW_BAD_ARGS \
  THROW_TYPE_ERROR("Bad argument/s")

using namespace v8;
using namespace node;

int main(int argc, char *argv[]) {
  pthread_t nodeThread;
  pthread_create(&nodeThread, NULL, &nodeInit, NULL);

  desktopInit();

  return 0;
}

void* nodeInit(void*) {
  char *argv[1] = { (char *) "node" };
  Start(1, argv);
  return NULL;
}

