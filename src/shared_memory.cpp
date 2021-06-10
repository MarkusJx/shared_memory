#include "shared_memory.hpp"
#include <napi_tools.hpp>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#   define OS_WINDOWS
#endif

#ifdef OS_WINDOWS

#include <windows.h>

#undef min

#endif

#ifdef OS_WINDOWS

// Source: https://stackoverflow.com/a/17387176
std::string GetLastErrorAsString() {
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
            errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, nullptr);

    // If the message ends with a new line
    // (+ a carriage return ['\r'], this is still windows) remove that
    if (size >= 2 && messageBuffer[size - 1] == '\n') {
        size -= 2;
    }

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

#endif

class shared_memory::extra_info {
public:
#ifdef OS_WINDOWS

    explicit extra_info(HANDLE map) : map(map) {}

    HANDLE map;
#endif
};

void shared_memory::init(Napi::Env env, Napi::Object &exports) {
    Napi::Function func = DefineClass(env, "shared_memory", {
            InstanceMethod("writeSync", &shared_memory::writeData, napi_enumerable),
            InstanceMethod("readSync", &shared_memory::readString, napi_enumerable),
            InstanceMethod("readBufferSync", &shared_memory::readBuffer, napi_enumerable),
            InstanceAccessor("data", &shared_memory::readString, &shared_memory::setString, napi_enumerable),
            InstanceAccessor("buffer", &shared_memory::readBuffer, &shared_memory::setBuffer, napi_enumerable)
    });

    auto constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);

    exports.Set("shared_memory", func);
    env.SetInstanceData<Napi::FunctionReference>(constructor);
}

shared_memory::shared_memory(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    CHECK_ARGS(napi_tools::string, napi_tools::number);
    global = info[2].IsBoolean() && info[2].ToBoolean().Value();

    std::string name;
    if (global) {
        name = "Global\\" + info[0].ToString().Utf8Value();
    } else {
        name = info[0].ToString().Utf8Value();
    }

    size = info[1].ToNumber().Int64Value();
    if (size <= 0) {
        throw Napi::TypeError::New(info.Env(), "The buffer size must be greater than zero");
    }

    if (info[3].IsBoolean()) {
        isHost = info[3].ToBoolean().Value();
    } else {
        isHost = true;
    }

    Value().DefineProperties({
                                     Napi::PropertyDescriptor::Value("size", Napi::Number::From(info.Env(), size),
                                                                     napi_enumerable),
                                     Napi::PropertyDescriptor::Value("name", Napi::String::New(info.Env(), name),
                                                                     napi_enumerable),
                                     Napi::PropertyDescriptor::Value("host", Napi::Boolean::New(info.Env(), isHost),
                                                                     napi_enumerable),
                                     Napi::PropertyDescriptor::Value("global", Napi::Boolean::New(info.Env(), global),
                                                                     napi_enumerable)
                             });

    HANDLE map;
    if (isHost) {
        map = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size, name.c_str());
    } else {
        map = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
    }

    if (map == nullptr) {
        throw Napi::Error::New(info.Env(), "Could not get/create the memory handle: " + GetLastErrorAsString());
    } else {
        extraInfo = std::make_shared<extra_info>(map);
    }

    buffer = (char *) MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (buffer == nullptr) {
        throw Napi::Error::New(info.Env(), "Could not get/create the data buffer: " + GetLastErrorAsString());
    }
}

void shared_memory::writeData(const Napi::CallbackInfo &info) {
    CHECK_ARGS(napi_tools::string | napi_tools::buffer);
    std::vector<char> data;
    if (info[0].IsBuffer()) {
        auto buf = info[0].As<Napi::Buffer<char>>();
        data = std::vector<char>(buf.Data(), buf.Data() + buf.Length());
    } else {
        std::string string = info[0].ToString();
        data = std::vector<char>(string.begin(), string.end());
    }

    if (data.size() > this->size) {
        throw Napi::Error::New(info.Env(), "Could not write to the buffer: The input is bigger than the buffer size");
    }

    memcpy(this->buffer, data.data(), data.size());
}

Napi::Value shared_memory::readString(const Napi::CallbackInfo &info) {
    std::string res(std::min(strlen(this->buffer), this->size), '\0');
    memcpy(res.data(), this->buffer, res.size());

    return Napi::String::New(info.Env(), res);
}

Napi::Value shared_memory::readBuffer(const Napi::CallbackInfo &info) {
    auto buf = Napi::Buffer<char>::New(info.Env(), this->size);
    memcpy(buf.Data(), this->buffer, this->size);

    return buf.ToObject();
}

void shared_memory::setString(const Napi::CallbackInfo &info, const Napi::Value &value) {
    if (!value.IsString()) {
        throw Napi::TypeError::New(info.Env(), "The string setter requires a string as an argument");
    }

    std::string data = value.ToString();
    if (data.size() > this->size) {
        throw Napi::Error::New(info.Env(), "Could not write to the buffer: The input is bigger than the buffer size");
    }

    memcpy(this->buffer, data.data(), data.size());
}

void shared_memory::setBuffer(const Napi::CallbackInfo &info, const Napi::Value &value) {
    if (!value.IsBuffer()) {
        throw Napi::TypeError::New(info.Env(), "The buffer setter requires a buffer as an argument");
    }

    auto buf = info[0].As<Napi::Buffer<char>>();
    if (buf.Length() > this->size) {
        throw Napi::Error::New(info.Env(), "Could not write to the buffer: The input is bigger than the buffer size");
    }

    memcpy(this->buffer, buf.Data(), buf.Length());
}

shared_memory::~shared_memory() {
#ifdef OS_WINDOWS
    UnmapViewOfFile(buffer);
    if (extraInfo->map != nullptr) {
        CloseHandle(extraInfo->map);
        extraInfo->map = nullptr;
    }
#endif // OS_WINDOWS
}