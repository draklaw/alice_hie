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


#include <cctype>

#include <lair/render_gl2/renderer.h>

#include "font.h"


Font::Font(const Json::Value font, Texture* tex)
    : texture(tex),
      baselineToTop(0),
      _height(),
      _glyphMap() {
	_fontSize = font["size"].asInt();
	_height = font["height"].asInt();
	baselineToTop = _height / 2;

	Vector2 texSize(texture->width(), texture->height());
	for(const Json::Value& c: font["chars"]) {
		unsigned cp = c[0].asInt();
		Glyph g;
		Vector2 pos = Vector2(c[1].asInt(), c[2].asInt()).array()
		        / texSize.array();
		g.size = Vector2(c[3].asInt(), c[4].asInt());
		g.region = Box2(pos, pos + (g.size.array()
		                            / texSize.array()).matrix());
		g.offset = Vector2(c[5].asInt(), c[6].asInt());
		g.advance = c[7].asInt();
		_glyphMap.emplace(cp, g);
	}
}


unsigned Font::textWidth(const std::string& msg) const {
	unsigned w = 0;
	for(char c: msg) {
		auto git = _glyphMap.find(c);
		if(git != _glyphMap.end()) {
			w += git->second.advance;
		}
	}
	return w;
}


std::string Font::layoutText(std::string msg, unsigned maxWidth) const {
	unsigned i = 0;
	unsigned w = 0;
	while(i < msg.size()) {
		unsigned ci = 1;
		unsigned ww = wordWidth(msg, i, &ci);
		if(w + ww > maxWidth) {
			msg[i] = '\n';
			w = 0;
		} else {
			w += ww;
			i = ci;
		}
	}
	return msg;
}


void Font::render(Renderer* renderer, const Vector3& position, const Vector4& color, const std::string& msg,
                  unsigned maxWidth) const {
	Batch& batch = renderer->mainBatch();
	VertexBuffer& buff = batch.getBuffer(
				renderer->spriteShader()->program(),
				texture, renderer->spriteFormat());
	GLuint index = buff.vertexCount();

	Vector4 cursor;
	cursor << position, 0;
	for(unsigned i = 0; i < msg.size(); ++i) {
		char c = msg[i];
		if(c == '\n' ||
		        (std::isspace(c) && cursor.x() - position.x() + wordWidth(msg, i) > maxWidth)) {
			cursor.x() = position.x();
			cursor.y() -= _height;
			continue;
		}
		auto git = _glyphMap.find(c);
		if(git == _glyphMap.end()) {
			continue;
		}

		const Box2& region = git->second.region;
		const Vector2& size = git->second.size;

		if(cursor.x() - position.x() + git->second.advance > maxWidth) {
			cursor.x() = position.x();
			cursor.y() -= _height;
		}

		Vector4 offset = cursor;
		offset[0] += git->second.offset[0];
		offset[1] += _height - size.y() - git->second.offset[1];

		Vector4 v0(       0, size.y(), 0, 1);
		Vector4 v1(       0,        0, 0, 1);
		Vector4 v2(size.x(), size.y(), 0, 1);
		Vector4 v3(size.x(),        0, 0, 1);
		buff.addVertex(SpriteVertex{
				v0 + offset, color, region.corner(Box2::BottomLeft) });
		buff.addVertex(SpriteVertex{
				v1 + offset, color, region.corner(Box2::TopLeft) });
		buff.addVertex(SpriteVertex{
				v2 + offset, color, region.corner(Box2::BottomRight) });
		buff.addVertex(SpriteVertex{
				v3 + offset, color, region.corner(Box2::TopRight) });
		buff.addIndex(index + 0);
		buff.addIndex(index + 1);
		buff.addIndex(index + 2);
		buff.addIndex(index + 2);
		buff.addIndex(index + 1);
		buff.addIndex(index + 3);
		index += 4;

		cursor[0] += git->second.advance;
	}
}


unsigned Font::wordWidth(const std::string& msg, unsigned i, unsigned* ci) const {
	unsigned w = 0;
	do {
		auto git = _glyphMap.find(msg[i]);
		if(git != _glyphMap.end()) {
			w += git->second.advance;
		}
		++i;
	} while(i < msg.size() && !std::isspace(msg[i]));
	if(ci) *ci = i;
	return w;
}
