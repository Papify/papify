
set(src_files
	cmake_editor.c
	eventLib.c
	eventLib.h
	original_files_recovery.c
	papicode.h
	papify.c
	papify.h
	papify_everything.c
	papify_lib.c
	xcf_creator
	xcf_creator.c
)

add_executable(papify ${src_files})
target_link_libraries(papify papi roxml)
