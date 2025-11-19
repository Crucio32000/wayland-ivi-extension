############################################################################
#
# Copyright 2010-2014 BMW Car IT GmbH
# Copyright (C) 2013 DENSO CORPORATION
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#		http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
############################################################################

cmake_minimum_required (VERSION 2.6...3.22)

project(ilmArtInputPolicy)

find_package(Threads)
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT wayland-client REQUIRED)

GET_TARGET_PROPERTY(ILM_CONTROL_INCLUDE_DIRS ilmControl INCLUDE_DIRECTORIES)

find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner)

add_custom_command(
    OUTPUT  ivi-art-input-policy-client-protocol.h
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header
            < ${CMAKE_SOURCE_DIR}/protocol/ivi-art-input-policy.xml
            > ${CMAKE_CURRENT_BINARY_DIR}/ivi-art-input-policy-client-protocol.h
    DEPENDS ${CMAKE_SOURCE_DIR}/protocol/ivi-art-input-policy.xml
)

add_custom_command(
    OUTPUT  ivi-art-input-policy-protocol.c
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} code
            < ${CMAKE_SOURCE_DIR}/protocol/ivi-art-input-policy.xml
            > ${CMAKE_CURRENT_BINARY_DIR}/ivi-art-input-policy-protocol.c
    DEPENDS ${CMAKE_SOURCE_DIR}/protocol/ivi-art-input-policy.xml
)

include_directories(
    include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ILM_CONTROL_INCLUDE_DIRS}
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
)

link_directories(
    ${WAYLAND_CLIENT_LIBRARY_DIRS}
)

add_library(${PROJECT_NAME} SHARED
    src/ilm_art_input_policy.c
    ivi-art-input-policy-client-protocol.h
    ivi-art-input-policy-protocol.c
)

add_dependencies(${PROJECT_NAME}
    ilmControl
)

set(LIBS
    ${LIBS}
    ilmControl
    rt
    dl
    ${CMAKE_THREAD_LIBS_INIT}
    ${WAYLAND_CLIENT_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME} ${LIBS})

install (
    TARGETS             ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install (
    FILES       ${CMAKE_SOURCE_DIR}/ivi-layermanagement-api/ilmArtInputPolicy/include/ilm_art_input_policy.h
    DESTINATION include/ilm
)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES VERSION ${ILM_API_VERSION} SOVERSION ${ILM_API_MAJOR_VERSION})


#=============================================================================================
# generate documentation for ilmInput API
#=============================================================================================

add_custom_target(ilm-art-input-policy-doc
                  COMMAND cat ${CMAKE_SOURCE_DIR}/doc/Doxyfile.template
                          | sed 's/___DOC_NAME___/ilm_art_input_policy.h API/'
                          | sed 's/___DOC_VERSION___/${ILM_API_VERSION}/'
                          | sed 's!___INPUT_FILE___!${CMAKE_SOURCE_DIR}/ivi-layermanagement-api/ilmCommon/include/ilm_common.h ${CMAKE_CURRENT_SOURCE_DIR}/include/ilm_art_input_policy.h!'
                          | sed 's/___OUTPUT_DIR___/ilm-art-input-policy-doc/'
                          | doxygen -
                  COMMAND make --silent -C ilm-art-input-policy-doc/latex
                  COMMAND cp ilm-art-input-policy-doc/latex/refman.pdf
                             ${CMAKE_BINARY_DIR}/ilm-art-input-policy-api-${ILM_API_VERSION}.pdf
                  COMMENT "Generating ilm-art-input-policy-api-${ILM_API_VERSION}.pdf"
)

#=============================================================================================
# generate pkg-config file for ilmInput API
#=============================================================================================

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/ilmArtInputPolicy.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ilmArtInputPolicy.pc"
    @ONLY
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/ilmArtInputPolicy.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
