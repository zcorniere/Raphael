# logger
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logger-cpp)

# magic_enum
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/magic_enum)

# glm
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm)

# sdl
set(SDL_TEST_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL ""
)
set(SDL_OPENGL
    OFF
    CACHE BOOL ""
)
set(SDL_OPENGLES
    OFF
    CACHE BOOL ""
)
set(SDL_OSS
    OFF
    CACHE BOOL ""
)
set(SDL_ALSA
    OFF
    CACHE BOOL ""
)
set(SDL_ALSA_SHARED
    OFF
    CACHE BOOL ""
)
set(SDL_JACK
    OFF
    CACHE BOOL ""
)
set(SDL_JACK_SHARED
    OFF
    CACHE BOOL ""
)
set(SDL_PIPEWIRE
    OFF
    CACHE BOOL ""
)
set(SDL_PIPEWIRE_SHARED
    OFF
    CACHE BOOL ""
)
set(SDL_PULSEAUDIO
    OFF
    CACHE BOOL ""
)
set(SDL_PULSEAUDIO_SHARED
    OFF
    CACHE BOOL ""
)
set(SDL_FILE
    OFF
    CACHE BOOL ""
)
set(SDL_FILESYSTEM
    OFF
    CACHE BOOL ""
)
set(SDL_ATOMIC
    OFF
    CACHE BOOL ""
)
set(SDL_RENDER
    OFF
    CACHE BOOL ""
)
set(SDL_SHARED
    OFF
    CACHE BOOL ""
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SDL)

# tomlplusplus
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tomlplusplus ${CMAKE_CURRENT_BINARY_DIR}/tomlplusplus)

#xdgpp
add_library(xdgpp INTERFACE)
target_include_directories(xdgpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/xdgpp)
target_compile_features(xdgpp INTERFACE cxx_std_17)

