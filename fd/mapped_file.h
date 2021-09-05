#pragma once
#include "fd.h"
#include <sys/mman.h>
#include <sys/stat.h>

class MappedFile
{
    unsigned char* buffer;
    std::string path;
    FD fd;
    bool valid;
    size_t fileSize;

    public:
    MappedFile(const std::string& _path) : path(_path), fd(open(_path.c_str(),O_RDONLY)) {
        valid = true;
        struct stat s;
        if(fstat(fd,&s) == -1) {
            valid = false;
            return;
        }

        fileSize = s.st_size;
        buffer = static_cast<unsigned char*>(mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0));
    }

    MappedFile(const MappedFile& other) = delete;

    ~MappedFile() {
        if(valid) munmap(buffer, fileSize);
    }
    
    inline const unsigned char* getBuffer() const { return valid ? buffer : reinterpret_cast<const unsigned char*>(""); }
    inline bool isValid() const { return valid; } 
};
