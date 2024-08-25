add_executable(daily-bootnode main.cpp)
target_link_libraries(daily-bootnode PRIVATE
    cli
    p2p
)

set(DAILY_BOOTNODE_INSTALL_DIR "/usr/local/bin" CACHE PATH "Daily bootnode install directory")
install(PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/daily-bootnode DESTINATION ${DAILY_BOOTNODE_INSTALL_DIR} COMPONENT daily-bootnode)