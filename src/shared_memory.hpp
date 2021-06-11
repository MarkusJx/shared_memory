#ifndef SHARED_MEMORY_SHARED_MEMORY_HPP
#define SHARED_MEMORY_SHARED_MEMORY_HPP

#include <memory>
#include <string>
#include <napi.h>

/**
 * The shared memory node wrapper class
 */
class shared_memory : public Napi::ObjectWrap<shared_memory> {
public:
    /**
     * Initialize the class
     *
     * @param env the environment
     * @param exports the exports
     */
    static void init(Napi::Env env, Napi::Object &exports);

    /**
     * Generate a shared memory id
     *
     * @param info the callback info
     * @return the generated id
     */
    static Napi::Value generateId(const Napi::CallbackInfo &info);

    /**
     * Generate a shared memory id. Async call.
     *
     * @param info the callback info
     * @return the generated id promise
     */
    static Napi::Value generateIdAsync(const Napi::CallbackInfo &info);

    /**
     * Create a shared_memory instance
     *
     * @param info the callback info
     */
    explicit shared_memory(const Napi::CallbackInfo &info);

    /**
     * Write data to the memory block
     *
     * @param info the callback info
     */
    void writeData(const Napi::CallbackInfo &info);

    /**
     * Copy a string to the memory block
     *
     * @param info the callback info
     * @param value the value to copy
     */
    void setString(const Napi::CallbackInfo &info, const Napi::Value &value);

    /**
     * Copy a node buffer to the memory block
     *
     * @param info the callback info
     * @param value the data to copy
     */
    void setBuffer(const Napi::CallbackInfo &info, const Napi::Value &value);

    /**
     * Read a string from the memory block
     *
     * @param info the callback info
     * @return the read string
     */
    Napi::Value readString(const Napi::CallbackInfo &info);

    /**
     * Read data into a node buffer from the memory block
     *
     * @param info the callback info
     * @return the read data
     */
    Napi::Value readBuffer(const Napi::CallbackInfo &info);

    /**
     * Destroy the shared memory instance
     */
    ~shared_memory() override;

private:
    /**
     * Some extra info
     */
    class extra_info;

    bool global;
    bool isHost;
    // The size of the memory block
    size_t size;
    // A pointer to the memory block
    char *buffer;
    // The extra info
    std::shared_ptr<extra_info> extraInfo;
};

#endif //SHARED_MEMORY_SHARED_MEMORY_HPP
