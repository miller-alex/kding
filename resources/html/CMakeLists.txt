set(kding_HTML_DIR ${DATA_INSTALL_DIR}/kding/html)

set(image_parts
	top-left top-middle top-right
	middle-left center middle-right
	bottom-left bottom-middle bottom-right
)
list(TRANSFORM image_parts APPEND ".png" OUTPUT_VARIABLE box_parts)
list(TRANSFORM box_parts PREPEND "${CMAKE_CURRENT_BINARY_DIR}/box-")

find_package(ImageMagick COMPONENTS magick-script REQUIRED)

add_custom_command(
	OUTPUT	${box_parts}
	COMMAND ${ImageMagick_magick-script_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/box.magick
	MAIN_DEPENDENCY box.magick
)
add_custom_target(box ALL DEPENDS ${box_parts})

install(
	FILES	kding.css welcome.html nomatches.html result.html
		top-left-kding.png ${box_parts}
	DESTINATION ${kding_HTML_DIR}
)
