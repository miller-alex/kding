find_package(Gettext REQUIRED)

if (NOT GETTEXT_MSGMERGE_EXECUTABLE)
    MESSAGE(FATAL_ERROR "Please install msgmerge binary")
endif (NOT GETTEXT_MSGMERGE_EXECUTABLE)

if (NOT GETTEXT_MSGFMT_EXECUTABLE)
    MESSAGE(FATAL_ERROR "Please install msgfmt binary")
endif (NOT GETTEXT_MSGFMT_EXECUTABLE)

add_subdirectory(de)
add_subdirectory(cs)
