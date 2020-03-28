cmake_minimum_required (VERSION 3.1)

# all files .cc in . and its subfolders.
# define variable CURRENT_SRC
file(GLOB_RECURSE CURRENT_SRC "*.cc")

# Find threads to link next in target_link_libraries
find_package(Threads REQUIRED)
find_package(Sqlite3 REQUIRED)
# sdplog
find_package(spdlog REQUIRED)
# JSON
find_package(nlohmann_json REQUIRED)

add_executable (openatd "${CURRENT_SRC}")
target_include_directories (openatd PRIVATE
    ${SQLITE3_INCLUDE_DIRS}
    ${SQLITECPP_INCLUDE_DIR}
    # current /include
    ${OPENATD_INCLUDE_DIR}
)

target_link_libraries (openatd PRIVATE
    openat
    Threads::Threads
    SQLiteCpp
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    ${SQLITE3_LIBRARIES}
)
