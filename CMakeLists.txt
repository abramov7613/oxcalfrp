# This project uses Git tag system for version control.

cmake_minimum_required(VERSION 3.25)

# Find Git commit details to get project version from latest commit tag
find_package(Git REQUIRED)
execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_LATEST_TAG_STRING
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)
# Extract version number
if(GIT_LATEST_TAG_STRING MATCHES "v([0-9]+\.[0-9]+\.[0-9]+)")
  set(OXCALFRP_VERSION ${CMAKE_MATCH_1})
else()
  message(FATAL_ERROR "Can't get project version from Git. "
                      "Check latest commit tag string, so that it contains the necessary data")
endif()

# setup project options
project(oxcalfrp VERSION ${OXCALFRP_VERSION})
set(RESOURCES_FOLDER_NAME "resources")
set(EXTERN_PROJECTS_FOLDER_NAME "extern")
set(RESOURCES_INPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCES_FOLDER_NAME}")
set(SQLITE3_DB_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month01.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month02.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month03.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month04.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month05.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month06.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month07.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month08.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month09.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month10.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month11.txt"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/database_month12.txt"
)
string(REPLACE ";" "$<SEMICOLON>" SQLITE3_DB_SRCS "${SQLITE3_DB_SOURCES}")
set(SQLITE3_DB_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCES_FOLDER_NAME}/sqlite_db.zip")
set(OXCALFRP_SOURCES
    src/aboutdialog.cpp
    src/calendarpanel.cpp
    src/main.cpp
    src/mainframe.cpp
    src/monthchoicectrl.cpp
    src/numbertextctrl.cpp
    src/fs_mem_rc.cpp
    src/yearinputdialog.cpp
)

# =================================================================================
# enable CPM
# =================================================================================
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${EXTERN_PROJECTS_FOLDER_NAME}")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${EXTERN_PROJECTS_FOLDER_NAME}")
endif()
set(CPM_SOURCE_CACHE ${EXTERN_PROJECTS_FOLDER_NAME})
include(cmake/CPM.cmake)

# =================================================================================
# find wxWidgets library
# =================================================================================
if(CMAKE_HOST_WIN32)
  find_package(wxWidgets REQUIRED CONFIG)
else()
  find_package(wxWidgets COMPONENTS std webview REQUIRED)
endif()
set(wxWidgets_USE_DEBUG OFF)
set(wxWidgets_USE_STATIC ON)
if(wxWidgets_USE_FILE) # not defined in CONFIG mode
  include(${wxWidgets_USE_FILE})
endif()

# =================================================================================
# get sqlite3
# =================================================================================
CPMAddPackage(
    NAME sqlite3
    URL "https://sqlite.org/2025/sqlite-amalgamation-3490100.zip"
    URL_HASH SHA256=6cebd1d8403fc58c30e93939b246f3e6e58d0765a5cd50546f16c00fd805d2c3
    DOWNLOAD_ONLY TRUE
)
if(NOT sqlite3_ADDED)
  message(FATAL_ERROR "Can't get SQLite3 library")
endif()
add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c ${sqlite3_SOURCE_DIR}/sqlite3.h)
target_include_directories(sqlite3 PUBLIC $<BUILD_INTERFACE:${sqlite3_SOURCE_DIR}>)
target_compile_definitions(sqlite3 PRIVATE
    SQLITE_OMIT_LOAD_EXTENSION
    SQLITE_THREADSAFE=0
)

# =================================================================================
# get sqlite_modern_cpp
# =================================================================================
CPMAddPackage(
    NAME sqlite_modern_cpp
    URL "https://github.com/SqliteModernCpp/sqlite_modern_cpp/archive/6e3009973025e0016d5573529067714201338c80.zip"
    URL_HASH SHA256=1d98fde34e621892ce042347dda5b055760e223d61f6d42a5b94d0f21f610a6d
    DOWNLOAD_ONLY YES
)
if (NOT sqlite_modern_cpp_ADDED)
  message(FATAL_ERROR "Can't get sqlite_modern_cpp library")
endif()
add_library(sqlite_modern_cpp INTERFACE "${sqlite_modern_cpp_SOURCE_DIR}/hdr/sqlite_modern_cpp.h")
target_include_directories(sqlite_modern_cpp INTERFACE "${sqlite_modern_cpp_SOURCE_DIR}/hdr")

# =================================================================================
# get oxc library
# =================================================================================
CPMAddPackage(
  NAME oxc
  GIT_REPOSITORY "https://github.com/abramov7613/oxc.git"
  VERSION 1.0.0
)
if (NOT oxc_ADDED)
  message(FATAL_ERROR "Can't get oxc library")
endif()
set(OXC_H_FILE "${oxc_SOURCE_DIR}/oxc.h")

# =================================================================================
# get timestamp library
# =================================================================================
CPMAddPackage("gh:abramov7613/cmake_timestamp@1.0.3")
if (NOT cmake_timestamp_ADDED)
  message(FATAL_ERROR "Can't get cmake_timestamp library")
endif()

# =================================================================================
# define targets
# =================================================================================
add_executable(wxZipper src/wxZipper.cpp)
target_compile_features(wxZipper PRIVATE cxx_std_20)
target_link_libraries(wxZipper ${wxWidgets_LIBRARIES})
add_executable(mk_embed
    src/MKlimenko_embed.hpp
    src/MKlimenko_main.cpp
    src/MKlimenko_resource.hpp
    src/MKlimenko_saver.hpp
)
target_compile_features(mk_embed PRIVATE cxx_std_20)
add_executable(resource_header_name_gen src/resource_header_name_gen.cpp)
target_compile_features(resource_header_name_gen PRIVATE cxx_std_20)
add_executable(sql_generator src/sql_generator.cpp)
target_compile_features(sql_generator PRIVATE cxx_std_20)
add_custom_target(project_resources ALL
  COMMAND
    ${CMAKE_COMMAND}
      -D OXC_H_FILE=${OXC_H_FILE}
      -D RESOURCES_INPUT_DIR=${RESOURCES_INPUT_DIR}
      -D SQLITE3_DB_FILE=${SQLITE3_DB_FILE}
      -D SQLITE3_DB_SOURCES=${SQLITE3_DB_SRCS}
      -D WXZIPPER_EXE=$<TARGET_FILE:wxZipper>
      -D MKEMBED_EXE=$<TARGET_FILE:mk_embed>
      -D SQL_GENERATOR_EXE=$<TARGET_FILE:sql_generator>
      -D RESOURCE_HEADER_NAME_GEN_EXE=$<TARGET_FILE:resource_header_name_gen>
      -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_resources.cmake"
  DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_resources.cmake"
  VERBATIM
)
add_dependencies(project_resources
    wxZipper
    mk_embed
    resource_header_name_gen
    sql_generator
)
add_executable(${PROJECT_NAME} WIN32 ${OXCALFRP_SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(${PROJECT_NAME}
    ${wxWidgets_LIBRARIES}
    oxc
    sqlite3
    sqlite_modern_cpp
    cmake_timestamp
)
add_dependencies(${PROJECT_NAME} project_resources)

# =================================================================================
# install & pack
# =================================================================================

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME})

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
  set(CPACK_PACKAGE_VENDOR "Vladimir Abramov")
  set(CPACK_PACKAGE_CONTACT "abramov7613@yandex.ru")
  set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
  set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
  set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
  set(CPACK_PACKAGE_DESCRIPTION "Православный церковный календарь для исследовательских целей - это приложение с открытым исходным кодом для настольных ПК под управлением ОС Windows или Linux. ")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Православный церковный календарь диц")
  set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/LICENSE)
  set(CPACK_RESOURCE_FILE_README  ${CMAKE_CURRENT_LIST_DIR}/README.md)
  set(CPACK_VERBATIM_VARIABLES YES)
  if(WIN32)
    set(CPACK_GENERATOR ZIP)
  elseif(APPLE)
    set(CPACK_GENERATOR productbuild)
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CPACK_GENERATOR DEB)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
  else()
    set(CPACK_GENERATOR TXZ)
  endif()
  include(CPack)
endif()
