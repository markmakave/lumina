/* 
    Copyright (c) 2023 Mokhov Mark

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <cuda_runtime.h>

#include "cuda/matrix.cuh"
#include "cuda/brief.cuh"
#include "base/color.hpp"

namespace lumina {
namespace cuda {

__global__
void
descript(
    const matrix<gray>                   image, 
    const matrix<bool>                   features,
    const brief<256>                     engine,
          matrix<brief<256>::descriptor> descriptors
) {
    const unsigned x = threadIdx.x + blockIdx.x * blockDim.x;
    const unsigned y = threadIdx.y + blockIdx.y * blockDim.y;

    if (x >= image.shape()[0] - 3 || y >= image.shape()[1] - 3 || x < 3 || y < 3)
        return;

    if (features(x, y))
        descriptors(x, y) = engine.descript(x, y, image);
}

}
}
