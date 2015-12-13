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


#include <functional>

#include "font.h"
#include "menu.h"
#include "game.h"

#include "main_state.h"


#define ONE_SEC (1000000000)


MainState::MainState(Game* game)
	: _game(game),

      _entities(_game->log()),
      _sprites(_game->renderer()),
      _texts(),
      _inputs(_game->sys(), &_game->log()),

      _slotTracker(),

      _camera(),

      _initialized(false),
      _running(false),
      _loop(_game->sys()),
      _fpsTime(0),
      _fpsCount(0),

      _fontTex(nullptr),
      _fontJson(),
      _font(),

	  _drinkInput(nullptr),
	  _eatInput(nullptr),
	  _debugInput(nullptr),

      _bgSprite(),

	  _bg() {
}


MainState::~MainState() {
}


Sprite MainState::loadSprite(const char* file, unsigned th, unsigned tv) {
	Texture* tex = _game->renderer()->getTexture(
				file, Texture::BILINEAR | Texture::CLAMP);
	return Sprite(tex, th, tv);
}


void MainState::initialize() {
	_loop.reset();
	_loop.setTickDuration(    1000000000 /  60);
	_loop.setFrameDuration(   1000000000 /  60);
	_loop.setMaxFrameDuration(_loop.frameDuration() * 3);
	_loop.setFrameMargin(     _loop.frameDuration() / 2);

	_game->window()->onResize.connect(std::bind(&MainState::layoutScreen, this))
	        .track(_slotTracker);
	layoutScreen();


	_drinkInput  = _inputs.addInput("drink");
	_eatInput    = _inputs.addInput("eat");

	_inputs.mapScanCode(_drinkInput, SDL_SCANCODE_DOWN);
	_inputs.mapScanCode(_eatInput,   SDL_SCANCODE_UP);

	//TODO: Remove cheats.
	_debugInput = _inputs.addInput("debug");
	_inputs.mapScanCode(_debugInput, SDL_SCANCODE_F1);

	_fontJson = _game->sys()->loader().getJson("8-bit_operator+_regular_23.json");
	_fontTex  = _game->renderer()->getTexture(_fontJson["file"].asString(),
	        Texture::NEAREST | Texture::REPEAT);
	_font.reset(new Font(_fontJson, _fontTex));
	_font->baselineToTop = 12;

	_bgSprite          = loadSprite("dummy.png");
	_characterSprite   = loadSprite("dummy.png");
	_foodBarSprite     = loadSprite("food_bar.png");
	_waterBarSprite    = loadSprite("water_bar.png");
	_barBgSprite       = loadSprite("bar_bg.png");

//	_music1      = _game->audio()->loadMusic(_game->dataPath() / "music1.ogg");

//	_hitSound       = _game->audio()->loadSound(_game->dataPath() / "hit.ogg");

//	_damageAnim.reset(new MoveAnim(ONE_SEC/2, Vector3(0, 30, 0), RELATIVE));
//	_damageAnim->onEnd = [this](_Entity* e){ _entities.destroyEntity(EntityRef(e)); };

//	_bg                = createSprite(&_bgSprite, Vector3(0, 0, 0), Vector2(2, 2), "bg");
	_character         = createSprite(&_bgSprite, Vector3(0, 0, 0), "character");
	_character.sprite()->setAnchor(Vector2(.5, 0));

	_foodBar           = createSprite(&_foodBarSprite,  "food_bar");
	_waterBar          = createSprite(&_waterBarSprite, "water_bar");
	_foodBarBg         = createSprite(&_barBgSprite,    "food_bar_bg");
	_waterBarBg        = createSprite(&_barBgSprite,    "water_bar_bg");
	_foodBar   .sprite()->setAnchor(Vector2(.5, .5));
	_waterBar  .sprite()->setAnchor(Vector2(.5, .5));
	_foodBarBg .sprite()->setAnchor(Vector2(.5, .5));
	_waterBarBg.sprite()->setAnchor(Vector2(.5, .5));

	_initialized = true;
}


void MainState::shutdown() {
//	_game->audio()->releaseMusic(_music1);

	_slotTracker.disconnectAll();

	_initialized = false;
}


void MainState::run() {
	lairAssert(_initialized);

	log().log("Starting main state...");
	_running = true;
	_loop.start();
	_fpsTime  = _game->sys()->getTimeNs();
	_fpsCount = 0;

	startGame();

	do {
		switch(_loop.nextEvent()) {
		case InterpLoop::Tick:
			updateTick();
			break;
		case InterpLoop::Frame:
			updateFrame();
			break;
		}
	} while (_running);
	_loop.stop();
}


void MainState::quit() {
	_running = false;
}


void MainState::layoutScreen() {
	int w = _game->window()->width();
	int h = _game->window()->height();
	glViewport(0, 0, w, h);
	_camera.setViewBox(Box3(
		Vector3(0, 0, -1),
		Vector3(w, h,  1)
	));
}


EntityRef MainState::createSprite(Sprite* sprite, const char* name) {
	return createSprite(sprite, Vector3(0, 0, 0), Vector2(1, 1), name);
}


EntityRef MainState::createSprite(Sprite* sprite, const Vector3& pos,
								  const char* name) {
	return createSprite(sprite, pos, Vector2(1, 1), name);
}


EntityRef MainState::createSprite(Sprite* sprite, const Vector3& pos,
                                  const Vector2& scale, const char* name) {
	EntityRef entity = _entities.createEntity(_entities.root(), name);
	_sprites.addComponent(entity);
	entity.sprite()->setSprite(sprite);
	entity.place(Translation(pos) * Eigen::Scaling(scale.x(), scale.y(), 1.f));
	_anims.addComponent(entity);
	return entity;
}


EntityRef MainState::createText(const std::string& text, const Vector3& pos,
                                      const Vector4& color) {
	EntityRef entity = _entities.createEntity(_entities.root(), "text");
	_texts.addComponent(entity);
	TextComponent* comp = _texts.get(entity);
	comp->font = _font.get();
	comp->text = text;
	comp->color = color;
	entity.place(Transform(Translation(pos)));
	return entity;
}


void MainState::startGame() {
	_state               = Playing;

	_foodstuffs = std::vector<Foodstuff>();

	_foodLevel           = 1;
	_waterLevel          = 1;
	_size                = 1;
// 	_sizeGrowthRemaining = 0;
// 	_sizeDecayRemaining  = 0;

	_activeEffects = std::vector<Effect>();

	//FIXME: Temporary chicken test (cluck-cluck).
	Foodstuff chicken = {FOOD,std::vector<Effect>()};
	chicken.effects.push_back({FOOD,-0.1,5,5,&chicken});
	chicken.effects.push_back({GROWTH,0.1,10,10,&chicken});
	
	_foodstuffs.push_back(chicken);
	
	for (Effect e : chicken.effects)
		_activeEffects.push_back(e);
	
	// Natural hunger and thirst.
	_activeEffects.push_back({FOOD,-0.001,INFINITY,INFINITY,nullptr});
	_activeEffects.push_back({DRINK,-0.01,INFINITY,INFINITY,nullptr});

// 	_foodDecayPerSecond  = 0.05;
// 	_foodPower           = 0.4;
// 	_foodSizeGrowth      = 0.3;
// 	_waterDecayPerSecond = 0.1;
// 	_waterPower          = 0.4;
// 	_waterSizeDecay      = 0.2;
// 	_sizeGrowthPerSecond = 0.6;
// 	_sizeDecayPerSecond  = 1.0;
}


void MainState::updateTick() {
	_inputs.sync();

	if(_debugInput->justPressed()) {
		// Hey ! Insert debug action here !
		startGame();
	}

	if(_state == Dead)
		return;

	float td = float(_loop.tickDuration()) / ONE_SEC;

	for (Effect& e : _activeEffects)
	{
		float amount = e.changePerSecond * td;
		switch (e.type)
		{
			case FOOD:
				_foodLevel += amount;
				break;
			case DRINK:
				_waterLevel += amount;
				break;
			case GROWTH:
				_size += amount;
				break;
		}
		e.effectDuration -= td;
	}
	
	//NOTE: StackOverflow comment :
	// +20 "STL 'idioms' like this make me use Python for small projects."
	std::vector<Effect> v = _activeEffects;
	_activeEffects.erase(
		std::remove_if(_activeEffects.begin(), _activeEffects.end(),
			[] (Effect e)->bool { return e.effectDuration <= 0; }),
		_activeEffects.end());
	
// 	_foodLevel  -= _foodDecayPerSecond  * td;
// 	_waterLevel -= _waterDecayPerSecond * td;
// 
// 	if(_eatInput->justPressed()) {
// 		_foodLevel            = std::min(_foodLevel  + _foodPower,  1.f);
// 		_sizeGrowthRemaining += _foodSizeGrowth;
// 	}
// 	if(_drinkInput->justPressed()) {
// 		_waterLevel           = std::min(_waterLevel + _waterPower, 1.f);
// 		_sizeDecayRemaining  += _waterSizeDecay;
// 	}
// 
// 	float growth = std::min(_sizeGrowthRemaining, _sizeGrowthPerSecond * td);
// 	float decay  = std::min(_sizeDecayRemaining,  _sizeDecayPerSecond  * td);
// 
// 	_size += growth - decay;
// 
// 	_sizeGrowthRemaining -= growth;
// 	_sizeDecayRemaining  -= decay;

	if(_size <= 0 || _size > 3 || _foodLevel <= 0 || _waterLevel <= 0)
		_state = Dead;

	//log().info("food: ", _foodLevel, ", water: ", _waterLevel, ", size: ", _size);
}


void MainState::updateFrame() {
	int w = _game->window()->width();
	int h = _game->window()->height();

	_character.place(Translation(Vector3(w/2, h/4, 0))
				   * Eigen::Scaling(_size, _size, 1.f));

	_foodBar   .place(Transform(Translation(Vector3(1./4. * w, h/2, .5))));
	_foodBarBg .place(Transform(Translation(Vector3(1./4. * w, h/2, .4))));
	_waterBar  .place(Transform(Translation(Vector3(3./4. * w, h/2, .5))));
	_waterBarBg.place(Transform(Translation(Vector3(3./4. * w, h/2, .4))));

	_foodBar .sprite()->setView(Box2(Vector2(0, 0), Vector2(1, _foodLevel)));
	_waterBar.sprite()->setView(Box2(Vector2(0, 0), Vector2(1, _waterLevel)));

	// Rendering

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_game->renderer()->mainBatch().clearBuffers();

	_entities.updateWorldTransform();
	_sprites.render(_loop.frameInterp(), _camera);
	_texts.render(  _loop.frameInterp(), _game->renderer());
	_anims.update(_loop.tickDuration());

	_game->renderer()->spriteShader()->use();
	_game->renderer()->spriteShader()->setTextureUnit(0);
	_game->renderer()->spriteShader()->setViewMatrix(_camera.transform());
	_game->renderer()->mainBatch().render();

	_game->window()->swapBuffers();

	uint64 now = _game->sys()->getTimeNs();
	++_fpsCount;
	if(_fpsCount == 60) {
		log().info("Fps: ", _fpsCount * float(ONE_SEC) / (now - _fpsTime));
		_fpsTime  = now;
		_fpsCount = 0;
	}

	LAIR_LOG_OPENGL_ERRORS_TO(log());
}

Logger& MainState::log() {
	return _game->log();
}
