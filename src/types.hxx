#ifndef _TYPES_HXX_
#define _TYPES_HXX_

#include <string_view>

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

  struct Options {
    void*            command_p{ nullptr };
    std::string_view appname{};
    //
    Exercise         exe{ Exercise::HELP };
    int32_t          nodes{ 1 };
    // for pi_monte_carlo
    uint64_t         throws{ 1 };
  };

  struct Error {
    // --- TYPES --- //
    enum class ErrorCode : int {
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


  using Command = void (*)(const Options&);
}

#endif /* _TYPES_HXX */
