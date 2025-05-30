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

  template<Exercise exe>
  struct SubOptions;

  template<>
  struct SubOptions<Exercise::HELP>;

  template<>
  struct SubOptions<Exercise::PI_MONTE_CARLO> {

  };

  struct Options {
    void*            command_p{ nullptr };
    std::string_view appname{};
    //
    Exercise         exe{ Exercise::HELP };
  };

  using Command = void (*)(const Options&);
}

#endif /* _TYPES_HXX */
