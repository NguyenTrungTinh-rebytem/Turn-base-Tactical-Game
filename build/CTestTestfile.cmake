# CMake generated Testfile for 
# Source directory: D:/Turn-base-Tactical-Game
# Build directory: D:/Turn-base-Tactical-Game/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_entities "D:/Turn-base-Tactical-Game/build/test_entities.exe" "D:/Turn-base-Tactical-Game/data")
set_tests_properties(test_entities PROPERTIES  _BACKTRACE_TRIPLES "D:/Turn-base-Tactical-Game/CMakeLists.txt;52;add_test;D:/Turn-base-Tactical-Game/CMakeLists.txt;0;")
add_test(test_core_integration "D:/Turn-base-Tactical-Game/build/test_core_integration.exe" "D:/Turn-base-Tactical-Game/data")
set_tests_properties(test_core_integration PROPERTIES  _BACKTRACE_TRIPLES "D:/Turn-base-Tactical-Game/CMakeLists.txt;63;add_test;D:/Turn-base-Tactical-Game/CMakeLists.txt;0;")
