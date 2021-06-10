#include "shared_memory.hpp"
#include <napi_tools.hpp>
#include <random>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#   define OS_WINDOWS
#endif

#ifdef OS_WINDOWS

#include <windows.h>

#undef min
#undef max

#else

#include <sys/ipc.h>
#include <sys/shm.h>

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

#else

std::string getErrnoAsString() {
    return strerror(errno);
}

#endif //OS_WINDOWS

class shared_memory::extra_info {
public:
#ifdef OS_WINDOWS

    explicit extra_info(HANDLE map) : map(map) {}

    HANDLE map;
#else

    explicit extra_info(int id) : id(id) {}

    int id;
#endif //OS_WINDOWS
};

void shared_memory::init(Napi::Env env, Napi::Object &exports) {
    Napi::Function func = DefineClass(env, "shared_memory", {
            InstanceMethod("write", &shared_memory::writeData, napi_enumerable),
            InstanceMethod("read", &shared_memory::readString, napi_enumerable),
            InstanceMethod("readBuffer", &shared_memory::readBuffer, napi_enumerable),
            InstanceAccessor("data", &shared_memory::readString, &shared_memory::setString, napi_enumerable),
            InstanceAccessor("buffer", &shared_memory::readBuffer, &shared_memory::setBuffer, napi_enumerable),
            StaticMethod("generateId", &shared_memory::generateId, napi_enumerable),
            StaticMethod("generateIdAsync", &shared_memory::generateIdAsync, napi_enumerable)
    });

    auto constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);

    exports.Set("shared_memory", func);
    env.SetInstanceData<Napi::FunctionReference>(constructor);
}

#ifdef OS_WINDOWS
std::string getGenerateId(bool global) {
#else

std::string getGenerateId(bool) {
#endif //OS_WINDOWS
    static std::random_device dev;
    static std::mt19937 rng(dev());
    static std::uniform_int_distribution<int> dist(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
#ifdef OS_WINDOWS
    SetLastError(0);
    int id = dist(rng);
    std::string name = (global ? "Global\\" : "") + std::to_string(id);

    HANDLE handle = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 1, name.c_str());
    while (GetLastError() == ERROR_ALREADY_EXISTS) {
        id = dist(rng);
        name = (global ? "Global\\" : "") + std::to_string(id);

        if (handle == nullptr) {
            throw std::runtime_error("Could not get the handle: " + GetLastErrorAsString());
        } else {
            CloseHandle(handle);
        }

        handle = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 1, name.c_str());
    }

    if (handle == nullptr) {
        throw std::runtime_error("Could not get the handle: " + GetLastErrorAsString());
    } else {
        CloseHandle(handle);
    }

    return std::to_string(id);
#else
    static std::hash<std::string> hash;

    int id = dist(rng);
    key_t key = static_cast<key_t>(hash(std::to_string(id)));
    int shm_id;
    while ((shm_id = shmget(key, 1, IPC_CREAT | IPC_EXCL)) < 0) {
        id = dist(rng);
        key = static_cast<key_t>(hash(std::to_string(id)));
    }

    shmctl(shm_id, IPC_RMID, nullptr);
    return std::to_string(id);
#endif //OS_WINDOWS
}

Napi::Value shared_memory::generateId(const Napi::CallbackInfo &info) {
    bool global = info[0].IsBoolean() && info[0].ToBoolean().Value();

    TRY
        return Napi::String::New(info.Env(), getGenerateId(global));
    CATCH_EXCEPTIONS
}

Napi::Value shared_memory::generateIdAsync(const Napi::CallbackInfo &info) {
    bool global = info[0].IsBoolean() && info[0].ToBoolean().Value();
    return napi_tools::promises::promise<std::string>(info.Env(), [global] {
        return getGenerateId(global);
    });
}

shared_memory::shared_memory(const Napi::CallbackInfo &info) : ObjectWrap(info) {
    CHECK_ARGS(napi_tools::string, napi_tools::number);
    global = info[2].IsBoolean() && info[2].ToBoolean().Value();

#ifdef OS_WINDOWS
    std::string name;
    if (global) {
        name = "Global\\" + info[0].ToString().Utf8Value();
    } else {
        name = info[0].ToString().Utf8Value();
    }
#else
    std::string name = info[0].ToString().Utf8Value();
#endif //OS_WINDOWS

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
                                     Napi::PropertyDescriptor::Value("name", info[0].ToString(),
                                                                     napi_enumerable),
                                     Napi::PropertyDescriptor::Value("host", Napi::Boolean::New(info.Env(), isHost),
                                                                     napi_enumerable),
                                     Napi::PropertyDescriptor::Value("global", Napi::Boolean::New(info.Env(), global),
                                                                     napi_enumerable)
                             });

#ifdef OS_WINDOWS
    HANDLE map;
    if (isHost) {
        SetLastError(0);
        map = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, size, name.c_str());

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            throw Napi::Error::New(info.Env(), "Could not create the memory handle: " + GetLastErrorAsString());
        }
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

    Value().DefineProperty(Napi::PropertyDescriptor::Value("id", Napi::String::New(info.Env(), name), napi_enumerable));
#else
    std::hash<std::string> hash;
    auto key = static_cast<key_t>(hash(name));

    if (isHost) {
        int id = shmget(key, size, IPC_CREAT | IPC_EXCL);
        if (id < 0) {
            throw Napi::Error::New(info.Env(), "Could not create the shared memory segment: " + getErrnoAsString());
        } else {
            extraInfo = std::make_shared<extra_info>(id);
        }

        buffer = static_cast<char *>(shmat(id, nullptr, 0777));
        if (reinterpret_cast<intptr_t>(buffer) <= 0) {
            throw Napi::Error::New(info.Env(), "Could not attach the shared memory segment: " + getErrnoAsString());
        }
    } else {
        int id = shmget(key, size, 0777);
        if (id < 0) {
            throw Napi::Error::New(info.Env(), "Could not get the shared memory segment: " + getErrnoAsString());
        } else {
            extraInfo = std::make_shared<extra_info>(id);
        }

        buffer = static_cast<char *>(shmat(id, nullptr, 0));
        if (reinterpret_cast<intptr_t>(buffer) <= 0) {
            throw Napi::Error::New(info.Env(), "Could not attach the shared memory segment: " + getErrnoAsString());
        }
    }

    Value().DefineProperty(Napi::PropertyDescriptor::Value("id", Napi::Number::From(info.Env(), key), napi_enumerable));
#endif //OS_WINDOWS
}

void shared_memory::writeData(const Napi::CallbackInfo &info) {
    CHECK_ARGS(napi_tools::string | napi_tools::buffer);
    std::vector<char> data;
    if (info[0].IsBuffer()) {
        auto buf = info[0].As<Napi::Buffer<char >>();
        data = std::vector<char>(buf.Data(), buf.Data() + buf.Length());
    } else {
        std::string string = info[0].ToString();
        data = std::vector<char>(string.begin(), string.end());

        if (string.size() < this->size) {
            this->buffer[string.size()] = '\0';
        }
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

    if (data.size() < this->size) {
        this->buffer[data.size()] = '\0';
    }

    memcpy(this->buffer, data.c_str(), data.size());
}

void shared_memory::setBuffer(const Napi::CallbackInfo &info, const Napi::Value &value) {
    if (!value.IsBuffer()) {
        throw Napi::TypeError::New(info.Env(), "The buffer setter requires a buffer as an argument");
    }

    auto buf = info[0].As<Napi::Buffer<char >>();
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
#else
    shmdt(this->buffer);
    if (this->isHost) {
        shmctl(this->extraInfo->id, IPC_RMID, nullptr);
    }
#endif // OS_WINDOWS
}