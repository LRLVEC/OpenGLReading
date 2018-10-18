set(MICRO micro)

file(GLOB MICRO_SOURCE *.cpp)
file(GLOB MICRO_INLINE *.inl)
file(GLOB MICRO_HEADER *.hpp)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../data/micro)

set(GL_SHADER_GTC
	draw.vert draw.frag
	draw-uniform.vert draw-uniform.frag
	draw-uniform2.vert draw-uniform2.frag
	draw-range.vert draw-range.frag
	draw-indexing-id.vert draw-indexing-id.frag
	draw-indexing-attrib.vert draw-indexing-attrib.frag
	draw-indexing-uniform.vert draw-indexing-uniform.frag
	screenspace_coherence.vert screenspace_coherence.frag
	test_buffer.vert test_buffer.frag
	test_draw_call.vert test_draw_call.frag
	test_uniform_caching.vert test_uniform_caching.frag)

foreach(FILE ${GL_SHADER_GTC})
	set(SHADER_PATH ${SHADER_PATH} ${SHADER_DIR}/${FILE})
endforeach(FILE)

source_group("Inline Files" FILES ${MICRO_INLINE})
source_group("Shader Files" FILES ${MICRO_SHADER})

foreach(FILE ${GL_SHADER_GTC})
	set(SHADER_PATH ${SHADER_PATH} ${SHADER_DIR}/${FILE})
endforeach(FILE)

source_group("Shader Files" FILES ${SHADER_PATH})
add_executable(${MICRO} ${MICRO_SOURCE} ${MICRO_INLINE} ${MICRO_HEADER} ${SHADER_PATH})
add_custom_command(
	TARGET ${MICRO} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)

target_link_libraries(${MICRO} ${FRAMEWORK_NAME} ${BINARY_FILES})
add_dependencies(${MICRO} ${FRAMEWORK_NAME} glfw)

