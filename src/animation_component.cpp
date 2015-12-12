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


#include <lair/render_gl2/renderer.h>

#include <lair/ec/sprite_component.h>

#include "font.h"

#include "animation_component.h"


void Animation::updateLength() {
}


void Animation::begin(_Entity* entity) {
	if(onBegin) {
		onBegin(entity);
	}
}


void Animation::end(_Entity* entity) {
	if(onEnd) {
		onEnd(entity);
	}
}


//---------------------------------------------------------------------------//


Sequence::Sequence()
    : pos(anims.end()) {
}


Sequence::~Sequence() {
	for(Animation* a: anims) {
		delete a;
	}
}


Animation* Sequence::clone() {
	Sequence* seq = new Sequence;
	seq->length = length;
	seq->onBegin = onBegin;
	seq->onEnd = onEnd;
	for(Animation* a: anims) {
		seq->anims.push_back(a->clone());
	}
	seq->baseTime = baseTime;
	seq->pos = seq->anims.begin() + (pos - anims.begin());
	return seq;
}


void Sequence::updateLength() {
	length = 0;
	for(Animation* a: anims) {
		a->updateLength();
		length += a->length;
	}
}


void Sequence::begin(_Entity* entity) {
	Animation::begin(entity);
	baseTime = 0;
	pos = anims.begin();
	if(pos != anims.end()) {
		(*pos)->begin(entity);
	}
}


void Sequence::update(uint64 time, _Entity* entity) {
	uint64 subTime = time - baseTime;
	while(pos != anims.end() && subTime >= (*pos)->length) {
		(*pos)->end(entity);
		subTime -= (*pos)->length;
		++pos;
		if(pos != anims.end()) {
			(*pos)->begin(entity);
		}
	}
	if(pos < anims.end()) {
		(*pos)->update(subTime, entity);
	}
	baseTime = time - subTime;
}


void Sequence::end(_Entity* entity) {
	update(length + 1, entity);
	Animation::end(entity);
}


//---------------------------------------------------------------------------//


MultiAnim::~MultiAnim() {
	for(Animation* a: anims) {
		delete a;
	}
}


Animation* MultiAnim::clone() {
	MultiAnim* ma = new MultiAnim;
	ma->length = length;
	ma->onBegin = onBegin;
	ma->onEnd = onEnd;
	for(Animation* a: anims) {
		ma->anims.push_back(a->clone());
	}
	return ma;
}


void MultiAnim::updateLength() {
	length = 0;
	for(Animation* a: anims) {
		a->updateLength();
		length = std::max(length, a->length);
	}
}


void MultiAnim::begin(_Entity* entity) {
	Animation::begin(entity);
	for(Animation* a: anims) {
		a->begin(entity);
	}
}


void MultiAnim::update(uint64 time, _Entity* entity) {
	for(Animation* a: anims) {
		a->update(time, entity);
	}
}


void MultiAnim::end(_Entity* entity) {
	for(Animation* a: anims) {
		a->end(entity);
	}
	Animation::end(entity);
}


//---------------------------------------------------------------------------//


MoveAnim::MoveAnim(uint64 len, const Vector3& to, AnimType type)
	: type(type),
      to(to) {
	length = len;
}


Animation* MoveAnim::clone() {
	MoveAnim* move = new MoveAnim(length, to, type);
	move->onBegin = onBegin;
	move->onEnd = onEnd;
	move->from = from;
	return move;
}


void MoveAnim::begin(_Entity* entity) {
	Animation::begin(entity);
	from = entity->transform.translation();
}


void MoveAnim::update(uint64 time, _Entity* entity) {
	float x = float(time) / float(length);
	Vector3 target = (type == ABSOLUTE)? to: to + from;
	if(x < 1) {
		entity->transform.translation() = lerp(x, from, target);
	} else {
		end(entity);
	}
}


void MoveAnim::end(_Entity* entity) {
	Vector3 target = (type == ABSOLUTE)? to: to + from;
	entity->transform.translation() = target;
	Animation::end(entity);
}


//---------------------------------------------------------------------------//


SpriteColorAnim::SpriteColorAnim(uint64 len, const Vector4& to)
	: to(to) {
	length = len;
}


Animation* SpriteColorAnim::clone() {
	SpriteColorAnim* move = new SpriteColorAnim(length, to);
	move->onBegin = onBegin;
	move->onEnd = onEnd;
	move->from = from;
	return move;
}


void SpriteColorAnim::begin(_Entity* entity) {
	Animation::begin(entity);
	from = entity->sprite->color();
}


void SpriteColorAnim::update(uint64 time, _Entity* entity) {
	float x = float(time) / float(length);
	if(x < 1) {
		entity->sprite->setColor(lerp(x, from, to));
	} else {
		end(entity);
	}
}


void SpriteColorAnim::end(_Entity* entity) {
	entity->sprite->setColor(to);
	Animation::end(entity);
}


//---------------------------------------------------------------------------//


SwapSpriteAnim::SwapSpriteAnim(uint64 cycleLen, Sprite* sprite, unsigned nSwap)
    : sprite(sprite),
      sourceSprite(nullptr),
      nSwap(nSwap) {
	length = cycleLen * nSwap;
}


Animation* SwapSpriteAnim::clone() {
	SwapSpriteAnim* anim = new SwapSpriteAnim(length / nSwap, sprite, nSwap);
	anim->onBegin = onBegin;
	anim->onEnd = onEnd;
	anim->sourceSprite = sourceSprite;
	return anim;
}


void SwapSpriteAnim::begin(_Entity* entity) {
	Animation::begin(entity);
	sourceSprite = entity->sprite->sprite();
}


void SwapSpriteAnim::update(uint64 time, _Entity* entity) {
	unsigned swap = time / (length / nSwap);
	entity->sprite->setSprite((swap%2)? sprite: sourceSprite);
}


void SwapSpriteAnim::end(_Entity* entity) {
	entity->sprite->setSprite((nSwap%2)? sprite: sourceSprite);
}


//---------------------------------------------------------------------------//


AnimationComponent::AnimationComponent(_Entity* entity,
                             SparseComponentManager<AnimationComponent>* manager)
    : Component(entity),
      anim(nullptr),
      time(0),
      _manager(static_cast<AnimationComponentManager*>(manager)) {
}


AnimationComponent::AnimationComponent(AnimationComponent&& other)
    : Component(other._entityPtr),
      anim(other.anim),
      time(other.time),
      _manager(other._manager) {
	other.anim = nullptr;
}


AnimationComponent::~AnimationComponent() {
	delete anim;
}


AnimationComponent& AnimationComponent::operator=(AnimationComponent other) {
	std::swap(_entityPtr, other._entityPtr);
	std::swap(anim, other.anim);
	std::swap(time, other.time);
	std::swap(_manager, other._manager);
	return *this;
}


void AnimationComponent::destroy() {
	_manager->removeComponent(EntityRef(_entity()));
}


void AnimationComponent::clone(EntityRef& target) {
	_manager->cloneComponent(EntityRef(_entity()), target);
}


void AnimationComponent::play(Animation* animation) {
	if(anim) {
		anim->end(_entity());
	}
	anim = animation;
	time = 0;
}


//---------------------------------------------------------------------------//


void AnimationComponentManager::cloneComponent(EntityRef base, EntityRef entity) {
	addComponent(entity);
	AnimationComponent* baseComp = get(base);
	AnimationComponent* comp = get(entity);
	comp->anim = baseComp->anim->clone();
	comp->time = baseComp->time;
}


void AnimationComponentManager::update(uint64 etime) {
	_collectGarbages();
	for(auto& entityComp: *this) {
		AnimationComponent& comp = entityComp.second;
		if(!comp._alive) {
			continue;
		}
		if(comp.anim) {
			if(comp.time == 0) {
				comp.anim->updateLength();
				comp.anim->begin(comp._entity());
			}
			comp.time += etime;
			if(comp.time < comp.anim->length) {
				comp.anim->update(comp.time, comp._entity());
			} else {
				comp.anim->end(comp._entity());
				comp.anim = nullptr;
			}
		}
	}
}
