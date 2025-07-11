

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
