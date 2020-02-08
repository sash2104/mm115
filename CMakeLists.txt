cmake_minimum_required(VERSION 3.13)
project(marathon)
set(root_dir ${PROJECT_SOURCE_DIR})

add_subdirectory(src)
# enable_testing()
# add_test(NAME example_test COMMAND test/example_test/run)

execute_process(COMMAND git rev-parse HEAD OUTPUT_VARIABLE sha)
set(exe ${CMAKE_CURRENT_BINARY_DIR}/src/run)
set(in_dir ${root_dir}/data)
set(out_dir ${root_dir}/result/${sha})
add_custom_target(example-test ${root_dir}/scripts/run.sh ${exe} ${in_dir} ${out_dir})