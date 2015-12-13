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


#ifndef _AHIE_ANIMATION_COMPONENT_H
#define _AHIE_ANIMATION_COMPONENT_H


#include <lair/core/lair.h>
#include <lair/core/log.h>

#include <lair/ec/entity.h>
#include <lair/ec/component.h>
#include <lair/ec/component_manager.h>


using namespace lair;

namespace lair {
class Sprite;
}


enum AnimType {
	ABSOLUTE,
	RELATIVE
};

class AnimationComponentManager;


struct Animation {
	virtual ~Animation() = default;
	virtual Animation* clone() = 0;

	virtual void updateLength();
	virtual void begin(_Entity* entity);
	virtual void update(uint64 time, _Entity* entity) = 0;
	virtual void end(_Entity* entity);

	uint64 length;
	std::function<void(_Entity*)> onBegin;
	std::function<void(_Entity*)> onEnd;

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


struct Sequence : public Animation {
	typedef std::vector<Animation*, Eigen::aligned_allocator<Animation*>> AnimationList;
	typedef AnimationList::iterator Iterator;

	Sequence();
	virtual ~Sequence();
	virtual Animation* clone();

	virtual void updateLength();
	virtual void begin(_Entity* entity);
	virtual void update(uint64 time, _Entity* entity);
	virtual void end(_Entity* entity);

	AnimationList anims;
	uint64 baseTime;
	Iterator pos;
};


struct MultiAnim : public Animation {
	typedef std::vector<Animation*, Eigen::aligned_allocator<Animation*>> AnimationList;

	virtual ~MultiAnim();
	virtual Animation* clone();

	virtual void updateLength();
	virtual void begin(_Entity* entity);
	virtual void update(uint64 time, _Entity* entity);
	virtual void end(_Entity* entity);

	AnimationList anims;
};


struct MoveAnim : public Animation {
	MoveAnim(uint64 len, const Vector3& to, AnimType type = ABSOLUTE);
	virtual ~MoveAnim() = default;
	virtual Animation* clone();

	virtual void begin(_Entity* entity);
	virtual void update(uint64 time, _Entity* entity);
	virtual void end(_Entity* entity);

	AnimType type;
	Vector3  from;
	Vector3  to;
};


struct SpriteColorAnim : public Animation {
	SpriteColorAnim(uint64 len, const Vector4& to);
	virtual ~SpriteColorAnim() = default;
	virtual Animation* clone();

	virtual void begin(_Entity* entity);
	virtual void update(uint64 time, _Entity* entity);
	virtual void end(_Entity* entity);

	Vector4  from;
	Vector4  to;
};


struct SwapSpriteAnim : public Animation {
	SwapSpriteAnim(uint64 cycleLen, Sprite* sprite, unsigned nSwap);
	virtual ~SwapSpriteAnim() = default;
	virtual Animation* clone();

	virtual void begin(_Entity* entity);
	virtual void update(uint64 time, _Entity* entity);
	virtual void end(_Entity* entity);

	Sprite* sprite;
	Sprite* sourceSprite;
	unsigned nSwap;
};


class AnimationComponent : public Component {
public:
	AnimationComponent(_Entity* entity, SparseComponentManager<AnimationComponent>* manager);
	AnimationComponent(AnimationComponent&& other);
	~AnimationComponent();

	AnimationComponent& operator=(AnimationComponent other);

	virtual void destroy();
	virtual void clone(EntityRef& target);

	void play(Animation* animation);

	Animation* anim;
	uint64     time;

public:
	AnimationComponentManager* _manager;
};


class AnimationComponentManager : public SparseComponentManager<AnimationComponent> {
public:
	void cloneComponent(EntityRef base, EntityRef entity);

	void update(uint64 etime);
};


#endif
