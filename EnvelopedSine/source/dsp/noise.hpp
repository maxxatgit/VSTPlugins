// (c) 2019-2020 Takamitsu Endo
//
// This file is part of EnvelopedSine.
//
// EnvelopedSine is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EnvelopedSine is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EnvelopedSine.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "../../../lib/vcl/vectorclass.h"

#include <cstdint>

namespace SomeDSP {

// Numerical Recipes In C p.284.
struct alignas(64) White16 {
  Vec16ui buffer{0};

  White16(uint32_t seed) { setSeed(seed); }

  void setSeed(uint32_t seed)
  {
    for (int idx = 0; idx < 16; ++idx) {
      seed = 1664525L * seed + 1013904223L;
      buffer.insert(idx, seed);
    }
  }

  Vec16f process()
  {
    buffer = 1664525L * buffer + 1013904223L;
    return to_float(buffer) / float(UINT32_MAX);
  }
};

} // namespace SomeDSP
