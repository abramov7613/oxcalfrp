# generate sqlite database cmd file
set(PROCESS_DB_CREATE false)
foreach(F IN LISTS SQLITE3_DB_SOURCES)
  if(NOT EXISTS ${F})
    message(FATAL_ERROR "'${F}' file NOT FOUND")
  endif()
  if(${F} IS_NEWER_THAN ${SQLITE3_DB_FILE})
    set(PROCESS_DB_CREATE true)
  endif()
endforeach()
if( ${OXC_H_FILE} IS_NEWER_THAN ${SQLITE3_DB_FILE} )
  set(PROCESS_DB_CREATE true)
endif()
if(PROCESS_DB_CREATE)
  message(STATUS "Generate sqlite database cmd file")
  cmake_path(GET SQLITE3_DB_FILE STEM F_NAME)
  set(DATABASE_TMP_FILE "${RESOURCES_INPUT_DIR}/${F_NAME}.sql")
  if(EXISTS ${DATABASE_TMP_FILE})
    file(REMOVE ${DATABASE_TMP_FILE})
  endif()
  if(EXISTS ${SQLITE3_DB_FILE})
    file(REMOVE ${SQLITE3_DB_FILE})
  endif()
  if(NOT EXISTS ${OXC_H_FILE})
    message(FATAL_ERROR "'${OXC_H_FILE}' file NOT FOUND")
  endif()
  execute_process(
    COMMAND
      ${SQL_GENERATOR_EXE}
      ${DATABASE_TMP_FILE}
      ${SQLITE3_DB_SOURCES}
    COMMAND_ERROR_IS_FATAL ANY
  )
  file(STRINGS ${OXC_H_FILE} LINES)
  foreach(LINE IN LISTS LINES)
    if(LINE MATCHES "^oxc_const[ \t]+([A-Za-z0-9_]+)[ \t]*=[ \t]*([0-9]+).*$")
      string(APPEND STR_BUF "INSERT INTO oxc_consts VALUES(${CMAKE_MATCH_2}, \"${CMAKE_MATCH_1}\");\n")
    endif()
  endforeach()
  file(APPEND ${DATABASE_TMP_FILE}
      "CREATE TABLE oxc_consts (const_id INT PRIMARY KEY NOT NULL, const_name TEXT NOT NULL);\n")
  file(APPEND ${DATABASE_TMP_FILE} ${STR_BUF})
  execute_process(
    COMMAND
      ${WXZIPPER_EXE}
      "-i${DATABASE_TMP_FILE}"
      "-o${SQLITE3_DB_FILE}"
    COMMAND_ERROR_IS_FATAL ANY
  )
  file(REMOVE ${DATABASE_TMP_FILE})
endif()

# generate & add resource headers
set(PROCESS_RESOURCES false)
set(RESOURCES_HEADERS_DIR "${CMAKE_BINARY_DIR}/embedded_resources")
if( (NOT EXISTS "${CMAKE_BINARY_DIR}/resource.hpp") OR
      (NOT EXISTS "${CMAKE_BINARY_DIR}/resource_holder.hpp") )
  set(PROCESS_RESOURCES true)
else()
  file(GLOB RESOURCES_INPUT_FILES_LIST LIST_DIRECTORIES false "${RESOURCES_INPUT_DIR}/*")
  foreach(F IN LISTS RESOURCES_INPUT_FILES_LIST)
    execute_process(
      COMMAND
        ${RESOURCE_HEADER_NAME_GEN_EXE}
        ${F}
      OUTPUT_VARIABLE HDR_NAME
      COMMAND_ERROR_IS_FATAL ANY
    )
    string(STRIP ${HDR_NAME} HDR_NAME)
    set(HDR_FILE "${RESOURCES_HEADERS_DIR}/${HDR_NAME}")
    if(${F} IS_NEWER_THAN ${HDR_FILE})
      set(PROCESS_RESOURCES true)
      break()
    endif()
  endforeach()
endif()
if(PROCESS_RESOURCES)
  file(REMOVE_RECURSE ${RESOURCES_HEADERS_DIR})
  file(REMOVE "${CMAKE_BINARY_DIR}/resource.hpp")
  file(REMOVE "${CMAKE_BINARY_DIR}/resource_holder.hpp")
  message(STATUS "Generate header files for resources")
  execute_process(
    COMMAND
      ${MKEMBED_EXE}
      ${RESOURCES_INPUT_DIR}
      "-o"
      ${CMAKE_BINARY_DIR}
    COMMAND_ERROR_IS_FATAL ANY
  )
endif()
