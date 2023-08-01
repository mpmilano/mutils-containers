cmake_minimum_required (VERSION 3.15)
project (mutils-containers)
include(GNUInstallDirs)

#Versions
set(mutils_containers_VERSION 1.0)

#CXX FLAGS
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_SHARED_LINKER_FLAGS "-shared --enable-new-dtags")

if ( NOT DEFINED CMAKE_INSTALL_LIBDIR )
    set( CMAKE_INSTALL_LIBDIR lib )
endif ( )

#Test dependencies
find_package(mutils 1.0 REQUIRED)

add_library(mutils-containers INTERFACE)

#Testers
include_directories(${mutils-containers_SOURCE_DIR}/include)
add_executable(test_buffer_generator test_buffer_generator.cpp)
add_executable(test_multitype_map test_multitype_map.cpp)
add_executable(test_type_map test_type_map.cpp)
target_link_libraries(test_multitype_map mutils::mutils)

#Make install
install(TARGETS mutils-containers
        EXPORT mutils-containersTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY include/mutils-containers
        DESTINATION include)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/mutils-containersConfigVersion.cmake"
    VERSION ${mutils_containers_VERSION}
    COMPATIBILITY AnyNewerVersion
)

set(ConfigPackageLocation ${CMAKE_INSTALL_LIBDIR}/cmake/mutils-containers)
configure_package_config_file(mutils-containersConfig.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/mutils-containersConfig.cmake"
    INSTALL_DESTINATION ${ConfigPackageLocation}
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

install(EXPORT mutils-containersTargets
    FILE mutils-containersTargets.cmake
    NAMESPACE mutils::
    DESTINATION ${ConfigPackageLocation}
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/mutils-containersConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/mutils-containersConfigVersion.cmake"
    DESTINATION ${ConfigPackageLocation}
)
