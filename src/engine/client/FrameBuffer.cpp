#include "FrameBuffer.h"

FrameBuffer::FrameBuffer () :
		_framebuffer(0), _attached(0)
{
}

FrameBuffer::~FrameBuffer ()
{
	if (_framebuffer != 0)
		glDeleteFramebuffers(1, &_framebuffer);
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
	bind();
	glViewport(x, y, w, h);
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

void FrameBuffer::attachDepthBuffer (int width, int height)
{
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
}

void FrameBuffer::attachRenderBuffer (GLenum internalformat, GLenum attachment, GLsizei width, GLsizei height)
{
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, depthrenderbuffer);
}

void FrameBuffer::attachTexture (GLuint texture, GLenum attachmentType)
{
	if (texture != 0)
		++_attached;
	else
		--_attached;
	glBindTexture(GL_TEXTURE_2D, texture);
	glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, texture, 0);
}

GLuint FrameBuffer::createTexture (GLenum attachmentType, int width, int height)
{
	GLuint texnum;
	glGenTextures(1, &texnum);
	glBindTexture(GL_TEXTURE_2D, texnum);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	attachTexture(texnum, attachmentType);
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
