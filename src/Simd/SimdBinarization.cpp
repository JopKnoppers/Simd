/*
* Simd Library.
*
* Copyright (c) 2011-2013 Yermalayeu Ihar.
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
#include "Simd/SimdEnable.h"
#include "Simd/SimdMemory.h"
#include "Simd/SimdLoad.h"
#include "Simd/SimdStore.h"
#include "Simd/SimdConst.h"
#include "Simd/SimdMath.h"
#include "Simd/SimdBinarization.h"

namespace Simd
{
	namespace Base
	{
        template <CompareType type> SIMD_INLINE bool Compare(const uchar & src, const uchar & b);

        template <> SIMD_INLINE bool Compare<CompareGreaterThen>(const uchar & a, const uchar & b)
        {
            return a > b;
        }

        template <> SIMD_INLINE bool Compare<CompareLesserThen>(const uchar & a, const uchar & b)
        {
            return a < b;
        }

        template <> SIMD_INLINE bool Compare<CompareEqualTo>(const uchar & a, const uchar & b)
        {
            return a == b;
        }

        template <CompareType compareType> 
		void Binarization(const uchar * src, size_t srcStride, size_t width, size_t height, 
			uchar value, uchar positive, uchar negative, uchar * dst, size_t dstStride)
		{
			for(size_t row = 0; row < height; ++row)
			{
				for(size_t col = 0; col < width; ++col)
					dst[col] = Compare<compareType>(src[col], value) ? positive : negative;
				src += srcStride;
				dst += dstStride;
			}
		}

        void Binarization(const uchar * src, size_t srcStride, size_t width, size_t height, 
            uchar value, uchar positive, uchar negative, uchar * dst, size_t dstStride, CompareType compareType)
        {
            switch(compareType)
            {
            case CompareGreaterThen:
                return Binarization<CompareGreaterThen>(src, srcStride, width, height, value, positive, negative, dst, dstStride);
            case CompareLesserThen:
                return Binarization<CompareLesserThen>(src, srcStride, width, height, value, positive, negative, dst, dstStride);
            case CompareEqualTo:
                return Binarization<CompareEqualTo>(src, srcStride, width, height, value, positive, negative, dst, dstStride);
            default:
                assert(0);
            }
        }
	}

#ifdef SIMD_SSE2_ENABLE    
	namespace Sse2
	{
        template<CompareType compareType> SIMD_INLINE __m128i Compare(__m128i a, __m128i b);

        template<> SIMD_INLINE __m128i Compare<CompareGreaterThen>(__m128i a, __m128i b)
        {
            return _mm_andnot_si128(_mm_cmpeq_epi8(_mm_min_epu8(a, b), a), K_INV_ZERO);
        }

        template<> SIMD_INLINE __m128i Compare<CompareLesserThen>(__m128i a, __m128i b)
        {
            return _mm_andnot_si128(_mm_cmpeq_epi8(_mm_max_epu8(a, b), a), K_INV_ZERO);
        }

        template<> SIMD_INLINE __m128i Compare<CompareEqualTo>(__m128i a, __m128i b)
        {
            return _mm_cmpeq_epi8(a, b);
        }

        SIMD_INLINE __m128i Combine(__m128i mask, __m128i positive, __m128i negative)
        {
            return _mm_or_si128(_mm_and_si128(mask, positive), _mm_andnot_si128(mask, negative));
        }

        template <bool align, CompareType compareType> 
        void Binarization(const uchar * src, size_t srcStride, size_t width, size_t height, 
            uchar value, uchar positive, uchar negative, uchar * dst, size_t dstStride)
        {
            assert(width >= A);
            if(align)
                assert(Aligned(src) && Aligned(srcStride) && Aligned(dst) && Aligned(dstStride));

            size_t alignedWidth = Simd::AlignLo(width, A);

            __m128i value_ = _mm_set1_epi8(value);
            __m128i positive_ = _mm_set1_epi8(positive);
            __m128i negative_ = _mm_set1_epi8(negative);
            for(size_t row = 0; row < height; ++row)
            {
                for(size_t col = 0; col < alignedWidth; col += A)
                {
                    const __m128i mask = Compare<compareType>(Load<align>((__m128i*)(src + col)), value_);
                    Store<align>((__m128i*)(dst + col), Combine(mask, positive_, negative_));
                }
                if(alignedWidth != width)
                {
                    const __m128i mask = Compare<compareType>(Load<align>((__m128i*)(src + width - A)), value_);
                    Store<align>((__m128i*)(dst + width - A), Combine(mask, positive_, negative_));
                }
                src += srcStride;
                dst += dstStride;
            }
        }

        template <CompareType compareType> 
        void Binarization(const uchar * src, size_t srcStride, size_t width, size_t height, 
            uchar value, uchar positive, uchar negative, uchar * dst, size_t dstStride)
        {
            if(Aligned(src) && Aligned(srcStride) && Aligned(dst) && Aligned(dstStride))
                Binarization<true, compareType>(src, srcStride, width, height, value, positive, negative, dst, dstStride);
            else
                Binarization<false, compareType>(src, srcStride, width, height, value, positive, negative, dst, dstStride);
        }

        void Binarization(const uchar * src, size_t srcStride, size_t width, size_t height, 
            uchar value, uchar positive, uchar negative, uchar * dst, size_t dstStride, CompareType compareType)
        {
            switch(compareType)
            {
            case CompareGreaterThen:
                return Binarization<CompareGreaterThen>(src, srcStride, width, height, value, positive, negative, dst, dstStride);
            case CompareLesserThen:
                return Binarization<CompareLesserThen>(src, srcStride, width, height, value, positive, negative, dst, dstStride);
            case CompareEqualTo:
                return Binarization<CompareEqualTo>(src, srcStride, width, height, value, positive, negative, dst, dstStride);
            default:
                assert(0);
            }
        }
    }
#endif// SIMD_SSE2_ENABLE

	void Binarization(const uchar * src, size_t srcStride, size_t width, size_t height, 
		uchar value, uchar positive, uchar negative, uchar * dst, size_t dstStride, CompareType compareType)
	{
#ifdef SIMD_AVX2_ENABLE
        if(Avx2::Enable && width >= Avx2::A)
            Avx2::Binarization(src, srcStride, width, height, value, positive, negative, dst, dstStride, compareType);
        else
#endif// SIMD_AVX2_ENABLE
#ifdef SIMD_SSE2_ENABLE
		if(Sse2::Enable && width >= Sse2::A)
			Sse2::Binarization(src, srcStride, width, height, value, positive, negative, dst, dstStride, compareType);
		else
#endif// SIMD_SSE2_ENABLE
			Base::Binarization(src, srcStride, width, height, value, positive, negative, dst, dstStride, compareType);
	}
}