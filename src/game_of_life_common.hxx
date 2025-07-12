/*
    Parallel Systems Exercise Batch 2 -- Solutions to Batch 2 of Exercises for
   the Parallel Systems Course of the "Computer Engineering" Masters Programme
   of NKUA Copyright (C) 2025 Christoforos-Marios Mamaloukas

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _GAME_OF_LIFE_COMMON_HXX_
#define _GAME_OF_LIFE_COMMON_HXX_

#include "types.hxx"

// --- TYPES --- //

namespace gol {
using Dim = utility::Matrix<short>::Dim;

static auto check(utility::Matrix<short> &neighborhood, const uint32_t at_x,
                  const uint32_t at_y) -> bool {
    const auto right{at_x > 0 ? at_x - 1 : at_x};
    const auto top{at_y > 0 ? at_y - 1 : at_y};
    const auto left{at_x < neighborhood.size<Dim::X>() - 1 ? at_x + 1 : at_x};
    const auto bottom{at_y < neighborhood.size<Dim::Y>() - 1 ? at_y + 1 : at_y};

    uint32_t alive_neighborhoods{0};
    for (uint32_t i{right}; i <= left; i++) {
        for (uint32_t j{top}; j <= bottom; j++) {
            if (i == at_x || j == at_y)
                continue;

            if (neighborhood(i, j) == 1)
                alive_neighborhoods++;
        }
    }

    switch (alive_neighborhoods) {
    case 3:
        return true;
    case 2:
        return static_cast<bool>(neighborhood(at_x, at_y));
    default:
        return false;
    }
}
} // namespace gol

#endif /* _GAME_OF_LIFE_COMMON_HXX_ */
