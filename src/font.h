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


#ifndef _LOF3_FONT_H
#define _LOF3_FONT_H


#include <unordered_map>

#include <lair/core/lair.h>
#include <lair/core/log.h>


using namespace lair;


namespace lair {
class Texture;
class Renderer;
}


class Font {
public:
	Font(const Json::Value font, Texture* tex);

	unsigned fontSize() const { return _fontSize; }
	unsigned height()   const { return _height; }

	unsigned textWidth(const std::string& msg) const;
	std::string layoutText(std::string msg, unsigned maxWidth) const;

	void render(Renderer* renderer, const Vector3& position, const Vector4& color,
	            const std::string& msg, unsigned maxWidth = 999999) const;

	Texture*    texture;
	unsigned    baselineToTop;

protected:
	struct Glyph {
		Box2     region;
		Vector2  size;
		Vector2  offset;
		unsigned advance;
	};

	typedef std::unordered_map<unsigned, Glyph, std::hash<unsigned>, std::equal_to<unsigned>,
	                           Eigen::aligned_allocator<std::pair<unsigned, Glyph>>> GlyphMap;

protected:
	unsigned wordWidth(const std::string& msg, unsigned i, unsigned* ci = nullptr) const;

protected:
	unsigned _fontSize;
	unsigned _height;
	GlyphMap _glyphMap;
};


#endif
