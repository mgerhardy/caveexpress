#include "FrameBuffer.h"

FrameBuffer::FrameBuffer () :
		_framebuffer(0u), _attached(0), _depth(false), _depthRenderBuffer(0u)
{
	_rect.x = _rect.y = _rect.w = _rect.h = 0;
}

FrameBuffer::~FrameBuffer ()
{
	destroy();
}

void FrameBuffer::destroy ()
{
	if (_depth)
		glDeleteRenderbuffers(1, &_depthRenderBuffer);
	glDeleteTextures(_textures.size(), &_textures[0]);
	if (_framebuffer != 0)
		glDeleteFramebuffers(1, &_framebuffer);
	_textures.clear();
	_framebuffer = 0u;
	_depth = false;
	_depthRenderBuffer = 0u;
}

void FrameBuffer::bind ()
{
	if (_framebuffer != 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
		return;
	}

	glGenFramebuffers(1, &_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
}

void FrameBuffer::bind (int x, int y, int w, int h)
{
	_rect.x = x;
	_rect.y = y;
	_rect.w = w;
	_rect.h = h;
	bind();
	glViewport(x, y, w, h);
	glClear(GL_COLOR_BUFFER_BIT | (_depth ? GL_DEPTH_BUFFER_BIT : 0));
	drawBuffer();
}

void FrameBuffer::unbind ()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool FrameBuffer::isSuccessful ()
{
	GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	return error == GL_FRAMEBUFFER_COMPLETE;
}

GLuint FrameBuffer::attachDepthBuffer (int width, int height)
{
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
	_depth = true;
	_depthRenderBuffer = depthrenderbuffer;
	return depthrenderbuffer;
}

GLuint FrameBuffer::attachRenderBuffer (GLenum internalformat, GLenum attachment, GLsizei width, GLsizei height)
{
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, depthrenderbuffer);
	return depthrenderbuffer;
}

void FrameBuffer::attachTexture (GLuint texture, GLenum attachmentType)
{
	if (texture != 0)
		++_attached;
	else
		--_attached;
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, texture, 0);
}

GLuint FrameBuffer::createTexture (GLenum attachmentType, int width, int height)
{
	GLuint texnum;
	glGenTextures(1, &texnum);
	glBindTexture(GL_TEXTURE_2D, texnum);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	attachTexture(texnum, attachmentType);
	_textures.push_back(texnum);
	return texnum;
}

void FrameBuffer::drawBuffer (const GLenum buffer)
{
	const GLenum buffers[] = { buffer };
	drawBuffers(1, buffers);
}

void FrameBuffer::drawBuffers (GLsizei n, const GLenum *buffers)
{
	glDrawBuffers(n, buffers);
}
