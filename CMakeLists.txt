cmake_minimum_required(VERSION 3.13)
project(GraphReconstruction)
set(root_dir ${PROJECT_SOURCE_DIR})

add_subdirectory(src)
add_subdirectory(checker)
# enable_testing()
# add_test(NAME example_test COMMAND test/example_test/run)

execute_process(COMMAND git rev-parse HEAD OUTPUT_VARIABLE sha OUTPUT_STRIP_TRAILING_WHITESPACE)
set(run ${CMAKE_CURRENT_BINARY_DIR}/src/run)
set(check ${CMAKE_CURRENT_BINARY_DIR}/checker/check)
set(in_dir ${root_dir}/data)
set(out_dir ${root_dir}/result/${sha})
set(submit_file ${PROJECT_NAME}.cpp)
add_custom_target(example-test ${root_dir}/scripts/run.sh ${run} ${in_dir} ${out_dir} ${check})
add_custom_target(submit cp ${root_dir}/src/main.cpp ${submit_file}
  COMMAND zip submit.zip ${submit_file}
  COMMAND mv submit.zip ${out_dir}
  COMMAND ls ${out_dir}/submit.zip
)