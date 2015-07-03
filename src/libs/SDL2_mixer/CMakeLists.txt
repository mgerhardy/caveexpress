set(SRCS
	dynamic_flac.c
	dynamic_fluidsynth.c
	dynamic_mod.c
	dynamic_mp3.c
	dynamic_ogg.c
	effect_position.c
	effects_internal.c
	effect_stereoreverse.c
	fluidsynth.c
	load_aiff.c
	load_flac.c
	load_ogg.c
	load_voc.c
	mixer.c
	music.c
	music_cmd.c
	music_flac.c
	music_mad.c
	music_mod.c
	music_modplug.c
	music_ogg.c
	wavestream.c
	libogg-1.3.1/src/framing.c
	libogg-1.3.1/src/bitwise.c
	)

if (ANDROID)
list(APPEND SRCS
	libvorbisidec-1.2.1/mdct.c
	libvorbisidec-1.2.1/block.c
	libvorbisidec-1.2.1/window.c
	libvorbisidec-1.2.1/synthesis.c
	libvorbisidec-1.2.1/info.c
	libvorbisidec-1.2.1/floor1.c
	libvorbisidec-1.2.1/floor0.c
	libvorbisidec-1.2.1/vorbisfile.c
	libvorbisidec-1.2.1/res012.c
	libvorbisidec-1.2.1/mapping0.c
	libvorbisidec-1.2.1/registry.c
	libvorbisidec-1.2.1/codebook.c
	libvorbisidec-1.2.1/sharedbook.c)
else()
list(APPEND SRCS
	libvorbis-1.3.3/lib/mapping0.c
	libvorbis-1.3.3/lib/synthesis.c
	libvorbis-1.3.3/lib/mdct.c
	libvorbis-1.3.3/lib/lpc.c
	libvorbis-1.3.3/lib/lookup.c
	libvorbis-1.3.3/lib/info.c
	libvorbis-1.3.3/lib/block.c
	libvorbis-1.3.3/lib/lsp.c
	libvorbis-1.3.3/lib/vorbisfile.c
	libvorbis-1.3.3/lib/envelope.c
	libvorbis-1.3.3/lib/registry.c
	libvorbis-1.3.3/lib/smallft.c
	libvorbis-1.3.3/lib/bitrate.c
	libvorbis-1.3.3/lib/sharedbook.c
	libvorbis-1.3.3/lib/window.c
	libvorbis-1.3.3/lib/res0.c
	libvorbis-1.3.3/lib/codebook.c
	libvorbis-1.3.3/lib/vorbisenc.c
	libvorbis-1.3.3/lib/psy.c
	libvorbis-1.3.3/lib/analysis.c
	libvorbis-1.3.3/lib/floor0.c
	libvorbis-1.3.3/lib/floor1.c)
endif()
cp_add_library(
	LIB SDL2_mixer
	SRCS ${SRCS}
	CFLAGS "-Wno-cast-qual -Wno-shadow -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-missing-field-initializers -Wno-unused-label -Wno-format-zero-length"
	DEFINES "-DOGG_MUSIC -DWAV_MUSIC")
if (NOT SDL2_MIXER_FOUND)
	if (ANDROID)
		add_definitions(-DOGG_USE_TREMOR)
	endif()
	include_directories("libogg-1.3.1/include")
	if (ANDROID)
		include_directories("libvorbisidec-1.2.1")
	else()
		include_directories("libvorbis-1.3.3/include")
		include_directories("libvorbis-1.3.3/lib")
	endif()
	include_directories(${SDL2_INCLUDE_DIR})
endif()
