#include "ByteStream.h"
#include "engine/common/String.h"

ByteStream::ByteStream () :
		_size(0)
{
}

ByteStream::~ByteStream ()
{
	_buffer.clear();
}

void ByteStream::checkSize () const
{
	if (_size != _buffer.size()) {
		_size = _buffer.size();
		_buffer.resize(_size);
	}
}

int32_t ByteStream::peekInt() const {
	if (_buffer.empty())
		return -1;
	uint8_t buf[4];
	const int l = sizeof(buf);
	VectorBuffer::const_iterator it = _buffer.begin();
	for (int i = 0; i < l; ++i) {
		if (it == _buffer.end())
			return -1;
		buf[i] = *it;
		++it;
	}
	const int32_t *word = (const int32_t*) (void*) &buf;
	const int32_t val = SDL_SwapLE32(*word);
	return val;
}

int16_t ByteStream::peekShort() const {
	if (_buffer.empty())
		return -1;
	uint8_t buf[2];
	const int l = sizeof(buf);
	VectorBuffer::const_iterator it = _buffer.begin();
	for (int i = 0; i < l; ++i) {
		if (it == _buffer.end())
			return -1;
		buf[i] = *it;
		++it;
	}
	const int16_t *word = (const int16_t*) (void*) &buf;
	const int16_t val = SDL_SwapLE16(*word);
	return val;
}

void ByteStream::addFormat (const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	while (*fmt) {
		const char typeID = *fmt++;
		switch (typeID) {
		case 'b':
			addByte((uint8_t) va_arg(ap, int));
			break;
		case 's':
			addShort((uint16_t) va_arg(ap, int));
			break;
		case 'i':
			addInt((uint32_t) va_arg(ap, int));
			break;
		default:
			System.exit("illegal format string character", 1);
		}
	}

	va_end(ap);
}

void ByteStream::readFormat (const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	while (*fmt) {
		const char typeID = *fmt++;
		switch (typeID) {
		case 'b':
			*va_arg(ap, int *) = readByte();
			break;
		case 's':
			*va_arg(ap, int *) = readShort();
			break;
		case 'i':
			*va_arg(ap, int *) = readInt();
			break;
		default:
			System.exit("illegal format string character", 1);
		}
	}

	va_end(ap);
}

std::string ByteStream::readString ()
{
	unsigned int size = 0;
	std::string strbuff;
	strbuff.reserve(64);
	for (;;) {
		const char chr = *(_buffer.begin() + size);
		++size;
		if (size > _buffer.size())
			System.exit(String::format("invalid string in readString - size (%i) is bigger than the buffer size (%i)", size, _buffer.size()), 1);
		if (chr == '\0')
			break;
		strbuff += chr;
	}
	_buffer.erase(_buffer.begin(), _buffer.begin() + size);
	_size -= size;
	return strbuff;
}
