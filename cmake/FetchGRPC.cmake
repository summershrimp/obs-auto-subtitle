include(FetchContent)

set(FETCHCONTENT_QUIET OFF)
FetchContent_Declare(
  gRPC
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG        v1.51.1
  GIT_SHALLOW TRUE
  )

FetchContent_GetProperties(gRPC)

if(NOT grpc_POPULATED)
    FetchContent_Populate(gRPC)
    message(STATUS "${grpc_SOURCE_DIR} ${grpc_BINARY_DIR}")
    add_subdirectory(${grpc_SOURCE_DIR} ${grpc_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
