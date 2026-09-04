#pragma once

#include "common/SpriteDefinition.h"
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace sprite_polygon_lua {

inline std::string formatNumber (float luaUnits)
{
	char buf[32];
	std::snprintf(buf, sizeof(buf), "%.4f", luaUnits);
	std::string s(buf);
	while (s.size() > 1 && s.back() == '0')
		s.pop_back();
	if (!s.empty() && s.back() == '.')
		s += '0';
	if (s == "-0.0")
		return "0.0";
	return s;
}

inline std::string escapeString (const std::string& value)
{
	std::string out;
	out.reserve(value.size());
	for (size_t i = 0; i < value.size(); ++i) {
		const char c = value[i];
		if (c == '\\' || c == '"')
			out.push_back('\\');
		out.push_back(c);
	}
	return out;
}

/**
 * Emit a sprites.lua polygons table. Vertex storage is tile units; Lua uses * 100.
 */
inline std::string toLua (const std::vector<SpritePolygon>& polygons)
{
	std::string out;
	out += "\t\tpolygons = {\n";
	for (size_t p = 0; p < polygons.size(); ++p) {
		const SpritePolygon& poly = polygons[p];
		out += "\t\t\t{\n";
		out += "\t\t\t\t\"";
		out += escapeString(poly.userData);
		out += "\"";
		for (size_t i = 0; i < poly.vertices.size(); ++i) {
			if (i % 4 == 0)
				out += ",\n\t\t\t\t";
			else
				out += ", ";
			out += formatNumber(poly.vertices[i].x * 100.0f);
			out += ", ";
			out += formatNumber(poly.vertices[i].y * 100.0f);
		}
		if (!poly.vertices.empty())
			out += ",\n";
		out += "\t\t\t},\n";
	}
	out += "\t\t},";
	return out;
}

enum TokenType {
	TOKEN_END,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_COMMA,
	TOKEN_EQUALS,
	TOKEN_STRING,
	TOKEN_NUMBER,
	TOKEN_IDENT
};

struct Token {
	TokenType type;
	std::string text;
	float number;
	Token () :
			type(TOKEN_END), number(0.0f)
	{
	}
};

class Lexer {
	const std::string& _s;
	size_t _i;
	Token _peeked;
	bool _hasPeek;
public:
	explicit Lexer (const std::string& s) :
			_s(s), _i(0), _hasPeek(false)
	{
	}

	void skip ()
	{
		for (;;) {
			while (_i < _s.size() && std::isspace(static_cast<unsigned char>(_s[_i])))
				++_i;
			if (_i + 1 < _s.size() && _s[_i] == '-' && _s[_i + 1] == '-') {
				_i += 2;
				while (_i < _s.size() && _s[_i] != '\n')
					++_i;
				continue;
			}
			break;
		}
	}

	Token read ()
	{
		skip();
		Token t;
		if (_i >= _s.size())
			return t;
		const char c = _s[_i];
		if (c == '{') {
			++_i;
			t.type = TOKEN_LBRACE;
			return t;
		}
		if (c == '}') {
			++_i;
			t.type = TOKEN_RBRACE;
			return t;
		}
		if (c == ',') {
			++_i;
			t.type = TOKEN_COMMA;
			return t;
		}
		if (c == '=') {
			++_i;
			t.type = TOKEN_EQUALS;
			return t;
		}
		if (c == '"' || c == '\'') {
			const char quote = c;
			++_i;
			while (_i < _s.size() && _s[_i] != quote) {
				if (_s[_i] == '\\' && _i + 1 < _s.size()) {
					t.text.push_back(_s[_i + 1]);
					_i += 2;
				} else {
					t.text.push_back(_s[_i]);
					++_i;
				}
			}
			if (_i < _s.size())
				++_i;
			t.type = TOKEN_STRING;
			return t;
		}
		if (c == '+' || c == '-' || c == '.' || std::isdigit(static_cast<unsigned char>(c))) {
			const size_t start = _i;
			if (c == '+' || c == '-')
				++_i;
			while (_i < _s.size() && (std::isdigit(static_cast<unsigned char>(_s[_i])) || _s[_i] == '.'))
				++_i;
			if (_i < _s.size() && (_s[_i] == 'e' || _s[_i] == 'E')) {
				++_i;
				if (_i < _s.size() && (_s[_i] == '+' || _s[_i] == '-'))
					++_i;
				while (_i < _s.size() && std::isdigit(static_cast<unsigned char>(_s[_i])))
					++_i;
			}
			t.text = _s.substr(start, _i - start);
			t.number = static_cast<float>(std::atof(t.text.c_str()));
			t.type = TOKEN_NUMBER;
			return t;
		}
		if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
			const size_t start = _i;
			++_i;
			while (_i < _s.size()
					&& (std::isalnum(static_cast<unsigned char>(_s[_i])) || _s[_i] == '_'))
				++_i;
			t.text = _s.substr(start, _i - start);
			t.type = TOKEN_IDENT;
			return t;
		}
		++_i;
		return read();
	}

	Token next ()
	{
		if (_hasPeek) {
			_hasPeek = false;
			return _peeked;
		}
		return read();
	}

	Token peek ()
	{
		if (!_hasPeek) {
			_peeked = read();
			_hasPeek = true;
		}
		return _peeked;
	}
};

inline bool parsePolygonBody (Lexer& lex, SpritePolygon& poly, std::string* error)
{
	bool first = true;
	std::vector<float> coords;
	for (;;) {
		const Token t = lex.peek();
		if (t.type == TOKEN_RBRACE || t.type == TOKEN_END)
			break;
		if (t.type == TOKEN_COMMA) {
			lex.next();
			continue;
		}
		if (first && t.type == TOKEN_STRING) {
			lex.next();
			poly.userData = t.text;
			first = false;
			continue;
		}
		if (t.type != TOKEN_NUMBER) {
			if (error)
				*error = "expected number or '}' in polygon";
			return false;
		}
		lex.next();
		coords.push_back(t.number);
		first = false;
	}
	if (coords.size() % 2 != 0) {
		if (error)
			*error = "polygon has an odd number of coordinates";
		return false;
	}
	for (size_t i = 0; i + 1 < coords.size(); i += 2)
		poly.vertices.push_back(SpriteVertex(coords[i] / 100.0f, coords[i + 1] / 100.0f));
	return true;
}

inline bool fromLua (const std::string& lua, std::vector<SpritePolygon>& out, std::string* error = nullptr)
{
	out.clear();
	Lexer lex(lua);
	Token t = lex.next();
	if (t.type == TOKEN_IDENT && t.text == "polygons") {
		t = lex.next();
		if (t.type != TOKEN_EQUALS) {
			if (error)
				*error = "expected '=' after polygons";
			return false;
		}
		t = lex.next();
	}
	if (t.type != TOKEN_LBRACE) {
		if (error)
			*error = "expected '{' at start of polygon definition";
		return false;
	}

	const Token inner = lex.peek();
	if (inner.type == TOKEN_LBRACE) {
		while (lex.peek().type != TOKEN_RBRACE && lex.peek().type != TOKEN_END) {
			if (lex.peek().type == TOKEN_COMMA) {
				lex.next();
				continue;
			}
			if (lex.peek().type != TOKEN_LBRACE) {
				if (error)
					*error = "expected '{' for a polygon";
				return false;
			}
			lex.next();
			SpritePolygon poly("");
			if (!parsePolygonBody(lex, poly, error))
				return false;
			const Token close = lex.next();
			if (close.type != TOKEN_RBRACE) {
				if (error)
					*error = "expected '}' at end of polygon";
				return false;
			}
			out.push_back(poly);
		}
		const Token close = lex.next();
		if (close.type != TOKEN_RBRACE) {
			if (error)
				*error = "expected '}' at end of polygons table";
			return false;
		}
		return true;
	}

	SpritePolygon poly("");
	if (!parsePolygonBody(lex, poly, error))
		return false;
	const Token close = lex.next();
	if (close.type != TOKEN_RBRACE) {
		if (error)
			*error = "expected '}' at end of polygon";
		return false;
	}
	out.push_back(poly);
	return true;
}

}
