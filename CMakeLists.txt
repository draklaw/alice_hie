##
##  Copyright (C) 2015 Simon Boyé
##
##  This file is part of alice_hie.
##
##  lair is free software: you can redistribute it and/or modify it
##  under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  lair is distributed in the hope that it will be useful, but
##  WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
##  General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with lair.  If not, see <http://www.gnu.org/licenses/>.
##

cmake_minimum_required(VERSION 2.8.12.1)

project(alice_hie)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)


add_subdirectory(third-party)


if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

add_executable(${PROJECT_NAME}
	src/frame.cpp
	src/font.cpp
	src/menu.cpp

	src/text_component.cpp
	src/animation_component.cpp
	src/sound_player.cpp

	src/game.cpp
	src/screen_state.cpp
	src/main_state.cpp
	src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
	lair
)
