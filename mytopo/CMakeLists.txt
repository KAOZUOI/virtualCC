project(tutorial-app VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

set(SOURCE_FILES
    nvdgx2n.cc
    allreduce_app.cc
    rdma_app.cc
)

build_exec(
    EXECNAME dgx
    SOURCE_FILES "${SOURCE_FILES}"
    # LIBRARIES_TO_LINK "${ns3-libs}" "${ns3-contrib-libs}"

    LIBRARIES_TO_LINK
        ${libcore}
        ${libpoint-to-point}
        ${libinternet}
        ${libapplications}
    EXECUTABLE_DIRECTORY_PATH ${CMAKE_CURRENT_BINARY_DIR}/
)
