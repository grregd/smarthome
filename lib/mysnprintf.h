#pragma once

template <size_t N>
char* mysnprintf(char (&buffer)[N], const char* format, ...)
{
    va_list vl;
    va_start(vl, format);
    vsnprintf(buffer, N, format, vl);
    va_end(vl);

    return buffer;
}
