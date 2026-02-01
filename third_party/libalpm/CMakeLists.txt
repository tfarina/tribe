set(TARGETNAME alpm)

set(SOURCES
  alpm_list.c
  alpm_list.h
)

add_library(${TARGETNAME}
  ${SOURCES}
)
