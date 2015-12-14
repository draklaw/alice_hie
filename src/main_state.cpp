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


Sprite MainState::loadSprite(const char* file, unsigned th, unsigned tv,
                             unsigned flags) {
	Texture* tex = _game->renderer()->getTexture(
				file, flags);
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

	_inputs.mapScanCode(_drinkInput, SDL_SCANCODE_RIGHT);
	_inputs.mapScanCode(_eatInput,   SDL_SCANCODE_LEFT);

	//TODO: Remove cheats.
	_debugInput = _inputs.addInput("debug");
	_inputs.mapScanCode(_debugInput, SDL_SCANCODE_F1);

	_fontJson = _game->sys()->loader().getJson("8-bit_operator+_regular_23.json");
	_fontTex  = _game->renderer()->getTexture(_fontJson["file"].asString(),
	        Texture::NEAREST | Texture::REPEAT);
	_font.reset(new Font(_fontJson, _fontTex));
	_font->baselineToTop = 12;

	_bgSprite          = loadSprite("bg.png");
	_characterSprite   = loadSprite("alice.png", 3, 1);
	_barsSprite        = loadSprite("bars.png", 3, 2);
	_foodsSprite       = loadSprite("foods.png", 8, 4);
	_dnSprite          = loadSprite("dn.png");
	_frameSprite       = loadSprite("frame.png", 3, 3, Texture::NEAREST | Texture::CLAMP);
	_splashSprite      = loadSprite("splash.png");

//	_music1      = _game->audio()->loadMusic(_game->dataPath() / "music1.ogg");

//	_hitSound       = _game->audio()->loadSound(_game->dataPath() / "hit.ogg");

//	_damageAnim.reset(new MoveAnim(ONE_SEC/2, Vector3(0, 30, 0), RELATIVE));
//	_damageAnim->onEnd = [this](_Entity* e){ _entities.destroyEntity(EntityRef(e)); };

	_bg                = createSprite(&_bgSprite, "bg");
	_bg.sprite()->setAnchor(Vector2(.5, .5));

	_journal           = createText("",Vector3(0,0,0));
	_texts.get(_journal)->color = Vector4(132/255., 87/255., 57/255., 1.);
	_character         = createSprite(&_characterSprite, Vector3(0, 0, 0), "character");
	_character.sprite()->setAnchor(Vector2(.5, .03));

	_foodBar           = createSprite(&_barsSprite, "food_bar",     4);
	_waterBar          = createSprite(&_barsSprite, "water_bar",    1);
	_foodBarBg         = createSprite(&_barsSprite, "food_bar_bg",  5);
	_waterBarBg        = createSprite(&_barsSprite, "water_bar_bg", 2);
	_foodBarFg         = createSprite(&_barsSprite, "food_bar_fg",  3);
	_waterBarFg        = createSprite(&_barsSprite, "water_bar_fg", 0);
	_foodBar   .sprite()->setAnchor(Vector2(1, .5));
	_waterBar  .sprite()->setAnchor(Vector2(0, .5));
	_foodBarBg .sprite()->setAnchor(Vector2(1, .5));
	_waterBarBg.sprite()->setAnchor(Vector2(0, .5));
	_foodBarFg .sprite()->setAnchor(Vector2(1, .5));
	_waterBarFg.sprite()->setAnchor(Vector2(0, .5));

	for(int i = 0; i < FOOD_QUEUE_SIZE; ++i) {
		_foodEntities .push_back(createSprite(&_foodsSprite));
		_drinkEntities.push_back(createSprite(&_foodsSprite));

		_foodEntities .back().sprite()->setAnchor(Vector2(.5, .5));
		_drinkEntities.back().sprite()->setAnchor(Vector2(.5, .5));
	}

	_dn                = createSprite(&_dnSprite);
	_dn        .sprite()->setAnchor(Vector2(.5, .5));

	_splash            = createSprite(&_splashSprite, "splash");
	_splash    .sprite()->setAnchor(Vector2(.5, .5));

	_frame.background  = &_frameSprite;

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


EntityRef MainState::createSprite(Sprite* sprite, const char* name, unsigned index) {
	return createSprite(sprite, Vector3(0, 0, 0), Vector2(1, 1), name, index);
}


EntityRef MainState::createSprite(Sprite* sprite, const Vector3& pos,
								  const char* name, unsigned index) {
	return createSprite(sprite, pos, Vector2(1, 1), name, index);
}


EntityRef MainState::createSprite(Sprite* sprite, const Vector3& pos,
                                  const Vector2& scale, const char* name,
                                  unsigned index) {
	EntityRef entity = _entities.createEntity(_entities.root(), name);
	_sprites.addComponent(entity);
	entity.sprite()->setSprite(sprite);
	entity.sprite()->setIndex(index);
	entity.place(Translation(pos) * Eigen::Scaling(scale.x(), scale.y(), 1.f));
	_anims.addComponent(entity);
	return entity;
}


EntityRef MainState::createMovingSprite(Sprite* sprite, int tileIndex,
										const Vector3& from, const Vector3& to,
										float duration, const Vector2& anchor) {
	EntityRef entity;
	for(MovingSprite& ms: _movingSprites) {
		if(ms.timeRemaining <= 0) {
			entity           = ms.entity;
			entity.place(Transform(Translation(from)));
			ms.target        = to;
			ms.timeRemaining = duration;
			break;
		}
	}
	if(!entity.isValid()) {
		entity = createSprite(sprite, from);
		_movingSprites.push_back(MovingSprite{entity, to, duration});
		log().info("Add MovingSprite...");
	}

	entity.sprite()->setIndex(tileIndex);
	entity.sprite()->setAnchor(anchor);
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


bool MainState::loadEffect(Effect* effect, const Json::Value& json) {
	std::string type = json["type"].asString();
	if     (type == "food")   effect->type = FOOD;
	else if(type == "drink")  effect->type = DRINK;
	else if(type == "growth") effect->type = GROWTH;
	else throw std::runtime_error("Unknown effect type "+type);

	effect->changePerSecond = json["cps"].asFloat();
	effect->totalDuration   = json["duration"].asFloat();

	return true;
}


bool MainState::loadFood(Foodstuff* foodstuff, const Json::Value& json) {
	std::string type = json["type"].asString();
	if     (type == "food")  foodstuff->type = FOOD;
	else if(type == "drink") foodstuff->type = DRINK;
	else throw std::runtime_error("Unknown foodstuff type "+type);

	foodstuff->name = json["name"].asString();
	foodstuff->tileIndex = json.get("tileIndex", 0).asInt();

	foodstuff->effects.clear();
	Effect effect;
	for(const Json::Value& value: json["effects"]) {
		if(loadEffect(&effect, value)) {
			effect.source = foodstuff;
			effect.effectDuration = effect.totalDuration;
			foodstuff->effects.push_back(effect);
		}
	}
	return true;
}


void MainState::loadFoodSettings(const char* filename) {
	_foodList.clear();
	_drinkList.clear();

	Path path = _game->dataPath() / filename;
	std::ifstream jsonFile(path.native());
	Json::Value json;
	try {
		jsonFile >> json;
	} catch(std::exception& e) {
		log().error("Error while parsing \"", filename, "\": ", e.what());
		return;
	}

	Foodstuff foodstuff;
	for(const Json::Value& food: json) {
		bool ok;
		try {
			ok = loadFood(&foodstuff, food);
		} catch(std::exception& e) {
			log().error("Error while parsing \"", filename, "\": ", e.what());
			return;
		}

		if(ok) {
			if(foodstuff.type == FOOD) {
				_foodList.push_back(foodstuff);
			} else {
				_drinkList.push_back(foodstuff);
			}
		}
	}
}

void MainState::loadMotd(const char* filename)
{
	_motd.clear();

	Path path = _game->dataPath() / filename;
	std::ifstream jsonFile(path.native());

	try {
		jsonFile >> _motd;
	} catch (std::exception& e) {
		log().error("Error while parsing \"", filename, "\": ", e.what());
		return;
	}
}


void MainState::startGame() {
	_state               = Playing;
	_lastFrameTime       = _loop.frameTime();

	_timeOfDay = DAY_LENGTH+.01;
	_day = _msg = 0;
	loadMotd("motd.json");

	_drinkDelay = -1;
	_eatDelay = -1;


	_foodLevel           = MAX_FOOD;
	_waterLevel          = MAX_DRINK;
	_size                = START_GROWTH;

	loadFoodSettings("food.json");
	fetchDailyCrate();

	srand(time(nullptr));
	for (unsigned i = 0 ; i < QUEUE_SIZE ; i++)
	{
		_foodQueue.push_back(randomFood());
		_drinkQueue.push_back(randomDrink());
	}

	_foodQueueOffset  = 0;
	_drinkQueueOffset = 0;

	_activeEffects = std::vector<Effect>();

	// Natural hunger and thirst.
	float inf = std::numeric_limits<float>::infinity();
	_activeEffects.push_back({FOOD,-30,inf,inf,nullptr});
	_activeEffects.push_back({DRINK,-50,inf,inf,nullptr});
}

//NOTE: Should probably be a lambda or something but frankly IDC.
Foodstuff MainState::getFoodByName (const std::string& name)
{
	for (Foodstuff& f : _foodList)
		if (f.name == name)
			return f;

	for (Foodstuff& f : _drinkList)
		if (f.name == name)
			return f;

	throw std::runtime_error("No such foodstuff.");
}

void MainState::fetchDailyCrate ()
{
	//NOTE: Adding the same food several times would change proportions.
	switch (_day)
	{
		case 0:
			_foodOfTheDay.push_back(getFoodByName("chicken"));
			_drinkOfTheDay.push_back(getFoodByName("water"));
			break;
		case 1:
			_foodOfTheDay.push_back(getFoodByName("tack"));
			_drinkOfTheDay.push_back(getFoodByName("soda"));
			break;
		case 2:
			_drinkOfTheDay.push_back(getFoodByName("seawater"));
			break;
		case 3:
			_drinkOfTheDay.push_back(getFoodByName("syrup"));
			_foodOfTheDay.push_back(getFoodByName("pizza"));
			break;
		case 4:
			_foodOfTheDay.push_back(getFoodByName("fries"));
			break;
		case 5:
			_foodOfTheDay.push_back(getFoodByName("blue_shroom"));
			_foodOfTheDay.push_back(getFoodByName("red_shroom"));
			break;
		default:
			//TODO: Here, nothing.
			break;
	}
}

Foodstuff MainState::randomFood ()
{
	return _foodOfTheDay[rand()%_foodOfTheDay.size()];
}

Foodstuff MainState::randomDrink ()
{
	return _drinkOfTheDay[rand()%_drinkOfTheDay.size()];
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
	_timeOfDay += td;

	if (_timeOfDay > DAY_LENGTH)
	{
		if (_day > _motd.size())
			return; //FIXME: Congratulations ! You win nothing.

		if (_drinkInput->justPressed() && _timeOfDay > DAY_LENGTH + MSG_DELAY)
			_msg++;

		if (_msg < _motd[_day].size())
			_texts.get(_journal)->text = _motd[_day][_msg].asString();
		else
		{
			_texts.get(_journal)->text = "";
			if (_day != 0)
				fetchDailyCrate();
			_day++;
			_msg = 0;
			_timeOfDay = 0;
		}

		return;
	}

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

	//NOTE: StackOverflow comment (+20) :
	// "STL 'idioms' like this make me use Python for small projects."
	_activeEffects.erase(
		std::remove_if(_activeEffects.begin(), _activeEffects.end(),
			[] (const Effect& e)->bool { return e.effectDuration <= 0; }),
		_activeEffects.end());

	if (_eatInput->justPressed()) {
		if (_eatDelay < 0)
			_eatDelay = 0;
		else if (_eatDelay < DOUBLE_TAP_TIME)
		{
			Vector3 pp = _foodEntities[0].transform().translation();
			createMovingSprite(&_foodsSprite, _foodQueue.front().tileIndex,
			                   pp, Vector3(- 32, pp.y(), 0), .5);
			_foodQueue.pop_front();
			_foodQueueOffset += 1;
			_foodQueue.push_back(randomFood());
			_eatDelay = -1;
		}
	}

	if (_eatDelay > DOUBLE_TAP_TIME)
	{
		if (_foodLevel < MAX_FOOD)
		{
			for (Effect& e : _foodQueue[0].effects)
				_activeEffects.push_back(e);

			Vector3 pp = _foodEntities[0].transform().translation();
			createMovingSprite(&_foodsSprite, _foodQueue.front().tileIndex,
			                   pp, aliceMouthPos(), .5);

			_foodQueue.pop_front();
			_foodQueueOffset += 1;
			_foodQueue.push_back(randomFood());
		}
		_eatDelay = -1;
	}
	else if (_eatDelay >= 0)
		_eatDelay += td;

	if (_drinkInput->justPressed()) {
		if (_drinkDelay < 0)
			_drinkDelay = 0;
		else if (_drinkDelay < DOUBLE_TAP_TIME)
		{
			Vector3 pp = _drinkEntities[0].transform().translation();
			int w = _game->window()->width();
			createMovingSprite(&_foodsSprite, _drinkQueue.front().tileIndex,
			                   pp, Vector3(w + 32, pp.y(), 0), .5);
			_drinkQueue.pop_front();
			_drinkQueueOffset += 1;
			_drinkQueue.push_back(randomDrink());
			_drinkDelay = -1;
		}
	}

	if (_drinkDelay > DOUBLE_TAP_TIME)
	{
		if (_waterLevel < MAX_DRINK)
		{
			for (Effect& e : _drinkQueue[0].effects)
				_activeEffects.push_back(e);

			Vector3 pp = _drinkEntities[0].transform().translation();
			createMovingSprite(&_foodsSprite, _drinkQueue.front().tileIndex,
			                   pp, aliceMouthPos(), .5);

			_drinkQueue.pop_front();
			_drinkQueueOffset += 1;
			_drinkQueue.push_back(randomDrink());
		}
		_drinkDelay = -1;
	}
	else if (_drinkDelay >= 0)
		_drinkDelay += td;

	if(_size <= 0 || _size > MAX_GROWTH || _foodLevel <= 0 || _waterLevel <= 0)
		_state = Dead;

// 	log().info("food: ", _foodLevel, ", water: ", _waterLevel, ", size: ", _size);
}


void MainState::updateFrame() {
	float fd = float(_loop.frameTime() - _lastFrameTime) / ONE_SEC;

	int w = _game->window()->width();
	int h = _game->window()->height();

	float bgScale = std::min(float(w) / _bgSprite.width(),
	                         float(h) / _bgSprite.height());
	auto bgScaling = Eigen::Scaling(bgScale, bgScale, 1.f);
	_bg.place(Translation(Vector3(w/2., h/2., -1)) * bgScaling);

	float charScale = bgScale * _size / MAX_GROWTH; //h / 5000. * _size / START_GROWTH;
	_character.place(Translation(Vector3(w/2, h*0.106, 0))
	               * Eigen::Scaling(charScale, charScale, 1.f));
	if (_size < TINY_GROWTH)
		_character.sprite()->setIndex(2);
	else if (_size > HUGE_GROWTH)
		_character.sprite()->setIndex(1);
	else
		_character.sprite()->setIndex(0);


	_journal.place(Transform(Translation(Vector3(w/10, 9./10. * h, 1))));

	float   barHeight = h * .235;
	float   leftSide  = (w - std::min(w, h)) / 2.;
	float   rightSide = (w + std::min(w, h)) / 2.;
	Vector3 fbPos(leftSide,  barHeight, .5);
	Vector3 dbPos(rightSide, barHeight, .5);
	_foodBar   .place(Translation(fbPos)                     * bgScaling);
	_foodBarBg .place(Translation(fbPos - Vector3(0, 0, .1)) * bgScaling);
	_foodBarFg .place(Translation(fbPos + Vector3(0, 0, .1)) * bgScaling);
	_waterBar  .place(Translation(dbPos)                     * bgScaling);
	_waterBarBg.place(Translation(dbPos - Vector3(0, 0, .1)) * bgScaling);
	_waterBarFg.place(Translation(dbPos + Vector3(0, 0, .1)) * bgScaling);

	_foodBar .sprite()->setView(Box2(Vector2(0, 0),
	                                 Vector2(1, std::min(_foodLevel / MAX_FOOD,   1.f))));
	_waterBar.sprite()->setView(Box2(Vector2(0, 0),
	                                 Vector2(1, std::min(_waterLevel / MAX_DRINK, 1.f))));

	float stackOffset = STACK_OFFSET * bgScale;
	_foodQueueOffset  = std::max(_foodQueueOffset  - QUEUE_SCROLL_SPEED * fd, 0.);
	_drinkQueueOffset = std::max(_drinkQueueOffset - QUEUE_SCROLL_SPEED * fd, 0.);
	Vector3 foodEntityPos (leftSide  - STACK_OFFSET * bgScale * .65,
	                       9./16. * h + _foodQueueOffset  * stackOffset, .5);
	Vector3 drinkEntityPos(rightSide + STACK_OFFSET * bgScale * .65,
	                       9./16. * h + _drinkQueueOffset * stackOffset, .5);
	for (unsigned i = 0; i < FOOD_QUEUE_SIZE; ++i) {
		_foodEntities[i] .place(Translation(foodEntityPos)  * bgScaling);
		_drinkEntities[i].place(Translation(drinkEntityPos) * bgScaling);
		foodEntityPos  += Vector3(0, stackOffset, 0);
		drinkEntityPos += Vector3(0, stackOffset, 0);
		_foodEntities [i].sprite()->setIndex((i < _foodQueue .size())?_foodQueue [i].tileIndex:31);
		_drinkEntities[i].sprite()->setIndex((i < _drinkQueue.size())?_drinkQueue[i].tileIndex:31);
	}

	for(MovingSprite& ms: _movingSprites) {
		if(ms.timeRemaining > 0) {
			Vector3 pos = ms.entity.transform().translation();
			Vector3 diff = ms.target - pos;
			ms.entity.place(Translation(pos + diff * (fd / ms.timeRemaining)) * bgScaling);
			ms.timeRemaining -= fd;
		} else {
			ms.entity.place(Transform(Translation(Vector3(-1000, -1000, -1000))));
		}
	}

	float time = std::min(_timeOfDay / DAY_LENGTH, 1.f);
	_dn.place(Translation(Vector3(w*.5, h, .2))
		* AngleAxis(-time * M_PI * 2., Vector3::UnitZ()));

	_splash.place(Translation(w*.5, h*.5, (_state == Dead)? 1: -2) * bgScaling);

	float margin    = 32;
	_frame.position = Vector3(w * .1 - margin,   h * .7 + margin, .9);
	_frame.size     = Vector2(w * .8 + 2*margin, h * .2);

	// Rendering

	glClearColor(133./255., 88./255., 58./255., 1.);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_game->renderer()->mainBatch().clearBuffers();

	_entities.updateWorldTransform();
	_sprites.render(_loop.frameInterp(), _camera);
	_texts.render(  _loop.frameInterp(), _game->renderer());
	_anims.update(_loop.tickDuration());

	if(!_texts.get(_journal)->text.empty()) {
		_frame.render(_game->renderer());
	}

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

	_lastFrameTime = _loop.frameTime();

	LAIR_LOG_OPENGL_ERRORS_TO(log());
}


Vector3 MainState::aliceMouthPos() const {
	int w = _game->window()->width();
	int h = _game->window()->height();
	Vector3 pos = _character.transform().translation();
	return Vector3(pos.x(), pos.y() + _size * h  / MAX_GROWTH * .75, pos.z());
}


Logger& MainState::log() {
	return _game->log();
}
