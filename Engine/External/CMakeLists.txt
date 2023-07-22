set(CMAKE_FOLDER  "Raphael/Engine/External")

# logger
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/logger-cpp EXCLUDE_FROM_ALL)

# magic_enum
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/magic_enum EXCLUDE_FROM_ALL)

# glm
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm EXCLUDE_FROM_ALL)

# sdl
set(SDL_TEST_ENABLED_BY_DEFAULT
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_OPENGL
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_OPENGLES
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_OSS
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_ALSA
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_ALSA_SHARED
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_JACK
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_JACK_SHARED
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_PIPEWIRE
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_PIPEWIRE_SHARED
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_PULSEAUDIO
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_PULSEAUDIO_SHARED
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_FILE
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_FILESYSTEM
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_ATOMIC
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_RENDER
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_SHARED
    OFF
    CACHE BOOL "" FORCE
)
set(SDL_STATIC
    ON
    CACHE BOOL "" FORCE
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SDL EXCLUDE_FROM_ALL)

# xdgpp
add_library(xdgpp INTERFACE EXCLUDE_FROM_ALL)
target_include_directories(xdgpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/xdgpp)
target_compile_features(xdgpp INTERFACE cxx_std_17)

set(MI_OVERRIDE
    OFF
    CACHE BOOL "" FORCE
)
set(MI_BUILD_TESTS
    OFF
    CACHE INTERNAL "" FORCE
)
set(MI_BUILD_OBJECT
    OFF
    CACHE INTERNAL "" FORCE
)
set(MI_BUILD_SHARED
    OFF
    CACHE INTERNAL "" FORCE
)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mimalloc)
