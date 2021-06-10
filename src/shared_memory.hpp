#ifndef SHARED_MEMORY_SHARED_MEMORY_HPP
#define SHARED_MEMORY_SHARED_MEMORY_HPP

#include <memory>
#include <string>
#include <napi.h>

class shared_memory : public Napi::ObjectWrap<shared_memory> {
public:
    static void init(Napi::Env env, Napi::Object &exports);

    explicit shared_memory(const Napi::CallbackInfo &info);

    void writeData(const Napi::CallbackInfo &info);

    void setString(const Napi::CallbackInfo &info, const Napi::Value &value);

    void setBuffer(const Napi::CallbackInfo &info, const Napi::Value &value);

    Napi::Value readString(const Napi::CallbackInfo &info);

    Napi::Value readBuffer(const Napi::CallbackInfo &info);

    ~shared_memory() override;

private:
    class extra_info;

    bool global;
    bool isHost;
    size_t size;
    char *buffer;
    std::shared_ptr<extra_info> extraInfo;
};

#endif //SHARED_MEMORY_SHARED_MEMORY_HPP
