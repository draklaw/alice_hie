//
//  Copyright (C) 2015 the authors (see AUTHORS)
//
//  This file is part of alice_hie.
//
//  lair is free software: you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  lair is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with lair.  If not, see <http://www.gnu.org/licenses/>.
//


#include <iostream>
#include <functional>

#include <SDL_mixer.h>

#include "main_state.h"

#include "game.h"


#define DEFAULT_LOG_LEVEL LogLevel::Debug


Game::Game(int argc, char** argv)
    : _mlogger(),
      _logStream("log.txt"),
//#ifndef _WIN32
      _stdlogBackend(std::clog, true),
//#endif
      _fileBackend(_logStream, false),
      _logger("game", &_mlogger, DEFAULT_LOG_LEVEL),

      _dataPath(),

      _sys(nullptr),
      _window(nullptr),

      _renderModule(nullptr),
      _renderer(nullptr),

      _audio(nullptr),

      _nextState(nullptr),
      _currentState(nullptr),

      _mainState(nullptr) {
//#ifndef _WIN32
	_mlogger.addBackend(&_stdlogBackend);
//#endif
	_mlogger.addBackend(&_fileBackend);
	dbgLogger.setMaster(&_mlogger);
	dbgLogger.setDefaultModuleName("DEBUG");
	dbgLogger.setLevel(LogLevel::Debug);
}


Game::~Game() {
	log().log("Stopping game...");
}


Path Game::dataPath() const {
	return _dataPath;
}


SysModule* Game::sys() {
	return _sys.get();
}


Window* Game::window() {
	return _window;
}


RenderModule* Game::renderModule() {
	return _renderModule.get();
}


Renderer* Game::renderer() {
	return _renderer;
}


SoundPlayer* Game::audio() {
	return _audio.get();
}


void Game::initialize() {
	log().log("Starting game...");

	_sys.reset(new SysModule(&_mlogger, LogLevel::Log));
	_sys->initialize();
	_sys->onQuit = std::bind(&Game::quit, this);

	const char* envPath = std::getenv("LOF3_DATA_DIR");
	if (envPath) {
		_dataPath = envPath;
	} else {
		_dataPath = _sys->basePath() / "assets";
	}
	log().log("Data directory: ", _dataPath);

	_sys->loader().setNThread(1);
	_sys->loader().setBasePath(dataPath());

	SDL_InitSubSystem(SDL_INIT_AUDIO);
	Mix_Init(MIX_INIT_OGG);

	log().log("Initialize SDL_mixer...");
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024)) {
		log().error("Failed to initialize SDL_mixer backend");
	}
	Mix_AllocateChannels(SOUNDPLAYER_MAX_CHANNELS);

	_window = _sys->createWindow("Alice had it easy", 1280, 720);
	//_window->setFullscreen(true);
	//_sys->setVSyncEnabled(false);
	log().info("VSync: ", _sys->isVSyncEnabled()? "on": "off");

	_renderModule.reset(new RenderModule(sys(), &_mlogger, DEFAULT_LOG_LEVEL));
	_renderModule->initialize();
	_renderer = _renderModule->createRenderer();

	_audio.reset(new SoundPlayer(this));
	_audio->setMusicVolume(.2);
	_music = _audio->loadMusic(dataPath() / "alice_hie.ogg");
	_audio->playMusic(_music);

	_screenState.reset(new ScreenState(this));
	_screenState->initialize();

	_mainState.reset(new MainState(this));
	_mainState->initialize();
}


void Game::shutdown() {
	_audio->releaseMusic(_music);

	_mainState->shutdown();
	_mainState.reset();

	_screenState->shutdown();
	_screenState.reset();

	_renderModule->shutdown();
	_renderModule.reset();

	Mix_Quit();

	_window->destroy();
	_sys->shutdown();
	_sys.reset();
}


void Game::setNextState(GameState* state) {
	if(_nextState) {
		log().warning("Setting next state while an other state is enqueued.");
	}
	_nextState = state;
}


void Game::run() {
	while(_nextState) {
		_currentState = _nextState;
		_nextState    = nullptr;
		_currentState->run();
	}
}


void Game::quit() {
	if(_currentState) {
		_currentState->quit();
	}
}


ScreenState* Game::screenState() {
	return _screenState.get();
}


MainState* Game::mainState() {
	return _mainState.get();
}
