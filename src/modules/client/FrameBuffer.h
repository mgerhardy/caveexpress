#pragma once

#include "GLFunc.h"
#include "textures/Texture.h"
#include <vector>

class FrameBuffer {
private:
	GLuint _framebuffer;
	int _attached;
	bool _depth;
	GLuint _depthRenderBuffer;
	std::vector<GLuint> _textures;
	TextureRect _rect;
public:
	FrameBuffer ();
	~FrameBuffer ();

	void destroy ();

	inline const TextureRect& rect() const {
		return _rect;
	}

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
	GLuint attachDepthBuffer (int width, int height);
	GLuint attachRenderBuffer (GLenum internalformat, GLenum attachment, GLsizei width, GLsizei height);
	void drawBuffers (GLsizei n, const GLenum *buffers);
	void drawBuffer (const GLenum buffer = GL_COLOR_ATTACHMENT0);
};
