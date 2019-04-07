cmake_minimum_required (VERSION 3.1)
project (mutils-containers)

#Versions
set(mutils_containers_VERSION 1.0)

#CXX FLAGS
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++14 -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++14 -fPIC -g")
set(CMAKE_SHARED_LINKER_FLAGS "-shared --enable-new-dtags")

#Test dependencies
find_package(mutils 1.0)
if(NOT ${mutils_FOUND})
  message( FATAL_ERROR "mutils not found.")
endif(NOT ${mutils_FOUND})

add_library(mutils-containers INTERFACE)

#Testers
include_directories(${mutils-containers_SOURCE_DIR}/include)
add_executable(test_buffer_generator test_buffer_generator.cpp)
add_executable(test_multitype_map test_multitype_map.cpp)
add_executable(test_type_map test_type_map.cpp)
target_link_libraries(test_multitype_map ${mutils_LIBRARIES})

#Make install
install(DIRECTORY include/mutils-containers
  DESTINATION include)
install(TARGETS mutils-containers EXPORT mutils-containers
  LIBRARY DESTINATION lib)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/mutils-containers/mutils-containersConfigVersion.cmake"
  VERSION ${mutils_containers_VERSION}
  COMPATIBILITY AnyNewerVersion
)

export(EXPORT mutils-containers
  FILE "${CMAKE_CURRENT_BINARY_DIR}/mutils-containers/mutils-containersTargets.cmake"
)

configure_file(mutils-containersConfig.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/mutils-containers/mutils-containersConfig.cmake"
  COPYONLY
)

set(ConfigPackageLocation lib/cmake/mutils-containers)
install(EXPORT mutils-containers
  FILE mutils-containersTargets.cmake
  DESTINATION ${ConfigPackageLocation}
)

install(FILES
  mutils-containersConfig.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/mutils-containers/mutils-containersConfigVersion.cmake"
  DESTINATION ${ConfigPackageLocation}
)
