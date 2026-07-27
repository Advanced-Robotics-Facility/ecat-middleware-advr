#include "ecat_master_future/shm_utils.hpp"

#include <iostream>
#include <system_error>

SharedMemoryOwner::SharedMemoryOwner(const char* name, size_t size) 
    : SharedMemory(name, size) 
{
    // Clear potential residue from previous crashes
    shm_unlink(name_);

    // Create shared memory object in /dev/shm
    int fd = shm_open(name_, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (fd < 0)
        return;

    // Set its size to the size of our structure
    if (ftruncate(fd, size_) == -1) {
        close(fd);
        return;
    }

    // Map the object into the caller's address space
    ptr_ = mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (ptr_ == MAP_FAILED) 
        ptr_ = nullptr;
}

SharedMemoryOwner::~SharedMemoryOwner() {
    if (ptr_ && ptr_ != MAP_FAILED)
        munmap(ptr_, size_);

    // Unlink the shared memory object from /dev/shm
    // Even if the peer process is still using the object, this is okay. 
    // The object will be removed only after all open references are closed
    shm_unlink(name_);
}

SharedMemoryClient::SharedMemoryClient(const char* name, size_t size)
    : SharedMemory(name, size)
{
    // Open existing shared memory object and map it into the caller's address space
    int fd = shm_open(name_, O_RDWR, 0666);
    if (fd < 0) {
        std::cerr << std::system_category().message(errno) << '\n';
        return;
    }

    ptr_ = mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (ptr_ == MAP_FAILED) {
        std::cerr << "Failed to map shared memory: " << name_ << std::endl;
        ptr_ = nullptr;
    }
}

SharedMemoryOpenOrCreate::SharedMemoryOpenOrCreate(const char* name, size_t size)
    : SharedMemory(name, size)
{
    int fd = shm_open(name_, O_CREAT | O_RDWR, 0666);
    if (fd < 0)
        return;

    struct stat st{};
    if (fstat(fd, &st) != 0) {
        close(fd);
        return;
    }

    created_ = (st.st_size == 0);

    if (created_) {
        if (ftruncate(fd, size_) != 0) {
            close(fd);
            return;
        }
    }

    ptr_ = mmap(nullptr,
                size_,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                fd,
                0);

    close(fd);

    if (ptr_ == MAP_FAILED)
        ptr_ = nullptr;
}

SharedMemoryOpenOrCreate::~SharedMemoryOpenOrCreate()
{
    if (ptr_)
        munmap(ptr_, size_);
}
