# Add ethash
set(include_dir ethash/include)
set(prefix_ethash ethash/lib/ethash)
## add new target, because ethash at submodule have too many properties
## and we can do it here more simplier
add_library(
    ethash
    ${prefix_ethash}/bit_manipulation.h
    ${prefix_ethash}/builtins.h
    ${prefix_ethash}/endianness.hpp
    ${include_dir}/ethash/ethash.h
    ${include_dir}/ethash/ethash.hpp
    ${prefix_ethash}/ethash-internal.hpp
    ${prefix_ethash}/ethash.cpp
    ${include_dir}/ethash/hash_types.h
    ${prefix_ethash}/managed.cpp
    ${include_dir}/ethash/keccak.h
    ${include_dir}/ethash/keccak.hpp
    ${prefix_ethash}/keccak.c
    ${prefix_ethash}/keccakf800.c
    ${prefix_ethash}/keccakf1600.c
    ${prefix_ethash}/kiss99.hpp
    ${prefix_ethash}/primes.h
    ${prefix_ethash}/primes.c
    ${include_dir}/ethash/progpow.hpp
    ${prefix_ethash}/progpow.cpp
)
target_include_directories(ethash PUBLIC ${include_dir})

# prefix of build dir
set(BUILD_DIR_PREFIX "${CMAKE_BINARY_DIR}/deps")
## add not cmake target
## libprefix - submodule prefix (examples: vrf/vdf)
## libpath - name of compiled library (examples: libvdf.a/libsodium.a/libdailyevm.dylib)
## libincludepath - relative path (after `*/include/`) to headers (examples: ""/""/tarxa-evm)
## buildcommand - build command of library (example: "${CMAKE_MAKE_PROGRAM} OPENSSL_HOME=${CONAN_OPENSSL_ROOT} all")
function(add_make_target libprefix libname buildcommand)
    # write it to file
    # because if we use it as it is we will have an error with enviroment vars
    # they will be interpreted in the wrong key (CGO_CFLAGS_ALLOW='.*' goes as "CGO_CFLAGS_ALLOW=.*")
    set(${libprefix}_SHFILE ${BUILD_DIR_PREFIX}/setenv_for_daily_${libprefix}.sh)
    file(WRITE ${${libprefix}_SHFILE} "${buildcommand}")
    # library path
    set(${libprefix}_LIBRARY "${BUILD_DIR_PREFIX}/daily-${libprefix}/lib/${libname}")
    # inlcude path
    set(${libprefix}_INCLUDE_DIR "${BUILD_DIR_PREFIX}/daily-${libprefix}/include")
    ExternalProject_Add(
        ${libprefix}_lib_submodule
        PREFIX ${BUILD_DIR_PREFIX}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/daily-${libprefix}
        BINARY_DIR ${BUILD_DIR_PREFIX}/daily-${libprefix}
        CONFIGURE_COMMAND cp -r <SOURCE_DIR> ${BUILD_DIR_PREFIX}
        BUILD_COMMAND chmod +x ${${libprefix}_SHFILE} && bash ${${libprefix}_SHFILE}
        INSTALL_COMMAND ""
        BUILD_BYPRODUCTS ${${libprefix}_LIBRARY}
    )

    add_library(daily-${libprefix} STATIC IMPORTED GLOBAL)
    file(MAKE_DIRECTORY "${${libprefix}_INCLUDE_DIR}")  # Must exist.
    set_property(TARGET daily-${libprefix} PROPERTY IMPORTED_CONFIGURATIONS Release)
    set_property(TARGET daily-${libprefix} PROPERTY IMPORTED_LOCATION_RELEASE "${${libprefix}_LIBRARY}")
    set_property(TARGET daily-${libprefix} PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${${libprefix}_INCLUDE_DIR}")
    add_dependencies(daily-${libprefix} ${libprefix}_lib_submodule)

    # add public includes
    target_include_directories(daily-${libprefix} PUBLIC ${INTERFACE_INCLUDE_DIRECTORIES})
endfunction()

# Add daily-vdf
set(VDF_LIBRARY_COMMAND "\
 ${CMAKE_MAKE_PROGRAM}\
 CPPFLAGS=\"-I./include -I${CONAN_INCLUDE_DIRS_OPENSSL} -I${GMP_INCLUDES} -I${MPFR_INCLUDES} -fPIC -std=c++20 -O3\"\
 lib/libvdf.a")
add_make_target(vdf libvdf.a "${VDF_LIBRARY_COMMAND}")


# Add daily-vrf
## run autotools
set(VRF_AUTOTOOLS_CMD "autoreconf && automake")
## run configure
set(VRF_AUTOTOOLS_CMD "${VRF_AUTOTOOLS_CMD} && ./configure --enable-static=\"yes\" --enable-shared=\"no\" --with-pic=\"yes\" --prefix=${CMAKE_BINARY_DIR}/deps/daily-vrf")
add_make_target(vrf libsodium.a "${VRF_AUTOTOOLS_CMD} && ${CMAKE_MAKE_PROGRAM} && ${CMAKE_MAKE_PROGRAM} install")

# Add daily-evm
set(EVM_BUILD_DIR ${BUILD_DIR_PREFIX}/daily-evm)
## add include of libs
set(EVM_BUILD_INCLUDE -I${CONAN_INCLUDE_DIRS_ROCKSDB})
## set C flags
set(EVM_BUILD_CGO_CFLAGS -O3 ${EVM_BUILD_INCLUDE})
## add link of libs
set(EVM_BUILD_LD -L${CONAN_LIB_DIRS_ROCKSDB} -lrocksdb -L${CONAN_LIB_DIRS_LZ4} -llz4)
## add path to homebew installed libs on macos
if (APPLE)
    set(EVM_BUILD_LD ${EVM_BUILD_LD} -L/opt/homebrew/lib)
endif()
## if we need full static build use flag
if(DAILY_STATIC_BUILD)
    if (NOT APPLE)
        set(EVM_BUILD_LD ${EVM_BUILD_LD} -static)
    endif()
    ## name of the static library
    set(EVM_LIBRARY_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}dailyevm${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
    ## name of the shared library
    set(EVM_LIBRARY_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}dailyevm${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()
## go env settings
set(EVM_BUILD_ENV CGO_CFLAGS_ALLOW='.*' CGO_CXXFLAGS_ALLOW='.*' CGO_CXXFLAGS=\"-O3\" CGO_CFLAGS=\"${EVM_BUILD_CGO_CFLAGS}\" CGO_LDFLAGS=\"${EVM_BUILD_LD}\" CACHE INTERNAL "")
## add build command
set(EVM_BUILD_COMMAND cd daily/C && ${EVM_BUILD_ENV} go build -buildvcs=false -buildmode=c-archive -o ${EVM_BUILD_DIR}/${EVM_LIBRARY_NAME})
## cp headers into `daily-evm/include`
set(EVM_AFTER_BUILD_COMMAND cd - && mkdir -p ${EVM_BUILD_DIR}/include/daily-evm && cp daily/C/*.h ${EVM_BUILD_DIR}/include/daily-evm/)
## mv library into `lib`
set(EVM_AFTER_BUILD_COMMAND ${EVM_AFTER_BUILD_COMMAND} && mkdir -p ${EVM_BUILD_DIR}/lib && cp ${EVM_BUILD_DIR}/${EVM_LIBRARY_NAME} ${EVM_BUILD_DIR}/lib/)
## rename header `libdailyevm.h` into `daily-evm.h`
set(EVM_AFTER_BUILD_COMMAND ${EVM_AFTER_BUILD_COMMAND} && mv ${EVM_BUILD_DIR}/libdailyevm.h ${EVM_BUILD_DIR}/include/daily-evm/daily-evm.h)
## final command
set(EVM_LIBRARY_COMMAND ${EVM_BUILD_COMMAND} && ${EVM_AFTER_BUILD_COMMAND})

file(GLOB_RECURSE DAILY_EVM_SOURCES CONFIGURE_DEPENDS "daily-evm/*.go" )
list(APPEND DAILY_EVM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/daily-evm/daily/C/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/daily-evm/daily/C/state.h)

add_custom_command(
    OUTPUT ${EVM_BUILD_DIR}/lib/${EVM_LIBRARY_NAME}
    DEPENDS ${DAILY_EVM_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/daily-evm
    COMMAND ${EVM_LIBRARY_COMMAND}
    COMMENT "Building daily-evm library")

add_custom_target(daily_evm_build DEPENDS ${EVM_BUILD_DIR}/lib/${EVM_LIBRARY_NAME})
add_dependencies(daily_evm_build CONAN_PKG::rocksdb)

add_library(daily-evm INTERFACE)
add_dependencies(daily-evm daily_evm_build)
target_link_libraries(daily-evm INTERFACE ${EVM_BUILD_DIR}/lib/${EVM_LIBRARY_NAME})
target_include_directories(daily-evm INTERFACE ${EVM_BUILD_DIR}/include)

# needed for golang runtime that comes together with daily-evm
if (APPLE)  # if "APPLE" not sufficient, use if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(daily-evm INTERFACE "-framework CoreFoundation")
    target_link_libraries(daily-evm INTERFACE "-framework Security")
else ()
    target_link_libraries(daily-evm INTERFACE stdc++fs)
endif ()

add_custom_target(run_go_test)

add_custom_command(
    TARGET run_go_test
    DEPENDS ${DAILY_EVM_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND cd ./daily-evm && ${EVM_BUILD_ENV} go test ./...
    COMMENT "Running Go EVM tests")
