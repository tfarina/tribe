set(TARGETNAME ab)

set(DEPENDENCIES
  sqlite
)

set(SOURCES
  ab.c
  ab.h
  ab_contact.c
  ab_contact.h
  dirs.c
  dirs.h
  util.c
  util.h
)

add_library(${TARGETNAME}
  ${SOURCES}
)

add_dependencies(${TARGETNAME}
  ${DEPENDENCIES}
)

target_include_directories(${TARGETNAME} PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${GLIB_INCLUDE_DIRS}
)

target_link_libraries(${TARGETNAME}
  ${DEPENDENCIES}
  ${GLIB_LDFLAGS}
)

add_subdirectory(cli)

#
# Tribe binary
#
set(TARGETNAME "${PROJECT_NAME}")

set(DEPENDENCIES
  ab
  sqlite
)

set(SOURCES
  about_dialog.c
  contact_editor.c
  main.c
  tribe-application.c
  window.c
)

add_executable(${TARGETNAME}
  ${SOURCES}
)

add_dependencies(${TARGETNAME}
  ${DEPENDENCIES}
)

target_compile_options(${TARGETNAME} PUBLIC
  ${GTK_CFLAGS}
)

target_include_directories(${TARGETNAME} PUBLIC
  ${CMAKE_BINARY_DIR}
  ${GTK_INCLUDE_DIRS}
)

target_link_libraries(${TARGETNAME}
  ${DEPENDENCIES}
  ${GTK_LDFLAGS}
)

install(TARGETS ${TARGETNAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
