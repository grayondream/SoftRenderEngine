# CMake generated Testfile for 
# Source directory: /home/ares/workspace/SoftGameEngine/tools
# Build directory: /home/ares/workspace/SoftGameEngine/build-asan/tools
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[golden_image]=] "/home/ares/workspace/SoftGameEngine/scripts/check_golden.sh" "/home/ares/workspace/SoftGameEngine/build-asan/tools/golden_render" "/home/ares/workspace/SoftGameEngine/assets/golden/lit_cube.md5")
set_tests_properties([=[golden_image]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/ares/workspace/SoftGameEngine/tools/CMakeLists.txt;10;add_test;/home/ares/workspace/SoftGameEngine/tools/CMakeLists.txt;0;")
