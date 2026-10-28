# CMake generated Testfile for 
# Source directory: D:/CODE/Turn-base-Tactical-Game
# Build directory: D:/CODE/Turn-base-Tactical-Game/build-release
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_entities "D:/CODE/Turn-base-Tactical-Game/build-release/test_entities.exe" "D:/CODE/Turn-base-Tactical-Game/data")
set_tests_properties(test_entities PROPERTIES  _BACKTRACE_TRIPLES "D:/CODE/Turn-base-Tactical-Game/CMakeLists.txt;52;add_test;D:/CODE/Turn-base-Tactical-Game/CMakeLists.txt;0;")
add_test(test_core_integration "D:/CODE/Turn-base-Tactical-Game/build-release/test_core_integration.exe" "D:/CODE/Turn-base-Tactical-Game/data")
set_tests_properties(test_core_integration PROPERTIES  _BACKTRACE_TRIPLES "D:/CODE/Turn-base-Tactical-Game/CMakeLists.txt;63;add_test;D:/CODE/Turn-base-Tactical-Game/CMakeLists.txt;0;")
