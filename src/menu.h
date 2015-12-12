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


#ifndef _LOF3_MENU_H
#define _LOF3_MENU_H


#include <lair/core/lair.h>
#include <lair/core/log.h>

#include <lair/utils/input.h>

#include "frame.h"
#include "font.h"


using namespace lair;


struct MenuInputs {
	Input*      down;
	Input*      up;
//	Input*      left;
//	Input*      right;
	Input*      ok;
	Input*      cancel;
};


class Menu {
public:
	typedef std::function<void()> Callback;
	enum {
		HIDDEN,
		DISABLED,
		ENABLED
	};

public:
	Menu(Sprite* bg, Font* font, MenuInputs* inputs,
	     const Callback& cancelCallback = Callback(),
	     const Vector2& size = Vector2::Zero());

	unsigned selected() const;
	void setSelected(unsigned index);

	uint8 entryStatus(unsigned i) const;
	void setEnabled(unsigned i, bool enabled);
	void hideEntry(unsigned i);
	void disableEntry(unsigned i);
	void enableEntry(unsigned i);

	void setLabel(unsigned i, const std::string& label);

	unsigned width() const;
	unsigned height() const;

	unsigned addEntry(const std::string& label, uint8 status = ENABLED,
	                  const Callback& callback = Callback());

	void show(const Vector3& position);
	void hide();

	void update();
	void selectNext();
	void selectPrev();
	void validate();
	void cancel();

	void layout();
	void render(Renderer* renderer);

	Vector2    anchor;
	Vector2    margin;
	float      indent;
	Vector4    textColor;
	Vector4    disabledColor;

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

protected:
	struct Entry {
		std::string label;
		uint8       status;
		Callback    callback;
	};

	typedef std::vector<Entry> EntryList;

protected:
	MenuInputs* _inputs;
	Callback    _cancelCallback;
	unsigned    _selected;
	unsigned    _nSelectable;
	EntryList   _entries;

	bool        _visible;

public:
	Frame       _frame;

protected:
	Font*       _font;
};


#endif
