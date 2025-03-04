#pragma once
#include "framebuffer.h"

namespace fb
{

	FrameBuffer createFrameBuffer(unsigned int width, unsigned int height, unsigned int colorFormat, DepthType type)
	{
		fb::FrameBuffer buffer;
		buffer.width = width;
		buffer.hight = height;

		//buffer code
		glGenFramebuffers(1, &buffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);

		//create color
		glGenTextures(1, &buffer.colorBuffer[0]);

		glBindTexture(GL_TEXTURE_2D, buffer.colorBuffer[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, colorFormat, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



		if (type == RENDER_BUFFER)
		{
			//create depth
			glGenRenderbuffers(1, &buffer.depthBuffer);

			glBindRenderbuffer(GL_RENDERBUFFER, buffer.depthBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);


			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer.depthBuffer);
		}
		else if (type == TEXTURE)
		{
			glGenTextures(1, &buffer.depthBuffer);

			glBindTexture(GL_TEXTURE_2D, buffer.depthBuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, buffer.depthBuffer, 0);
		}
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.colorBuffer[0], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("frame buffer is not complete!");
			return FrameBuffer();

		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return buffer;
	}

	FrameBuffer createHDR_FramBuffer(unsigned int width, unsigned int height)
	{

		fb::FrameBuffer buffer;
		buffer.width = width;
		buffer.hight = height;

		glGenFramebuffers(1, &buffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);

		glGenTextures(1, &buffer.colorBuffer[0]);

		glBindTexture(GL_TEXTURE_2D, buffer.colorBuffer[0]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.colorBuffer[0], 0);

		glGenTextures(1, &buffer.colorBuffer[1]);

		glBindTexture(GL_TEXTURE_2D, buffer.colorBuffer[1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffer.colorBuffer[1], 0);

		GLuint att[2] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, att);

		glGenTextures(1, &buffer.depthBuffer);

		glBindTexture(GL_TEXTURE_2D, buffer.depthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, buffer.depthBuffer, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("frame buffer is not complete!");
			return FrameBuffer();

		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return buffer;
	}

	FrameBuffer createShadowBuffer(unsigned int shdw_width, unsigned int shdw_hight)
	{
		FrameBuffer buffer;

		glGenFramebuffers(1, &buffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, buffer.fbo);

		//create and bind depth
		glGenTextures(1, &buffer.depthBuffer);
		glBindTexture(GL_TEXTURE_2D, buffer.depthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			shdw_width, shdw_hight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthBuffer, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("frame buffer is not complete!");
			return FrameBuffer();

		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return buffer;

	}
}