include(BundleUtilities)

#set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")

macro(cp_osx_get_directory_size DIR OUTVAR)
	file(GLOB_RECURSE FILES_ "${DIR}/*")
	set(OUTPUT 0)
	foreach(FILE ${FILES_})
		execute_process(COMMAND stat -f%z ${FILE} TIMEOUT 5 OUTPUT_VARIABLE FILESIZE_)
		math(EXPR OUTPUT "${OUTPUT} + ${FILESIZE_}")
	endforeach()
	set(${OUTVAR} ${OUTPUT})
endmacro()

macro(cp_osx_prepare_content TARGET BUNDLED)
	if (EXISTS ${ROOT_DIR}/cmake/${TARGET}.xcscheme)
		configure_file(${ROOT_DIR}/cmake/${TARGET}.xcscheme ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.xcodeproj/xcshareddata/xcschemes/${TARGET}.xcscheme)
	endif()
	set(ICNS_SOURCE ${ROOT_DIR}/contrib/${TARGET}-icon-512x512.png)
	if (EXISTS ${ICNS_SOURCE})
		set(ICNS_TARGET ${CMAKE_BINARY_DIR}/${TARGET}.icns)
		set(ICNS_SIZES "16;32;64;128;256;512")
		set(ICONSET_DIR ${CMAKE_BINARY_DIR}/${TARGET}.iconset)
		message(STATUS "Writing iconset ${ICONSET_DIR}")
		file(MAKE_DIRECTORY ${ICONSET_DIR})
		# TODO: fimd sips and iconutil
		foreach(ICNS_SIZE ${ICNS_SIZES})
			add_custom_command(TARGET ${TARGET} POST_BUILD
				COMMAND sips
				ARGS -z ${ICNS_SIZE} ${ICNS_SIZE} ${ICNS_SOURCE} --out ${ICONSET_DIR}/icon_${ICNS_SIZE}x${ICNS_SIZE}.png
				COMMENT "Create ${ICNS_SIZE}x${ICNS_SIZE} icon for ${TARGET}"
			)
			math(EXPR TARGETSIZE "${ICNS_SIZE} / 2")
			add_custom_command(TARGET ${TARGET} POST_BUILD
				COMMAND sips
				ARGS -z ${ICNS_SIZE} ${ICNS_SIZE} ${ICNS_SOURCE} --out ${ICONSET_DIR}/icon_${TARGETSIZE}x${TARGETSIZE}x2.png
				COMMENT "Create ${ICNS_SIZE}x${ICNS_SIZE}@2 icon for ${TARGET}"
			)
		endforeach()
		add_custom_command(TARGET ${TARGET} POST_BUILD
			COMMAND iconutil
			ARGS -c icns -o ${ICNS_TARGET} ${ICONSET_DIR}
			COMMENT "Create ${TARGET}.icns"
		)
		install(FILES ${ICNS_TARGET} DESTINATION ${TARGET}.app/Contents/Resources COMPONENT ${TARGET})
	endif()
endmacro()

macro(cp_osx_generate_plist_file TARGET)
	set(PLIST_PATH ${PROJECT_BINARY_DIR}/Info.plist)
	set_target_properties(${TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PLIST_PATH})
	configure_file(${ROOT_DIR}/contrib/installer/osx/Info.plist.template ${PLIST_PATH} @ONLY)
endmacro()

macro(cp_osx_add_target_properties TARGET APPNAME VERSION VERSION_CODE)
	cp_message("Set mac parameters")
	cp_set_properties(${TARGET} MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION}")
	cp_set_properties(${TARGET} MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_CODE}")
	cp_set_properties(${TARGET} MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_CODE}")
	cp_set_properties(${TARGET} MACOSX_BUNDLE_COPYRIGHT "CaveProductions")
	cp_set_properties(${TARGET} MACOSX_BUNDLE_INFO_STRING "")
	cp_set_properties(${TARGET} MACOSX_BUNDLE_GUI_IDENTIFIER "org.${TARGET}")
	cp_set_properties(${TARGET} MACOSX_BUNDLE_ICON_FILE "${TARGET}.icns")
	cp_set_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_INSTALL_PATH "/Applications")
	set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
	cp_set_properties(${TARGET} MACOSX_BUNDLE_SIGNATURE "????")
	cp_set_properties(${TARGET} XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY})
	cp_set_properties(${TARGET} MACOSX_BUNDLE_EXECUTABLE_NAME "${TARGET}")
	cp_set_properties(${TARGET} MACOSX_BUNDLE_PRODUCT_NAME "${APPNAME}")
	cp_set_properties(${TARGET} MACOSX_BUNDLE_BUNDLE_NAME "${APPNAME}")
	#cp_set_properties(${TARGET} MACOSX_BUNDLE TRUE)
	cp_osx_generate_plist_file(${TARGET})
endmacro()
