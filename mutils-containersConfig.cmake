@PACKAGE_INIT@

set_and_check(mutils-containers_INCLUDE_DIRS "@PACKAGE_CMAKE_INSTALL_INCLUDEDIR@")
include("${CMAKE_CURRENT_LIST_DIR}/mutils-containersTargets.cmake")

check_required_components(mutils-containers)