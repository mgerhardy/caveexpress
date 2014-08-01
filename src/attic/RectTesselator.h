#pragma once

#include "engine/common/Math.h"
#include <stdint.h>
#include <vector>

class RectTesselator {
public:
	typedef std::vector<int> Indices;
	typedef Indices::const_iterator IndicesIter;
	typedef std::vector<glm::vec2> Vertices;
	typedef Vertices::const_iterator VerticesIter;
	typedef std::vector<glm::vec2> Texcoords;
	typedef Texcoords::const_iterator TexcoordsIter;
private:
	Indices _indices;
	Texcoords _texcoords;
	Vertices _vertices;
public:
	RectTesselator (uint32_t tesselation = 10);
	virtual ~RectTesselator ();

	const Vertices getVertices () const;
	const Indices getIndices () const;
	const Texcoords getTexcoords () const;
};

inline const RectTesselator::Vertices RectTesselator::getVertices () const
{
	return _vertices;
}

inline const RectTesselator::Indices RectTesselator::getIndices () const
{
	return _indices;
}

inline const RectTesselator::Texcoords RectTesselator::getTexcoords () const
{
	return _texcoords;
}
