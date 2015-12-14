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


#ifndef _AHIE_MAIN_STATE_H
#define _AHIE_MAIN_STATE_H


#include <vector>
#include <deque>

#include <lair/core/lair.h>
#include <lair/core/log.h>
#include <lair/core/signal.h>

#include <lair/utils/interp_loop.h>
#include <lair/utils/input.h>

#include <lair/render_gl2/orthographic_camera.h>

#include <lair/ec/entity.h>
#include <lair/ec/entity_manager.h>
#include <lair/ec/sprite_component.h>

#include "menu.h"
#include "text_component.h"
#include "animation_component.h"
#include "sound_player.h"

#include "game_state.h"


using namespace lair;


class Game;

class Font;


#define FOOD_QUEUE_SIZE 10
#define QUEUE_SCROLL_SPEED 3.
#define STACK_OFFSET 70

#define TINY_GROWTH 300
#define START_GROWTH 1000
#define HUGE_GROWTH 1700
#define MAX_GROWTH 2000
#define MAX_FOOD 2000
#define MAX_DRINK 2000

#define DAY_LENGTH 30
#define MSG_DELAY 3

#define DOUBLE_TAP_TIME 0.3

enum Meter { FOOD, DRINK, GROWTH };

struct Foodstuff;

struct Effect {
	Meter type;            // Which meter is affected.
	float changePerSecond; // Impact on meter (units/s).
	float effectDuration;  // Remaining duration (s).
	float totalDuration;   // Total duration (s).
	Foodstuff* source;     // Source of the effect.
};

struct Foodstuff {
	Meter type;                  // Should be FOOD or DRINK.
	int tileIndex;
	std::vector<Effect> effects; // List of triggered effects.
};

struct MovingSprite {
	EntityRef entity;
	Vector3   target;
	float     timeRemaining;
};


class MainState : public GameState {
public:
	MainState(Game* game);
	~MainState();

	Sprite loadSprite(const char* file, unsigned th = 1, unsigned tv = 1);

	virtual void initialize();
	virtual void shutdown();

	virtual void run();
	virtual void quit();

	void layoutScreen();

	EntityRef createSprite(Sprite* sprite, const char* name = nullptr);
	EntityRef createSprite(Sprite* sprite, const Vector3& pos,
	                       const char* name = nullptr);
	EntityRef createSprite(Sprite* sprite, const Vector3& pos,
	                       const Vector2& scale,
	                       const char* name = nullptr);
	EntityRef createMovingSprite(Sprite* sprite, int tileIndex,
	                             const Vector3& from, const Vector3& to,
	                             float duration);
	EntityRef createText(const std::string& msg, const Vector3& pos,
	                     const Vector4& color = Vector4(1, 1, 1, 1));

	bool loadEffect(Effect* effect, const Json::Value& json);
	bool loadFood(Foodstuff* foodstuff, const Json::Value& json);
	void loadFoodSettings(const char* filename);
	void loadMotd(const char* filename);
	void startGame();

	Foodstuff randomFood ();
	Foodstuff randomDrink ();

	void updateTick();
	void updateFrame();

	Logger& log();


public:
	enum State {
		Playing,
		Dead
	};

public:
	Game* _game;

	// More or less system stuff

	EntityManager             _entities;
	SpriteComponentManager    _sprites;
	TextComponentManager      _texts;
	AnimationComponentManager _anims;
	InputManager              _inputs;

	SlotTracker _slotTracker;

	OrthographicCamera _camera;

	bool        _initialized;
	bool        _running;
	InterpLoop  _loop;
	int64       _fpsTime;
	unsigned    _fpsCount;

	Texture*    _fontTex;
	Json::Value _fontJson;
	std::unique_ptr<Font>
	            _font;

	std::vector<MovingSprite> _movingSprites;

	// Game related stuff

	Input*      _drinkInput;
	Input*      _eatInput;
	Input*      _debugInput;
	float       _drinkDelay;
	float       _eatDelay;

	Sprite      _bgSprite;
	Sprite      _characterSprite;
	Sprite      _foodBarSprite;
	Sprite      _waterBarSprite;
	Sprite      _barBgSprite;
	Sprite      _foodsSprite;

	EntityRef   _bg;
	EntityRef   _journal;
	EntityRef   _character;
	EntityRef   _foodBar;
	EntityRef   _waterBar;
	EntityRef   _foodBarBg;
	EntityRef   _waterBarBg;
	std::vector<EntityRef> _foodEntities;
	std::vector<EntityRef> _drinkEntities;

	// Game states

	State       _state;
	uint64      _lastFrameTime;

	float _timeOfDay;
	unsigned _day, _msg;
	Json::Value _motd;

	std::vector<Foodstuff> _foodList;
	std::vector<Foodstuff> _drinkList;

	std::deque<Foodstuff> _foodQueue;
	std::deque<Foodstuff> _drinkQueue;
	float       _foodQueueOffset;
	float       _drinkQueueOffset;

	float       _foodLevel;
	float       _waterLevel;
	float       _size;

	std::vector<Effect> _activeEffects;

};


#endif
