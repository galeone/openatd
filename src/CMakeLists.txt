cmake_minimum_required (VERSION 3.1)

# all files .cc in . and its subfolders.
# define variable CURRENT_SRC
file(GLOB_RECURSE CURRENT_SRC "*.cc")

# Find threads to link next in target_link_libraries
find_package(Threads REQUIRED)
find_package(Sqlite3 REQUIRED)

add_executable (openatd "${CURRENT_SRC}")
include_directories (
    # OpenAT
    ${OPENAT_INCLUDE_DIR}
    # OpenAT deps
    ${OPENAT_REQUIRED_INCLUDES}
    # Openatd deps
    ${SPDLOG_INCLUDE_DIR}
    ${SQLITE3_INCLUDE_DIRS}
    ${SQLITECPP_INCLUDE_DIR}
    # current /include
    ${OPENATD_INCLUDE_DIR}
)

target_link_libraries (openatd LINK_PUBLIC
    openat
    pthread
    SQLiteCpp
    ${SQLITE3_LIBRARIES}
)
