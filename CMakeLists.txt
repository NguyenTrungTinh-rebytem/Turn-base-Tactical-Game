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
find_package(raylib 4.0 REQUIRED)

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
