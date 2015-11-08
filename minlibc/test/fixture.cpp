
#include "fixture.h"


char buffer[BUFFER_SIZE];
uint32_t i = 0;

int _open(const char *name, int flags, int mode)
{
    (void)name;
    (void)flags;
    (void)mode;
    return 3;
}

int _close(int file)
{
    return file == EOF ? EOF : 0;
}

int _unlink(char *name)
{
    return name == NULL ? EOF : 0;
}

int _rename(const char *oldname, const char *newname)
{
    return oldname == NULL ? EOF : 0;
}

int mkdir(const char *pathname, mode_t mode)
{
    return pathname == NULL ? EOF : 0;
}

int _fsync(int file)
{
    return file == EOF ? EOF : 0;
}

int _write(int file, char *buf, unsigned int count)
{
    if(file == EOF)
        return  EOF;

    if(i >= sizeof(buffer))
        return EOF;

    int n = EOF;

    for(n = 0; n < (int)count && i < sizeof(buffer); n++, i++) {
        buffer[i] = *buf;
        buf++;
    }

    return n;
}

int _read(int file, char *buf, unsigned int count)
{
    if(file == EOF)
        return  EOF;

    if(i >= sizeof(buffer))
        return EOF;

    int n = EOF;

    for(n = 0; n < (int)count &&  i < sizeof(buffer); n++, i++) {
        *buf = buffer[i];
        buf++;
    }

    return n;
}

extern long int _ftell(int file)
{
    if(file == EOF)
        return  EOF;

    return i;
}

extern int _lseek(int file, int offset, int whence)
{
    if(file == EOF)
        return  EOF;

    int n = EOF;

    if(whence == SEEK_CUR)
        offset = _ftell(file) + offset;
    else if(whence == SEEK_END)
        offset = sizeof(buffer) - offset;

    i = offset;
    n = 0;

    return n;
}


void reset_fixture()
{
    memset(buffer, 0, sizeof(buffer));
    i = 0;
}

char* get_buffer()
{
    return buffer;
}
