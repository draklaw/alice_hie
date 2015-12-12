/*
 *  Copyright (C) 2015 the authors (see AUTHORS)
 *
 *  This file is part of alice_hie.
 *
 *  usb_warrior is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  usb_warrior is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with usb_warrior.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdlib>
#include <iostream>

#include <SDL_mixer.h>

#include "game.h"

#include "sound_player.h"


Sound::~Sound() {
	if(chunk) {
		Mix_FreeChunk(chunk);
	}
}


Music::~Music() {
	if(track) {
		Mix_FreeMusic(track);
	}
}


SoundPlayer::SoundPlayer(Game* game)
	: _game(game),
	  _soundMap(),
	  _musicMap() {
}


const Sound* SoundPlayer::loadSound(const lair::Path& filename) {
	auto it = _soundMap.find(filename);
	if(it == _soundMap.end()) {
		Sound snd;

		_game->log().log("Load sound \"", filename, "\"...");

		snd.chunk = Mix_LoadWAV(filename.utf8CStr());
		if(!snd.chunk) {
			_game->log().error("Failed to load sound: ", Mix_GetError());
			return nullptr;
		}

		snd.volume = SOUNDPLAYER_DEFAULT_VOLUME;
		Mix_VolumeChunk(snd.chunk, snd.volume);
		snd.name = filename;
		snd.useCount = 0;

		auto res = _soundMap.emplace(filename, snd);
		assert(res.second);
		it = res.first;

		// Do not destroy the chunk when snd goes out of scope
		snd.chunk = nullptr;
	}

	++(it->second.useCount);

	return &(it->second);
}


const Music* SoundPlayer::loadMusic(const lair::Path& filename) {
	auto it = _musicMap.find(filename);
	if(it == _musicMap.end()) {
		Music mus;

		_game->log().log("Load music \"", filename, "\"...");

		mus.track = Mix_LoadMUS(filename.utf8CStr());
		if(!mus.track) {
			_game->log().error("Failed to load music: ", Mix_GetError());
			return nullptr;
		}
		mus.name = filename;
		mus.useCount = 0;

		auto res = _musicMap.emplace(filename, mus);
		assert(res.second);
		it = res.first;

		// Do not destroy the track when mus goes out of scope
		mus.track = nullptr;
	}

	++(it->second.useCount);

	return &(it->second);
}


void SoundPlayer::releaseSound(const Sound* sound) {
	auto it = _soundMap.find(sound->name);
	assert(it != _soundMap.end());

	--(it->second.useCount);
	if(!it->second.useCount) {
		_game->log().log("Release sound \"", it->second.name, "\"...");
		_soundMap.erase(it);
	}
}


void SoundPlayer::releaseMusic(const Music* music) {
	auto it = _musicMap.find(music->name);
	assert(it != _musicMap.end());

	--(it->second.useCount);
	if(!it->second.useCount) {
		_game->log().log("Release music \"", it->second.name, "\"...");
		_musicMap.erase(it);
	}
}


int SoundPlayer::playSound(const Sound* sound, int loops) {
	return sound->chunk ? Mix_PlayChannel(-1, sound->chunk, loops) : -1;
}


void SoundPlayer::playMusic(const Music* music) {
	if(music->track) {
		Mix_PlayMusic(music->track, -1);
	}
}


void SoundPlayer::haltSound(int channel) {
	if(channel != -1) { Mix_HaltChannel(channel); }
}


void SoundPlayer::haltMusic() {
	Mix_HaltMusic();
}


void SoundPlayer::setMusicVolume(float volume) {
	Mix_VolumeMusic(128 * volume);
}
