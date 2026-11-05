# CMake generated Testfile for 
# Source directory: D:/KIET/devC/DSA/Turn-base-Tactical-Game
# Build directory: D:/KIET/devC/DSA/Turn-base-Tactical-Game/build-release
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test_entities "D:/KIET/devC/DSA/Turn-base-Tactical-Game/build-release/test_entities.exe" "D:/KIET/devC/DSA/Turn-base-Tactical-Game/data")
set_tests_properties(test_entities PROPERTIES  _BACKTRACE_TRIPLES "D:/KIET/devC/DSA/Turn-base-Tactical-Game/CMakeLists.txt;58;add_test;D:/KIET/devC/DSA/Turn-base-Tactical-Game/CMakeLists.txt;0;")
add_test(test_core_integration "D:/KIET/devC/DSA/Turn-base-Tactical-Game/build-release/test_core_integration.exe" "D:/KIET/devC/DSA/Turn-base-Tactical-Game/data")
set_tests_properties(test_core_integration PROPERTIES  _BACKTRACE_TRIPLES "D:/KIET/devC/DSA/Turn-base-Tactical-Game/CMakeLists.txt;69;add_test;D:/KIET/devC/DSA/Turn-base-Tactical-Game/CMakeLists.txt;0;")
add_test(test_boss_mouse "D:/KIET/devC/DSA/Turn-base-Tactical-Game/build-release/test_boss_mouse.exe")
set_tests_properties(test_boss_mouse PROPERTIES  _BACKTRACE_TRIPLES "D:/KIET/devC/DSA/Turn-base-Tactical-Game/CMakeLists.txt;79;add_test;D:/KIET/devC/DSA/Turn-base-Tactical-Game/CMakeLists.txt;0;")
add_test(test_battle_hud "D:/KIET/devC/DSA/Turn-base-Tactical-Game/build-release/test_battle_hud.exe")
set_tests_properties(test_battle_hud PROPERTIES  _BACKTRACE_TRIPLES "D:/KIET/devC/DSA/Turn-base-Tactical-Game/CMakeLists.txt;86;add_test;D:/KIET/devC/DSA/Turn-base-Tactical-Game/CMakeLists.txt;0;")
