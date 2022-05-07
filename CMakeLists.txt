cmake_minimum_required(VERSION 3.16)
project(Cosmo CXX)

set(HL2SDK_PATH ${PROJECT_SOURCE_DIR}/hl2sdk)
set(METAMOD_SOURCE ${PROJECT_SOURCE_DIR}/metamod-source)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 20)

# Source Engine is 32-bit, so force Lagom to build 32-bit too
add_compile_options(-m32)
add_link_options(-m32)

add_compile_definitions(AK_DONT_REPLACE_STD)
add_compile_options(-Werror=implicit-fallthrough)
add_compile_options(-Werror=switch)
add_compile_options(-Wno-literal-suffix)
add_compile_options(-fno-gnu-keywords)
# mathlib.h uses register and just spams a ton of warnings, so let's just ignore them, no one is actually going to use
# register anyways.
add_compile_options(-Wno-register)

include(FetchContent)
include(cmake/FetchLagom.cmake)
include(cmake/GetGitRevisionDescription.cmake)

get_git_head_revision(GIT_REFSPEC GIT_SHA1)
string(SUBSTRING ${GIT_SHA1} 0 7 GIT_SHA1_SHORT)

add_library(Cosmo SHARED
        src/Cosmo.cpp
        src/Dump.cpp
        src/Helpers.cpp
        src/Scripting/Command.cpp
        src/Scripting/Entity.cpp
        src/Scripting/EntityPrototype.cpp
        src/Scripting/Game.cpp
        src/Scripting/GlobalObject.cpp
        src/Scripting/Helpers.cpp
        src/Scripting/Server.cpp
        )

target_compile_definitions(Cosmo PRIVATE
        SOURCE_ENGINE=11
        stricmp=strcasecmp
        _stricmp=strcasecmp
        _snprintf=snprintf
        _vsnprintf=vsnprintf
        HAVE_STDINT_H
        GNUC
        _LINUX
        POSIX
        # These are all from sourcemod/AMBuildScript
        SE_EPISODEONE=1
        SE_ORANGEBOX=3
        SE_CSS=6
        SE_HL2DM=7
        SE_DODS=8
        SE_SDK2013=9
        SE_LEFT4DEAD=12
        SE_NUCLEARDAWN=13
        SE_LEFT4DEAD2=15
        SE_DARKMESSIAH=2
        SE_ALIENSWARM=16
        SE_BLOODYGOODTIME=4
        SE_EYE=5
        SE_CSGO=21
        SE_PORTAL2=17
        SE_BLADE=18
        SE_INSURGENCY=19
        SE_CONTAGION=14
        SE_DOI=20
        SE_MOCK=999
        COSMO_GIT_SHA1="${GIT_SHA1}"
        COSMO_GIT_SHA1_SHORT="${GIT_SHA1_SHORT}"
        )
target_include_directories(Cosmo PRIVATE SYSTEM
        ${PROJECT_SOURCE_DIR}
        ${HL2SDK_PATH}/public
        ${HL2SDK_PATH}/public/engine
        ${HL2SDK_PATH}/public/mathlib
        ${HL2SDK_PATH}/public/vstdlib
        ${HL2SDK_PATH}/public/tier0
        ${HL2SDK_PATH}/public/tier1
        ${HL2SDK_PATH}/public/game/server
        ${METAMOD_SOURCE}/core
        ${METAMOD_SOURCE}/core/sourcehook
        )

set_target_properties(Cosmo PROPERTIES PREFIX "")
target_link_directories(Cosmo PRIVATE ${HL2SDK_PATH}/lib/linux)
target_link_libraries(Cosmo PRIVATE Lagom::Core Lagom::JS -l:tier1_i486.a -l:libvstdlib_srv.so -l:libtier0_srv.so)
