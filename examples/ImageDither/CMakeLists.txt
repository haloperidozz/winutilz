add_executable(ImageDither WIN32 main.c)

set_target_properties(ImageDither PROPERTIES C_STANDARD 90)

target_link_libraries(ImageDither PRIVATE winutilz)
