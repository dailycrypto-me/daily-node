add_executable(dailyd main.cpp)
target_link_libraries(dailyd PRIVATE
    app
)

set(DAILYD_INSTALL_DIR "/usr/local/bin" CACHE PATH "Daily application install directory")
install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dailyd DESTINATION ${DAILYD_INSTALL_DIR} COMPONENT dailyd)