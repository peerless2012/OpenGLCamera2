/**
 *
 * Created by 公众号：字节流动 on 2022/4/18.
 * https://github.com/githubhaohao/NDK_OpenGLES_3_0
 * 最新文章首发于公众号：字节流动，有疑问或者技术交流可以添加微信 Byte-Flow ,领取视频教程, 拉你进技术交流群
 *
 * */

#include <gtc/matrix_transform.hpp>
#include <ImageDef.h>
#include "ConveyorBeltExample_1.h"
#include "GLUtils.h"

ConveyorBeltExample_1::ConveyorBeltExample_1()
{

	m_MVPMatLoc = GL_NONE;

    m_TextureId = GL_NONE;
	m_VaoId0 = GL_NONE;
	m_VaoId1 = GL_NONE;

    m_frameIndex = 0;
	m_loopCount = 0;
}

ConveyorBeltExample_1::~ConveyorBeltExample_1()
{
    NativeImageUtil::FreeNativeImage(&m_RenderImage);
    NativeImageUtil::FreeNativeImage(&m_SrcImage);
}

void ConveyorBeltExample_1::Init()
{
	if(m_RenderImage.ppPlane[0] == nullptr || m_ProgramObj)
		return;

    LOGCATE("ConveyorBeltExample_1::Init");

    glGenTextures(1, &m_TextureId);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    char vShaderStr[] =
            "#version 300 es\n"
            "layout(location = 0) in vec4 a_position;\n"
            "layout(location = 1) in vec2 a_texCoord;\n"
            "uniform mat4 u_MVPMatrix;\n"
            "out vec2 v_texCoord;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = u_MVPMatrix * a_position;\n"
            "    v_texCoord = a_texCoord;\n"
            "}";

	char fShaderStr[] =
	        "#version 300 es\n"
            "precision mediump float;\n"
            "in vec2 v_texCoord;\n"
            "layout(location = 0) out vec4 outColor;\n"
            "uniform sampler2D u_texture;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    outColor = texture(u_texture, v_texCoord);\n"
            "}";


	m_ProgramObj = GLUtils::CreateProgram(vShaderStr, fShaderStr);


    GO_CHECK_GL_ERROR();
	if (m_ProgramObj)
	{
		m_MVPMatLoc = glGetUniformLocation(m_ProgramObj, "u_MVPMatrix");
	}
	else
	{
		LOGCATE("ConveyorBeltExample_1::OnSurfaceCreated create program fail");
	}
	/*------------------------------------------------  混合  ---------------------------------------------------*/

    char vMixShaderStr[] =
            "#version 300 es\n"
            "layout(location = 0) in vec4 a_position;\n"
            "layout(location = 1) in vec2 a_texCoord;\n"
            "uniform mat4 u_MVPMatrix;\n"
            "out vec2 v_texCoord;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = u_MVPMatrix * a_position;\n"
            "    v_texCoord = a_texCoord;\n"
            "}";

    char fMixShaderStr[] =
            "#version 300 es\n"
            "precision mediump float;\n"
            "in vec2 v_texCoord;\n"
            "layout(location = 0) out vec4 outColor;\n"
            "uniform sampler2D u_texture;\n"
            "uniform sampler2D u_pre_texture;\n"
            "uniform float u_offset;\n"
            "void main()\n"
            "{\n"
            "   if(v_texCoord.x > 0.5) {\n"
            "       vec2 tmp = vec2(v_texCoord.x - u_offset, v_texCoord.y);\n"
            "       outColor = texture(u_pre_texture, tmp);\n"
//            "       outColor = texture(u_texture, v_texCoord);\n"
            "   } else {\n"
            "       outColor = texture(u_texture, v_texCoord);\n"
            "   }\n"
            "}";

	m_MixProgramObj = GLUtils::CreateProgram(vMixShaderStr, fMixShaderStr);

    GO_CHECK_GL_ERROR();
    if (m_MixProgramObj)
    {
        m_MixMVPMatLoc = glGetUniformLocation(m_MixProgramObj, "u_MVPMatrix");
    }
    else
    {
        LOGCATE("ConveyorBeltExample_1::OnSurfaceCreated create program fail");
    }

    GLfloat verticesCoords[] = {
            -1.0f,  1.0f, 0.0f,  // Position 0
            -1.0f, -1.0f, 0.0f,  // Position 1
            1.0f,  -1.0f, 0.0f,  // Position 2
            1.0f,   1.0f, 0.0f,  // Position 3
    };

    // 顶点在上，渲染视频、图片用
    GLfloat textureCoords0[] = {
            0.0f,  0.0f,        // TexCoord 0
            0.0f,  1.0f,        // TexCoord 1
            1.0f,  1.0f,        // TexCoord 2
            1.0f,  0.0f         // TexCoord 3
    };

    // 顶点在下，渲染纹理用
    GLfloat textureCoords1[] = {
            0.0f,  1.0f,        // TexCoord 1
            0.0f,  0.0f,        // TexCoord 0
            1.0f,  0.0f,         // TexCoord 3
            1.0f,  1.0f,        // TexCoord 2
    };

    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    // Generate VBO Ids and load the VBOs with data
    glGenBuffers(4, m_VboIds);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords0), textureCoords0, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords1), textureCoords1, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Generate VAO Id
    glGenVertexArrays(1, &m_VaoId0);
    glBindVertexArray(m_VaoId0);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[1]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);

    glBindVertexArray(GL_NONE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate VAO Id
    glGenVertexArrays(1, &m_VaoId1);
    glBindVertexArray(m_VaoId1);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ARRAY_BUFFER, m_VboIds[2]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (const void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VboIds[3]);

    glBindVertexArray(GL_NONE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void ConveyorBeltExample_1::LoadImage(NativeImage *pImage)
{
    LOGCATE("ConveyorBeltExample_1::LoadImage pImage = %p,[w=%d,h=%d,f=%d]", pImage->ppPlane[0], pImage->width, pImage->height, pImage->format);
    unique_lock<mutex> lock(m_Lock);
    if(pImage && pImage->format == IMAGE_FORMAT_I420) {
        cv::Mat mati420 = cv::Mat(pImage->height * 3 / 2, pImage->width, CV_8UC1, pImage->ppPlane[0]);

        if(m_RenderImage.ppPlane[0] == nullptr) {
            m_RenderImage.width = pImage->width;
            m_RenderImage.height = pImage->height;
            m_RenderImage.format = IMAGE_FORMAT_RGBA;
            NativeImageUtil::AllocNativeImage(&m_RenderImage);

            m_SrcImage = m_RenderImage;
            NativeImageUtil::AllocNativeImage(&m_SrcImage);

            m_bannerNum = m_RenderImage.height / 20;
        } else if(m_RenderImage.width != pImage->width || m_RenderImage.height != pImage->height) {
            NativeImageUtil::FreeNativeImage(&m_RenderImage);
            NativeImageUtil::FreeNativeImage(&m_SrcImage);

            m_RenderImage.width = pImage->width;
            m_RenderImage.height = pImage->height;
            m_RenderImage.format = IMAGE_FORMAT_RGBA;
            NativeImageUtil::AllocNativeImage(&m_RenderImage);

            m_SrcImage = m_RenderImage;
            NativeImageUtil::AllocNativeImage(&m_SrcImage);

            m_bannerNum = m_RenderImage.height / 20;
        }

        cv::Mat matRgba = cv::Mat(m_SrcImage.height, m_SrcImage.width, CV_8UC4, m_SrcImage.ppPlane[0]);

        cv::cvtColor(mati420, matRgba, CV_YUV2RGBA_I420);
        LOGCATE("ConveyorBeltExample_1::cvtColor");
	}
}

void ConveyorBeltExample_1::Draw(int width, int height)
{
    glViewport(0, 0, width, height);
    glClearColor(1.0f,1.0f,1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (m_MixFrameBufferId == GL_NONE) {
        glGenFramebuffers(1, &m_MixFrameBufferId);

        glGenTextures(1, &m_MixTextureId0);
        glBindTexture(GL_TEXTURE_2D, m_MixTextureId0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void *) nullptr);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);

        glGenTextures(1, &m_MixTextureId1);
        glBindTexture(GL_TEXTURE_2D, m_MixTextureId1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void *) nullptr);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);

        glGenTextures(1, &m_MixTextureId2);
        glBindTexture(GL_TEXTURE_2D, m_MixTextureId2);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void *) nullptr);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }

	if(m_ProgramObj == GL_NONE || m_TextureId == GL_NONE
	|| m_MixFrameBufferId == GL_NONE || m_MixProgramObj == GL_NONE
	|| m_MixTextureId0 == GL_NONE || m_MixTextureId1 == GL_NONE) return;
    LOGCATE("ConveyorBeltExample_1::Draw[w, h]=[%d, %d]",width, height);

    // 由于摄像头数据需要旋转的，因此先把摄像头数据摆正
    glBindFramebuffer(GL_FRAMEBUFFER, m_MixFrameBufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_MixTextureId0, 0);
    UpdateMVPMatrix(m_MVPMatrix, m_TransformMatrix);
    // draw fbo tex
    // Use the program object
    glUseProgram (m_ProgramObj);
    GO_CHECK_GL_ERROR();
    glBindVertexArray(m_VaoId0);
    GO_CHECK_GL_ERROR();
    glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPMatrix[0][0]);
    GO_CHECK_GL_ERROR();

    // 绑定数据到纹理上
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_RenderImage.width, m_RenderImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_SrcImage.ppPlane[0]);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    GO_CHECK_GL_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    GLUtils::setInt(m_ProgramObj, "u_texture", 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
    glBindVertexArray(GL_NONE);
    GO_CHECK_GL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unique_lock<mutex> lock(m_Lock);
    m_frameIndex ++;

    GLint preTex = m_frameIndex % 2 == 0 ? m_MixTextureId1 : m_MixTextureId2;
    GLint fboTex = m_frameIndex % 2 == 0 ? m_MixTextureId2 : m_MixTextureId1;
    glBindFramebuffer(GL_FRAMEBUFFER, m_MixFrameBufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);

	// Use the program object
	glUseProgram (m_MixProgramObj);
    GO_CHECK_GL_ERROR();
	glBindVertexArray(m_VaoId1);
    GO_CHECK_GL_ERROR();
	glUniformMatrix4fv(m_MixMVPMatLoc, 1, GL_FALSE, &m_MVPIdMatrix[0][0]);
    GO_CHECK_GL_ERROR();

	GLUtils::setFloat(m_MixProgramObj, "u_offset", 0.01);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_MixTextureId0);
    GLUtils::setInt(m_MixProgramObj, "u_texture", 0);
    GO_CHECK_GL_ERROR();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, preTex);
    GLUtils::setInt(m_MixProgramObj, "u_pre_texture", 1);
    GO_CHECK_GL_ERROR();

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
    GO_CHECK_GL_ERROR();

    glBindVertexArray(GL_NONE);
    GO_CHECK_GL_ERROR();

    // unbind fbo
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GO_CHECK_GL_ERROR();

    // draw fbo tex
    // Use the program object
    glUseProgram (m_ProgramObj);
    GO_CHECK_GL_ERROR();
    glBindVertexArray(m_VaoId1);
    GO_CHECK_GL_ERROR();
    glUniformMatrix4fv(m_MVPMatLoc, 1, GL_FALSE, &m_MVPIdMatrix[0][0]);
    GO_CHECK_GL_ERROR();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fboTex);
    GLUtils::setInt(m_ProgramObj, "u_texture", 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (const void *)0);
    glBindVertexArray(GL_NONE);
    GO_CHECK_GL_ERROR();
}

void ConveyorBeltExample_1::Destroy()
{
	if (m_ProgramObj)
	{
		glDeleteProgram(m_ProgramObj);
		glDeleteBuffers(3, m_VboIds);
		glDeleteVertexArrays(1, &m_VaoId0);
		glDeleteVertexArrays(1, &m_VaoId1);
		glDeleteTextures(1, &m_TextureId);
	}
}

void ConveyorBeltExample_1::SetTransformMatrix(TransformMatrix &transformMatrix)
{
    LOGCATE("ConveyorBeltExample_1::SetTransformMatrix translateX = %f, translateY = %f, scaleX = %f, scaleY = %f, degree = %d, mirror = %d",
            transformMatrix.translateX, transformMatrix.translateY, transformMatrix.scaleX, transformMatrix.scaleY, transformMatrix.degree, transformMatrix.mirror);
    m_TransformMatrix.degree = transformMatrix.degree;
    m_TransformMatrix.mirror = transformMatrix.mirror;
}

void ConveyorBeltExample_1::LoadMultiImageWithIndex(int index, NativeImage *pImage) {
	LOGCATE("ConveyorBeltExample_1::LoadMultiImageWithIndex pImage = %p,[w=%d,h=%d,f=%d]", pImage->ppPlane[0], pImage->width, pImage->height, pImage->format);
}

void ConveyorBeltExample_1::UpdateMVPMatrix(mat4 &mat4Matrix, TransformMatrix &transformMatrix) {
    LOGCATE("ConveyorBeltExample_1::UpdateMVPMatrix");
    float fFactorX = 1.0f;
    float fFactorY = 1.0f;

    if (transformMatrix.mirror == 1) {
        fFactorX = -1.0f;
    } else if (transformMatrix.mirror == 2) {
        fFactorY = -1.0f;
    }

    float fRotate = MATH_PI * transformMatrix.degree * 1.0f / 180;
    if (transformMatrix.mirror == 0) {
        if (transformMatrix.degree == 270) {
            fRotate = MATH_PI * 0.5;
        } else if (transformMatrix.degree == 180) {
            fRotate = MATH_PI;
        } else if (transformMatrix.degree == 90) {
            fRotate = MATH_PI * 1.5;
        }
    } else if (transformMatrix.mirror == 1) {
        if (transformMatrix.degree == 90) {
            fRotate = MATH_PI * 0.5;
        } else if (transformMatrix.degree == 180) {
            fRotate = MATH_PI;
        } else if (transformMatrix.degree == 270) {
            fRotate = MATH_PI * 1.5;
        }
    }

    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
    //glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
    //glm::mat4 Projection = glm::perspective(45.0f,4.0f/3.0f,0.1f,100.f);
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 1), // Camera is at (0,0,1), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0) // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(fFactorX * transformMatrix.scaleX,
                                        fFactorY * transformMatrix.scaleY, 1.0f));
    Model = glm::rotate(Model, fRotate, glm::vec3(0.0f, 0.0f, 1.0f));
    Model = glm::translate(Model,
                           glm::vec3(transformMatrix.translateX, transformMatrix.translateY, 0.0f));

    LOGCATE("ConveyorBeltExample_1::UpdateMVPMatrix: rotate %d,%.2f,%0.5f,%0.5f,%0.5f,%0.5f,", transformMatrix.degree, fRotate,
            transformMatrix.translateX, transformMatrix.translateY,
            fFactorX * transformMatrix.scaleX, fFactorY * transformMatrix.scaleY);

    mat4Matrix =
            Projection * View * Model;// Remember, matrix multiplication is the other way around
}
