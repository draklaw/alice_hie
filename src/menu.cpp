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

#include "menu.h"


Menu::Menu(Sprite* bg, Font* font, MenuInputs* inputs,
           const Callback& cancelCallback, const Vector2& size)
    : anchor(0, 0),
      margin(12, 12),
      indent(12),
      textColor(1, 1, 1, 1),
      disabledColor(.5, .5, .5, 1),
      _inputs(inputs),
      _cancelCallback(cancelCallback),
      _selected(0),
      _nSelectable(0),
      _entries(),
      _visible(false),
      _frame(bg, size),
      _font(font) {
	lairAssert(bg);
}


unsigned Menu::selected() const {
	return _selected;
}


void Menu::setSelected(unsigned index) {
	lairAssert(index < _entries.size());
	_selected = index;
}


uint8 Menu::entryStatus(unsigned i) const {
	lairAssert(i < _entries.size());
	return _entries[i].status;
}


void Menu::setEnabled(unsigned i, bool enabled) {
	if(enabled) {
		enableEntry(i);
	} else {
		disableEntry(i);
	}
}


void Menu::hideEntry(unsigned i) {
	lairAssert(i < _entries.size());
	if(_entries[i].status == ENABLED) {
		--_nSelectable;
		if(_selected == i) {
			selectNext();
		}
	}
	_entries[i].status = HIDDEN;
}


void Menu::disableEntry(unsigned i) {
	lairAssert(i < _entries.size());
	if(_entries[i].status == ENABLED) {
		--_nSelectable;
		if(_selected == i) {
			selectNext();
		}
	}
	_entries[i].status = DISABLED;
}


void Menu::enableEntry(unsigned i) {
	lairAssert(i < _entries.size());
	if(_entries[i].status != ENABLED) {
		++_nSelectable;
		if(_nSelectable == 1) {
			_selected = i;
		}
	}
	_entries[i].status = ENABLED;
}


void Menu::setLabel(unsigned i, const std::string& label) {
	_entries[i].label = label;
}


unsigned Menu::width() const {
	return _frame.size.x();
}


unsigned Menu::height() const {
	return _frame.size.y();
}


unsigned Menu::addEntry(const std::string& label, uint8 status,
                            const Callback& callback) {
	_entries.emplace_back(Entry{
	    label,
	    status,
	    callback
	});
	if(status == ENABLED) {
		++_nSelectable;
	}
	return _entries.size() - 1;
}


void Menu::show(const Vector3& position) {
	_visible  = true;
	_frame.position = position;
}


void Menu::hide() {
	_visible = false;
}


void Menu::layout() {
	unsigned textWidth = 0;
	for(const Entry& e: _entries) {
		textWidth = std::max(textWidth, _font->textWidth(e.label));
	}
	_frame.size = Vector2(
	            margin.x() * 2 + indent + textWidth,
	            margin.y() * 2 + _font->height() * _entries.size());
}


void Menu::update() {
	if(_inputs->down->justPressed()) {
		selectNext();
	} else if(_inputs->up->justPressed()) {
		selectPrev();
	} else if(_inputs->ok->justPressed()) {
		validate();
	} else if(_inputs->cancel->justPressed()) {
		cancel();
	}
}


void Menu::selectNext() {
	if(_nSelectable != 0) {
		for(unsigned i = 1; i < _entries.size(); ++i) {
			unsigned index = (i + _selected) % _entries.size();
			if(entryStatus(index) == ENABLED) {
				_selected = index;
				return;
			}
		}
	}
}


void Menu::selectPrev() {
	if(_nSelectable != 0) {
		for(unsigned i = 1; i < _entries.size(); ++i) {
			unsigned index = (_entries.size() - i + _selected) % _entries.size();
			if(entryStatus(index) == ENABLED) {
				_selected = index;
				return;
			}
		}
	}
}


void Menu::validate() {
	if(_nSelectable != 0 && entryStatus(_selected) == ENABLED
	        && _entries[_selected].callback) {
		_entries[_selected].callback();
	}
}


void Menu::cancel() {
	if(_cancelCallback) {
		_cancelCallback();
	}
}


void Menu::render(Renderer* renderer) {
	if(!_visible) {
		return;
	}

	_frame.render(renderer);

	Vector3 pos = _frame.position;
	pos[0] += margin.x();
	pos[1] += _frame.size.y() - _font->baselineToTop - margin.y();
	pos[2] += 0.001;
	Vector3 offset(indent, 0, 0);
	for(unsigned i = 0; i < _entries.size(); ++i) {
		if(_entries[i].status != HIDDEN) {
			Vector4 color = (_entries[i].status == ENABLED)?
			            textColor: disabledColor;
			if(i == _selected && entryStatus(i) == ENABLED) {
				_font->render(renderer, pos, color, ">");
			}
			_font->render(renderer, pos + offset, color, _entries[i].label);
			pos.y() -= _font->height();
		}
	}
}
