


add_library(includeLib STATIC textures/stb_image.cpp)

target_include_directories(includeLib PUBLIC bases)
target_include_directories(includeLib PUBLIC mapSetting)
target_include_directories(includeLib PUBLIC NPC)
target_include_directories(includeLib PUBLIC shader)
target_include_directories(includeLib PUBLIC parts)
target_include_directories(includeLib PUBLIC textures)
target_include_directories(includeLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(includeLib PUBLIC globalValues)

install(TARGETS includeLib ARCHIVE DESTINATION lib
                            LIBRARY DESTINATION lib)

install(DIRECTORY bases/ DESTINATION include/bases FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY globalValues/ DESTINATION include/globalValues FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY mapSetting/ DESTINATION include/mapSetting FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY NPC/ DESTINATION include/NPC FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY parts/ DESTINATION include/parts FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY shader/ DESTINATION include/shader FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY textures/ DESTINATION include/textures FILES_MATCHING PATTERN "*.hpp")
install(FILES itemTypes.hpp accessoryType.hpp DESTINATION include)

