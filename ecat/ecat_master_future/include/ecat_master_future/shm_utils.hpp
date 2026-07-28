#pragma once

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


class SharedMemory {
    protected:
        const char* name_;
        size_t size_;
        void* ptr_ {nullptr};
        
        SharedMemory(const char* name, size_t size)
            : name_ (name), size_ (size) 
        {}

    public:
        SharedMemory(const SharedMemory&) = delete;
        SharedMemory& operator=(const SharedMemory&) = delete;

        virtual ~SharedMemory() = default;

        template <typename T>
        T* get() const { return reinterpret_cast<T*>(ptr_); }

        void* raw_ptr() const { return ptr_; }
        bool is_valid() const { return ptr_ != nullptr; }
};


class SharedMemoryOwner : public SharedMemory {
    public:
        SharedMemoryOwner(const char* name, size_t size);
        ~SharedMemoryOwner() override;
};

class SharedMemoryClient : public SharedMemory {
    public:
        SharedMemoryClient(const char* name, size_t size);
        ~SharedMemoryClient() override {
            if (ptr_ && ptr_ != MAP_FAILED)
                munmap(ptr_, size_);
        }
};  

class SharedMemoryOpenOrCreate : public SharedMemory {
public:
    SharedMemoryOpenOrCreate(const char* name, size_t size);
    ~SharedMemoryOpenOrCreate() override;

    bool created() const { return created_; }

private:
    bool created_ = false;
};
