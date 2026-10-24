cmake_minimum_required(VERSION 3.16)
project(TurnBaseTacticalGame CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Thu nhặt toàn bộ file mã nguồn (.cpp)
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

# 2. Bổ sung các thư mục Include để giải quyết lỗi #include
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/data
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dsa
    ${CMAKE_CURRENT_SOURCE_DIR}/src/entities
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics
)

# 3. Thư viện đồ họa Raylib (Bắt buộc cho dự án này)
set(RAYLIB_ROOT "" CACHE PATH "Thu muc goc raylib (chua include/ va lib/)")

find_path(RAYLIB_INCLUDE_DIR raylib.h PATHS "${RAYLIB_ROOT}/include" REQUIRED)
find_library(RAYLIB_LIBRARY NAMES raylib raylib.lib PATHS "${RAYLIB_ROOT}/lib" REQUIRED)

if(NOT TARGET raylib)
    add_library(raylib STATIC IMPORTED)
    set_target_properties(raylib PROPERTIES
        IMPORTED_LOCATION "${RAYLIB_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR}")
    if(WIN32)
        set_property(TARGET raylib APPEND PROPERTY INTERFACE_LINK_LIBRARIES winmm gdi32 opengl32)
    endif()
endif()
# 4. Tạo file thực thi
add_executable(game ${SOURCES})
target_link_libraries(game PRIVATE raylib)


# 5. Copy data/ và assets/ cạnh game.exe sau mỗi lần build
#    (game cũng tự tìm ở thư mục chạy hiện tại và ../ so với exe)
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/data $<TARGET_FILE_DIR:game>/data
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:game>/assets
)


# 6. Test T08 (Factory/Party/Item/Inventory) - KHONG can Raylib, khong nam trong build mac dinh
#    Chay: cmake --build build --target test_entities && ctest --test-dir build --output-on-failure
enable_testing()
file(GLOB ENTITY_SOURCES "src/entities/*.cpp")
add_executable(test_entities EXCLUDE_FROM_ALL
    tests/test_entities.cpp
    ${ENTITY_SOURCES}
    src/data/GameData.cpp
)
add_test(NAME test_entities COMMAND test_entities ${CMAKE_CURRENT_SOURCE_DIR}/data)
