# mallocer - A program to experiment with memory allocation
# Copyright (C) 2018 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.0)
project(mallocer VERSION 0.1.0)

option(WARNINGS "Switch on extra warnings" OFF)
option(WERROR "Turn warnings into errors" OFF)

include(GNUInstallDirs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu11")

if(WARNINGS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow -Wcast-qual -Wconversion")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winit-self -Wno-unused-parameter")
endif()

if(WERROR)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()

file(GLOB MALLOCER_SOURCES_C RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  mallocer.c)

add_executable(mallocer ${MALLOCER_SOURCES_C})

install(TARGETS mallocer
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# install(FILES
#   ${CMAKE_CURRENT_SOURCE_DIR}/doc/man/mallocer.1
#   DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

# EOF #
