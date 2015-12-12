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

#ifndef _UW_SOUND_PLAYER_H_
#define _UW_SOUND_PLAYER_H_

#include <string>
#include <unordered_map>

#include <SDL_mixer.h>

#include <lair/core/lair.h>
#include <lair/core/path.h>


#define SOUNDPLAYER_MAX_CHANNELS    32
#define SOUNDPLAYER_DEFAULT_VOLUME  (MIX_MAX_VOLUME / 2)


class Game;


class Sound {
public:
	~Sound();

public:
	Mix_Chunk*   chunk;
	unsigned     volume;

	lair::Path   name;
	unsigned     useCount;
};


class Music {
public:
	~Music();

public:
	Mix_Music*   track;

	lair::Path   name;
	unsigned     useCount;
};


class SoundPlayer {
public:
	SoundPlayer(Game *game);

	const Sound* loadSound(const lair::Path& filename);
	const Music* loadMusic(const lair::Path& filename);

	void releaseSound(const Sound* sound);
	void releaseMusic(const Music* music);

	int playSound(const Sound* sound, int loops);
	void playMusic(const Music* music);

	void haltSound(int channel);
	void haltMusic();

	void setMusicVolume(float volume);

private:
	typedef std::unordered_map<lair::Path, Sound> SoundMap;
	typedef std::unordered_map<lair::Path, Music> MusicMap;

private:
	Game*    _game;

	SoundMap _soundMap;
	MusicMap _musicMap;
};

#endif
