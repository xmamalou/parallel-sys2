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

#ifndef _TYPES_HXX_
#define _TYPES_HXX_

#include <string_view>
#include <variant>
#include <vector>

#include <stdint.h>

// --- TYPES --- //

namespace utility {
enum class Exercise : uint32_t {
  HELP = 0,
  PI_MONTE_CARLO = 1,
  GAME_OF_LIFE = 2,
  GAME_OF_LIFE_ASYNC = 3,
  GAME_OF_LIFE_MIX = 4,
};

// OPTIONS

template <Exercise which> struct ExerciseOptions;

template <> struct ExerciseOptions<Exercise::PI_MONTE_CARLO> {
  uint64_t throws{1};
};

template <> struct ExerciseOptions<Exercise::GAME_OF_LIFE> {
  uint64_t generations{1};
  uint32_t dims[2]{10, 10}; // rows, columns
};

template <> struct ExerciseOptions<Exercise::GAME_OF_LIFE_ASYNC> {
  uint64_t generations{1};
  uint32_t dims[2]{10, 10}; // rows, columns
};

template <> struct ExerciseOptions<Exercise::GAME_OF_LIFE_MIX> {
  uint64_t generations{1};
  uint32_t dims[2]{10, 10}; // rows, columns
  uint32_t jobs{1};         // number of jobs (threads) per node
};

using ExeOpts = std::variant<ExerciseOptions<Exercise::PI_MONTE_CARLO>,
                             ExerciseOptions<Exercise::GAME_OF_LIFE>,
                             ExerciseOptions<Exercise::GAME_OF_LIFE_ASYNC>,
                             ExerciseOptions<Exercise::GAME_OF_LIFE_MIX>>;

// RETURNS

template <Exercise which> struct ExerciseReturn;

template <> struct ExerciseReturn<Exercise::HELP> {};

template <> struct ExerciseReturn<Exercise::PI_MONTE_CARLO> {
  double pi{3.1415};
  uint64_t time{0}; // in msecs
};

template <> struct ExerciseReturn<Exercise::GAME_OF_LIFE> {
  uint64_t time{0};
  uint32_t dims[2]{10, 10};
};

template <> struct ExerciseReturn<Exercise::GAME_OF_LIFE_MIX> {
  uint64_t time{0};
  uint32_t dims[2]{10, 10};
};

template <> struct ExerciseReturn<Exercise::GAME_OF_LIFE_ASYNC> {
  uint64_t time{0};
  uint32_t dims[2]{10, 10};
};

using ExerciseVariantReturn =
    std::variant<ExerciseReturn<Exercise::PI_MONTE_CARLO>,
                 ExerciseReturn<Exercise::GAME_OF_LIFE>,
                 ExerciseReturn<Exercise::GAME_OF_LIFE_ASYNC>,
                 ExerciseReturn<Exercise::GAME_OF_LIFE_MIX>,
                 ExerciseReturn<Exercise::HELP>>;

// GLOBAL OPTIONS

struct Options {
  void *command_p{nullptr};
  std::string_view appname{};
  std::string_view filepath{"./out.txt"};
  bool do_append{true};
  //
  Exercise exe{Exercise::HELP};
  int32_t nodes{1};
  ExeOpts specifics{ExeOpts()};
};

struct Error {
  // --- TYPES --- //
  enum class ErrorCode : int {
    SHOW_HELP = 0, // not an error, the user just asked for help
    WRONG_COMMAND_ERR = 1,
    WRONG_OPTION_ERR = 2,
    NO_VALUE_ERR = 3,
    BAD_VALUE_ERR = 4,
    BAD_DUMPING_FAC_ERR = 5,
    OUT_OF_BOUNDS_ERR = 6,
    WRONG_DIMS_ERR = 7,
  };
  // --- FIELDS --- //
  std::string_view erroneous{};
  ErrorCode error{};
};

using Command = ExerciseVariantReturn (*)(const Options &);

template <typename T> class Matrix {
  std::vector<T> vector;
  const uint32_t dims[2]{2, 2};

public:
  enum class Dim : uint32_t {
    X = 0,
    Y = 1,
  };

  Matrix(const uint32_t x, const uint32_t y)
      : vector(std::vector<T>(x * y)), dims{x, y} {}

  auto operator()(const uint32_t i, const uint32_t j) -> T & {
    return this->vector[i + this->dims[0] * j];
  }

  template <Dim dim> auto size() const noexcept -> uint32_t {
    return this->dims[static_cast<uint32_t>(dim)];
  }

  auto data() const noexcept { return this->vector.data(); }
};

} // namespace utility

#endif /* _TYPES_HXX */
