#pragma once

#include "common/SpritePolygonLua.h"
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

namespace sprite_lua_patcher {

inline void skipCommentAndSpace (const std::string& s, size_t& i)
{
	for (;;) {
		while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])))
			++i;
		if (i + 1 < s.size() && s[i] == '-' && s[i + 1] == '-') {
			i += 2;
			while (i < s.size() && s[i] != '\n')
				++i;
			continue;
		}
		break;
	}
}

inline size_t matchingBrace (const std::string& s, size_t open)
{
	if (open >= s.size() || s[open] != '{')
		return std::string::npos;
	int depth = 0;
	bool inStr = false;
	char quote = 0;
	for (size_t i = open; i < s.size(); ++i) {
		const char c = s[i];
		if (inStr) {
			if (c == '\\' && i + 1 < s.size()) {
				++i;
				continue;
			}
			if (c == quote)
				inStr = false;
			continue;
		}
		if (c == '-' && i + 1 < s.size() && s[i + 1] == '-') {
			i += 1;
			while (i + 1 < s.size() && s[i + 1] != '\n')
				++i;
			continue;
		}
		if (c == '"' || c == '\'') {
			inStr = true;
			quote = c;
			continue;
		}
		if (c == '{')
			++depth;
		else if (c == '}') {
			--depth;
			if (depth == 0)
				return i;
		}
	}
	return std::string::npos;
}

inline bool findSpriteTable (const std::string& lua, const std::string& spriteId, size_t& tableOpen, size_t& tableClose)
{
	const std::string needle = "[\"" + spriteId + "\"]";
	size_t pos = 0;
	while ((pos = lua.find(needle, pos)) != std::string::npos) {
		size_t i = pos + needle.size();
		skipCommentAndSpace(lua, i);
		if (i < lua.size() && lua[i] == '=') {
			++i;
			skipCommentAndSpace(lua, i);
			if (i < lua.size() && lua[i] == '{') {
				const size_t close = matchingBrace(lua, i);
				if (close == std::string::npos)
					return false;
				tableOpen = i;
				tableClose = close;
				return true;
			}
		}
		pos += needle.size();
	}
	return false;
}

inline bool findAssignmentTable (const std::string& lua, const std::string& name, size_t& tableOpen, size_t& tableClose)
{
	if (name.empty())
		return false;
	size_t i = 0;
	bool inStr = false;
	char quote = 0;
	while (i < lua.size()) {
		const char c = lua[i];
		if (inStr) {
			if (c == '\\' && i + 1 < lua.size()) {
				i += 2;
				continue;
			}
			if (c == quote)
				inStr = false;
			++i;
			continue;
		}
		if (c == '-' && i + 1 < lua.size() && lua[i + 1] == '-') {
			i += 2;
			while (i < lua.size() && lua[i] != '\n')
				++i;
			continue;
		}
		if (c == '"' || c == '\'') {
			inStr = true;
			quote = c;
			++i;
			continue;
		}
		if (lua.compare(i, name.size(), name) == 0) {
			const bool startOk = i == 0
					|| !(std::isalnum(static_cast<unsigned char>(lua[i - 1])) || lua[i - 1] == '_');
			const size_t end = i + name.size();
			const bool endOk = end >= lua.size()
					|| !(std::isalnum(static_cast<unsigned char>(lua[end])) || lua[end] == '_');
			if (startOk && endOk) {
				size_t j = end;
				skipCommentAndSpace(lua, j);
				if (j < lua.size() && lua[j] == '=') {
					++j;
					skipCommentAndSpace(lua, j);
					if (j < lua.size() && lua[j] == '{') {
						const size_t close = matchingBrace(lua, j);
						if (close == std::string::npos)
							return false;
						tableOpen = j;
						tableClose = close;
						return true;
					}
				}
			}
		}
		++i;
	}
	return false;
}

inline size_t lineIndentStart (const std::string& s, size_t pos)
{
	size_t i = pos;
	while (i > 0 && (s[i - 1] == ' ' || s[i - 1] == '\t'))
		--i;
	return i;
}

inline bool needsCommaBefore (const std::string& s, size_t insertAt)
{
	size_t i = insertAt;
	while (i > 0 && std::isspace(static_cast<unsigned char>(s[i - 1])))
		--i;
	if (i == 0)
		return false;
	const char prev = s[i - 1];
	return prev != '{' && prev != ',';
}

inline bool findTableKey (const std::string& lua, size_t tableOpen, size_t tableClose, const char* key,
		size_t& keyStart, size_t& valueClose)
{
	const size_t keyLen = std::strlen(key);
	size_t i = tableOpen + 1;
	int depth = 1;
	bool inStr = false;
	char quote = 0;
	while (i < tableClose) {
		const char c = lua[i];
		if (inStr) {
			if (c == '\\' && i + 1 < lua.size()) {
				i += 2;
				continue;
			}
			if (c == quote)
				inStr = false;
			++i;
			continue;
		}
		if (c == '-' && i + 1 < lua.size() && lua[i + 1] == '-') {
			i += 2;
			while (i < lua.size() && lua[i] != '\n')
				++i;
			continue;
		}
		if (c == '"' || c == '\'') {
			inStr = true;
			quote = c;
			++i;
			continue;
		}
		if (c == '{') {
			++depth;
			++i;
			continue;
		}
		if (c == '}') {
			--depth;
			++i;
			continue;
		}
		if (depth == 1 && lua.compare(i, keyLen, key) == 0) {
			const bool startOk = i == 0 || !(std::isalnum(static_cast<unsigned char>(lua[i - 1])) || lua[i - 1] == '_');
			const bool endOk = i + keyLen >= lua.size()
					|| !(std::isalnum(static_cast<unsigned char>(lua[i + keyLen])) || lua[i + keyLen] == '_');
			if (startOk && endOk) {
				size_t j = i + keyLen;
				skipCommentAndSpace(lua, j);
				if (j < lua.size() && lua[j] == '=') {
					++j;
					skipCommentAndSpace(lua, j);
					if (j < lua.size() && lua[j] == '{') {
						const size_t close = matchingBrace(lua, j);
						if (close == std::string::npos || close > tableClose)
							return false;
						keyStart = lineIndentStart(lua, i);
						valueClose = close;
						return true;
					}
				}
			}
		}
		++i;
	}
	return false;
}

inline bool verifyPatchedShapes (const std::string& lua, const std::string& spriteId,
		const std::vector<SpritePolygon>& polygons, const std::vector<SpriteCircle>& circles,
		std::string* error)
{
	size_t tableOpen = 0;
	size_t tableClose = 0;
	if (!findSpriteTable(lua, spriteId, tableOpen, tableClose)) {
		if (error)
			*error = "sprite table missing after patch";
		return false;
	}
	size_t keyStart = 0;
	size_t valueClose = 0;
	const bool hasPolyKey = findTableKey(lua, tableOpen, tableClose, "polygons", keyStart, valueClose);
	const bool hasCircleKey = findTableKey(lua, tableOpen, tableClose, "circles", keyStart, valueClose);
	if (polygons.empty() && hasPolyKey) {
		if (error)
			*error = "empty polygons left a polygons key";
		return false;
	}
	if (circles.empty() && hasCircleKey) {
		if (error)
			*error = "empty circles left a circles key";
		return false;
	}
	if (polygons.empty() && circles.empty())
		return true;

	const std::string body = lua.substr(tableOpen, tableClose - tableOpen + 1);
	std::vector<SpritePolygon> parsedPolys;
	std::vector<SpriteCircle> parsedCircles;
	std::string parseErr;
	if (!sprite_polygon_lua::fromLuaShapes(body, parsedPolys, parsedCircles, &parseErr)) {
		if (error)
			*error = parseErr.empty() ? "patched shapes are not valid Lua" : parseErr;
		return false;
	}
	if (parsedPolys.size() != polygons.size() || parsedCircles.size() != circles.size()) {
		if (error)
			*error = "patched shape count mismatch";
		return false;
	}
	return true;
}

inline bool insertShapeBlock (std::string& out, size_t& tableOpen, size_t& tableClose, const std::string& spriteId,
		const std::string& block, std::string* error)
{
	if (block.empty())
		return true;
	size_t insertAt = lineIndentStart(out, tableClose);
	std::string text = block;
	if (!text.empty() && text.back() != '\n')
		text += "\n";
	if (needsCommaBefore(out, insertAt)) {
		size_t commaAt = insertAt;
		while (commaAt > 0 && std::isspace(static_cast<unsigned char>(out[commaAt - 1])))
			--commaAt;
		out.insert(commaAt, ",");
		insertAt += 1;
		if (!findSpriteTable(out, spriteId, tableOpen, tableClose)) {
			if (error)
				*error = "failed to re-locate sprite table after comma insert";
			return false;
		}
		insertAt = lineIndentStart(out, tableClose);
	}
	out.insert(insertAt, text);
	if (!findSpriteTable(out, spriteId, tableOpen, tableClose)) {
		if (error)
			*error = "failed to re-locate sprite table after insert";
		return false;
	}
	return true;
}

/**
 * Replace or insert polygons/circles for one sprite id. Leaves comments and other keys intact.
 */
inline bool patchSpriteShapes (const std::string& lua, const std::string& spriteId,
		const std::vector<SpritePolygon>& polygons, const std::vector<SpriteCircle>& circles,
		std::string& out, std::string* error)
{
	size_t tableOpen = 0;
	size_t tableClose = 0;
	if (!findSpriteTable(lua, spriteId, tableOpen, tableClose)) {
		if (error)
			*error = "sprite id not found in sprites.lua";
		return false;
	}

	out = lua;
	auto replaceOrErase = [&] (const char* key, const std::string& replacement) {
		size_t keyStart = 0;
		size_t valueClose = 0;
		if (!findTableKey(out, tableOpen, tableClose, key, keyStart, valueClose))
			return false;
		size_t eraseEnd = valueClose + 1;
		if (eraseEnd < out.size() && out[eraseEnd] == ',')
			++eraseEnd;
		while (eraseEnd < out.size() && (out[eraseEnd] == '\r' || out[eraseEnd] == ' ' || out[eraseEnd] == '\t'))
			++eraseEnd;
		if (eraseEnd < out.size() && out[eraseEnd] == '\n')
			++eraseEnd;
		if (replacement.empty()) {
			out.erase(keyStart, eraseEnd - keyStart);
		} else {
			std::string block = replacement;
			if (!block.empty() && block.back() != '\n')
				block += "\n";
			out.replace(keyStart, eraseEnd - keyStart, block);
		}
		if (!findSpriteTable(out, spriteId, tableOpen, tableClose))
			return false;
		return true;
	};

	const std::string polyLua = polygons.empty() ? "" : sprite_polygon_lua::toLua(polygons);
	const std::string circleLua = circles.empty() ? "" : sprite_polygon_lua::toLuaCircles(circles);

	if (!replaceOrErase("polygons", polyLua) && !polyLua.empty()) {
		if (!insertShapeBlock(out, tableOpen, tableClose, spriteId, polyLua, error))
			return false;
	}
	if (!replaceOrErase("circles", circleLua) && !circleLua.empty()) {
		if (!insertShapeBlock(out, tableOpen, tableClose, spriteId, circleLua, error))
			return false;
	}
	if (!verifyPatchedShapes(out, spriteId, polygons, circles, error))
		return false;
	return true;
}

}
