#include <napi.h>
#include "shared_memory.hpp"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
    shared_memory::init(env, exports);
    return exports;
}

NODE_API_MODULE(shared_memory, InitAll)