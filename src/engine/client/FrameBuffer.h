#pragma once

#include "GLFunc.h"

class FrameBuffer {
private:
	GLuint _framebuffer;
	int _attached;
public:
	FrameBuffer ();
	~FrameBuffer ();

	bool isSuccessful ();

	/**
	 * @note Call glViewport after the framebuffer was bound
	 */
	void bind ();
	void bind (int x, int y, int w, int h);
	void unbind ();
	/**
	 * @note Make sure you've bound the correct fbo before
	 * @param[in] texture The texture handle
	 * @param[in] attachmentType Possible values are @c GL_COLOR_ATTACHMENT0..GL_COLOR_ATTACHMENTn, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT
	 */
	void attachTexture (GLuint texture, GLenum attachmentType);
	/**
	 * @note Make sure you've bound the correct fbo before
	 * @param[in] attachmentType Possible values are @c GL_COLOR_ATTACHMENT0..GL_COLOR_ATTACHMENTn, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT
	 */
	GLuint createTexture (GLenum attachmentType, int width, int height);
	void attachDepthBuffer (int width, int height);
	void attachRenderBuffer (GLenum internalformat, GLenum attachment, GLsizei width, GLsizei height);
	void drawBuffers (GLsizei n, const GLenum *buffers);
	void drawBuffer (const GLenum buffer = GL_COLOR_ATTACHMENT0);
};
