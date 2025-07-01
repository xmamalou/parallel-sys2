#ifndef _TYPES_HXX_
#define _TYPES_HXX_

#include <string_view>
#include <variant>

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

  template<Exercise which>
  struct ExerciseOptions;

  template<>
  struct ExerciseOptions<Exercise::PI_MONTE_CARLO> {
    uint64_t throws{ 1 };
  };

  template<>
  struct ExerciseOptions<Exercise::GAME_OF_LIFE> {
    uint64_t generations{ 1 };
    uint32_t dims[2]{ 10, 10 }; // rows, columns
  };

  template<>
  struct ExerciseOptions<Exercise::GAME_OF_LIFE_ASYNC> {
    uint64_t generations{ 1 };
    uint32_t dims[2]{ 10, 10 }; // rows, columns
  };

  template<>
  struct ExerciseOptions<Exercise::GAME_OF_LIFE_MIX> {
    uint64_t generations{ 1 };
    uint32_t dims[2]{ 10, 10 }; // rows, columns
    uint32_t jobs{ 1 }; // number of jobs (threads) per node
  };

  using ExeOpts = std::variant<
    ExerciseOptions<Exercise::PI_MONTE_CARLO>,
    ExerciseOptions<Exercise::GAME_OF_LIFE>,
    ExerciseOptions<Exercise::GAME_OF_LIFE_ASYNC>,
    ExerciseOptions<Exercise::GAME_OF_LIFE_MIX>
  >;

  // RETURNS
  
  template<Exercise which>
  struct ExerciseReturn;

  template<>
  struct ExerciseReturn<Exercise::HELP> {};

  template<>
  struct ExerciseReturn<Exercise::PI_MONTE_CARLO> {
    double pi{ 3.1415 };
    double time{ 0.0 }; // in msecs
  };

  template<>
  struct ExerciseReturn<Exercise::GAME_OF_LIFE> {};

  template<>
  struct ExerciseReturn<Exercise::GAME_OF_LIFE_MIX> {};

  template<>
  struct ExerciseReturn<Exercise::GAME_OF_LIFE_ASYNC> {};

  using ExerciseVariantReturn = std::variant<
    ExerciseReturn<Exercise::PI_MONTE_CARLO>,
    ExerciseReturn<Exercise::GAME_OF_LIFE>,
    ExerciseReturn<Exercise::GAME_OF_LIFE_ASYNC>,
    ExerciseReturn<Exercise::GAME_OF_LIFE_MIX>,
    ExerciseReturn<Exercise::HELP>
  >;

  // GLOBAL OPTIONS

  struct Options {
    void*            command_p{ nullptr };
    std::string_view appname{};
    std::string_view filepath{ "./out.txt" };
    bool             do_append{ true };
    //
    Exercise         exe{ Exercise::HELP };
    int32_t          nodes{ 1 };
    ExeOpts          specifics{ ExeOpts() };
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
    ErrorCode        error{};
  };


  using Command = ExerciseVariantReturn (*)(const Options&);
}

#endif /* _TYPES_HXX */
