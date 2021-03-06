/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2017 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Simd/SimdMemory.h"
#include "Simd/SimdImageSave.h"

#include <stdio.h>

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable: 4996)
#endif

namespace Simd
{
    namespace Base
    {
        uint8_t* ImageSaveToMemory(const uint8_t* data, size_t stride, size_t width, size_t height, SimdPixelFormatType format, SimdImageFileType file, int quality, size_t* size)
        {
            return NULL;
        }

        SimdBool ImageSaveToFile(const ImageSaveToMemoryPtr saver, const uint8_t* data, size_t stride, size_t width, size_t height, SimdPixelFormatType format, SimdImageFileType file, int quality, const char* path)
        {
            SimdBool result = SimdFalse;
            size_t size;
            uint8_t * buffer = saver(data, stride, width, height, format, file, quality, &size);
            if (buffer)
            {
                ::FILE* file = ::fopen(path, "wb");
                if (file)
                {
                    if (::fwrite(buffer, 1, size, file) == size)
                        result = SimdTrue;
                    ::fclose(file);
                }
                Simd::Free(buffer);
            }
            return result;
        }
    }
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif
