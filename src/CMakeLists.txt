
set(SOURCE_FILES 
  ${CMAKE_SOURCE_DIR}/src/main.cc
  ${CMAKE_SOURCE_DIR}/src/frate.cc
  ${CMAKE_SOURCE_DIR}/src/frate.h
  )
include_directories(${CMAKE_SOURCE_DIR}/src)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME})
