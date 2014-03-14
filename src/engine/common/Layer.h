#pragma once

// the layer types - don't change the order
typedef enum {
	LAYER_BACK, LAYER_MIDDLE, LAYER_FRONT, MAX_LAYERS
} Layer;

namespace std {
template<>
struct iterator_traits<Layer> {
	typedef Layer value_type;
	typedef int difference_type;
	typedef Layer *pointer;
	typedef Layer &reference;
	typedef std::bidirectional_iterator_tag iterator_category;
};
}

inline Layer &operator++ (Layer &c)
{
	c = static_cast<Layer>(c + 1);
	return c;
}

inline Layer operator++ (Layer &c, int)
{
	++c;
	return static_cast<Layer>(c - 1);
}

inline Layer &operator-- (Layer &c)
{
	return c = static_cast<Layer>(c - 1);
}

inline Layer operator-- (Layer &c, int)
{
	--c;
	return static_cast<Layer>(c + 1);
}

inline Layer operator* (Layer c)
{
	return c;
}

#define LAYER_DEFAULT LAYER_MIDDLE
