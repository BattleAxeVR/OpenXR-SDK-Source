// Copyright (c) 2017-2025 The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "pch.h"
#include "common.h"
#include "geometry.h"
#include "graphicsplugin.h"
#include "options.h"

#ifdef XR_USE_GRAPHICS_API_OPENGL_ES

#include "common/gfxwrapper_opengl.h"
#include <common/xr_linear.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "stb_image_write.c"

#define EGL_EGLEXT_PROTOTYPES
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GLES/gl.h"
#define GL_GLEXT_PROTOTYPES
#include "GLES/glext.h"

#include <jni.h>
#include <cassert>
#include <cstdint>
#include <dlfcn.h>
#include <android/log.h>
#include <sys/time.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <map>

#define HARDCODE_VIEW_MATRIX 0
#define HARDCODE_PROJECTION_MATRIX 0
#define HARDCODE_FOV 0

extern int current_eye;
extern float IPD;

#if USE_THUMBSTICKS
extern BVR::GLMPose player_pose;
extern BVR::GLMPose local_hmd_pose;
#endif

namespace {

#if ENABLE_HDR_SWAPCHAIN
bool has_GL_extension(const char *extension) 
{
    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    
    for (int i = 0; i < numExtensions; i++) 
    {
        const GLubyte *string = glGetStringi(GL_EXTENSIONS, i);
        
        if (strcmp((const char *)string, extension) == 0) 
        {
            return true;
        }
    }
    return false;
}
#endif

bool check_gl_errors()
{
    bool no_error_occurred = true;
    int error =  eglGetError();

    while (error != EGL_SUCCESS)
    {
        //const char* error_string = EglErrorString(error);
        Log::Write(Log::Level::Info, "check_gl_errors - " + std::to_string(error));

        error = eglGetError();
        no_error_occurred = false;
    }

    return no_error_occurred;
}

    static const char* VertexShaderGlsl = R"_(#version 320 es

    in vec3 VertexPos;
    in vec3 VertexColor;

    out vec3 PSVertexColor;

    uniform mat4 ModelViewProjection;

    void main() {
       gl_Position = ModelViewProjection * vec4(VertexPos, 1.0);
       PSVertexColor = VertexColor;
    }
    )_";

#if ENABLE_TINT
    static const char* FragmentShaderGlsl = R"_(#version 320 es

    in lowp vec3 PSVertexColor;
    out lowp vec4 FragColor;

    uniform lowp vec4 Tint;

    void main() {
       FragColor = vec4(PSVertexColor, 1) * Tint;
    }
    )_";
#else
// The version statement has come on first line.
static const char* FragmentShaderGlsl = R"_(#version 320 es

    in lowp vec3 PSVertexColor;
    out lowp vec4 FragColor;

    void main() {
       FragColor = vec4(PSVertexColor, 1);
    }
    )_";
#endif // ENABLE_TINT

struct OpenGLESGraphicsPlugin : public IGraphicsPlugin {
    OpenGLESGraphicsPlugin(const std::shared_ptr<Options>& options, const std::shared_ptr<IPlatformPlugin> /*unused*/&)
        : m_clearColor(options->GetBackgroundClearColor()) {}

    OpenGLESGraphicsPlugin(const OpenGLESGraphicsPlugin&) = delete;
    OpenGLESGraphicsPlugin& operator=(const OpenGLESGraphicsPlugin&) = delete;
    OpenGLESGraphicsPlugin(OpenGLESGraphicsPlugin&&) = delete;
    OpenGLESGraphicsPlugin& operator=(OpenGLESGraphicsPlugin&&) = delete;

#if ENABLE_TINT
    GLint tint_location_ = 0;
#endif

    ~OpenGLESGraphicsPlugin() override {
        if (m_swapchainFramebuffer != 0) {
            glDeleteFramebuffers(1, &m_swapchainFramebuffer);
        }
        if (m_program != 0) {
            glDeleteProgram(m_program);
        }
        if (m_vao != 0) {
            glDeleteVertexArrays(1, &m_vao);
        }
        if (m_cubeVertexBuffer != 0) {
            glDeleteBuffers(1, &m_cubeVertexBuffer);
        }
        if (m_cubeIndexBuffer != 0) {
            glDeleteBuffers(1, &m_cubeIndexBuffer);
        }

        for (auto& colorToDepth : m_colorToDepthMap) {
            if (colorToDepth.second != 0) {
                glDeleteTextures(1, &colorToDepth.second);
            }
        }

        ksGpuWindow_Destroy(&window);
    }

    std::vector<std::string> GetInstanceExtensions() const override { return {XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME}; }

    ksGpuWindow window{};

    void DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message) {
        (void)source;
        (void)type;
        (void)id;
        (void)severity;
        Log::Write(Log::Level::Info, "GLES Debug: " + std::string(message, 0, length));
    }

#if ENABLE_HDR_SWAPCHAIN
        bool supports_hdr_ = false;
#endif

    void InitializeDevice(XrInstance instance, XrSystemId systemId) override {
        // Extension function must be loaded by name
        PFN_xrGetOpenGLESGraphicsRequirementsKHR pfnGetOpenGLESGraphicsRequirementsKHR = nullptr;
        CHECK_XRCMD(xrGetInstanceProcAddr(instance, "xrGetOpenGLESGraphicsRequirementsKHR",
                                          reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetOpenGLESGraphicsRequirementsKHR)));

        XrGraphicsRequirementsOpenGLESKHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR};
        CHECK_XRCMD(pfnGetOpenGLESGraphicsRequirementsKHR(instance, systemId, &graphicsRequirements));

        // Initialize the gl extensions. Note we have to open a window.
        ksDriverInstance driverInstance{};
        ksGpuQueueInfo queueInfo{};
        ksGpuSurfaceColorFormat colorFormat{KS_GPU_SURFACE_COLOR_FORMAT_B8G8R8A8};
        ksGpuSurfaceDepthFormat depthFormat{KS_GPU_SURFACE_DEPTH_FORMAT_D24};
        ksGpuSampleCount sampleCount{KS_GPU_SAMPLE_COUNT_1};
        if (!ksGpuWindow_Create(&window, &driverInstance, &queueInfo, 0, colorFormat, depthFormat, sampleCount, 640, 480, false)) {
            THROW("Unable to create GL context");
        }

        GLint major = 0;
        GLint minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);

        const XrVersion desiredApiVersion = XR_MAKE_VERSION(major, minor, 0);
        if (graphicsRequirements.minApiVersionSupported > desiredApiVersion) {
            THROW("Runtime does not support desired Graphics API and/or version");
        }

        m_contextApiMajorVersion = major;

#if defined(XR_USE_PLATFORM_ANDROID)
        m_graphicsBinding.display = window.display;
        m_graphicsBinding.config = (EGLConfig)0;
        m_graphicsBinding.context = window.context.context;

#if ENABLE_HDR_SWAPCHAIN
        // https://developer.qualcomm.com/sites/default/files/docs/adreno-gpu/snapdragon-game-toolkit/gdg/tutorials/android/hdr10.html
        
        // Check extensions for HDR
        const bool supports_dci_p3_gamut = has_GL_extension("EGL_EXT_gl_colorspace_display_p3");
        const bool supports_bt2020_gamut = has_GL_extension("EGL_EXT_gl_colorspace_bt2020_pq");
        const bool supports_smpte_2086 = has_GL_extension("EGL_EXT_surface_SMPTE2086_metadata");

        supports_hdr_ = supports_dci_p3_gamut && supports_bt2020_gamut && supports_smpte_2086;
#endif
        
#endif

        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(
            [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
               const void* userParam) {
                ((OpenGLESGraphicsPlugin*)userParam)->DebugMessageCallback(source, type, id, severity, length, message);
            },
            this);

        InitializeResources();
    }

    void InitializeResources() {
        glGenFramebuffers(1, &m_swapchainFramebuffer);

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &VertexShaderGlsl, nullptr);
        glCompileShader(vertexShader);
        CheckShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &FragmentShaderGlsl, nullptr);
        glCompileShader(fragmentShader);
        CheckShader(fragmentShader);

        m_program = glCreateProgram();
        glAttachShader(m_program, vertexShader);
        glAttachShader(m_program, fragmentShader);
        glLinkProgram(m_program);
        CheckProgram(m_program);
        
#if ENABLE_TINT
        tint_location_ = glGetUniformLocation(m_program, "Tint");
#endif

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        m_modelViewProjectionUniformLocation = glGetUniformLocation(m_program, "ModelViewProjection");

        m_vertexAttribCoords = glGetAttribLocation(m_program, "VertexPos");
        m_vertexAttribColor = glGetAttribLocation(m_program, "VertexColor");

        glGenBuffers(1, &m_cubeVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_cubeVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Geometry::c_cubeVertices), Geometry::c_cubeVertices, GL_STATIC_DRAW);

        glGenBuffers(1, &m_cubeIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Geometry::c_cubeIndices), Geometry::c_cubeIndices, GL_STATIC_DRAW);

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glEnableVertexAttribArray(m_vertexAttribCoords);
        glEnableVertexAttribArray(m_vertexAttribColor);
        glBindBuffer(GL_ARRAY_BUFFER, m_cubeVertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeIndexBuffer);
        glVertexAttribPointer(m_vertexAttribCoords, 3, GL_FLOAT, GL_FALSE, sizeof(Geometry::Vertex), nullptr);
        glVertexAttribPointer(m_vertexAttribColor, 3, GL_FLOAT, GL_FALSE, sizeof(Geometry::Vertex),
                              reinterpret_cast<const void*>(sizeof(XrVector3f)));
    }

    void CheckShader(GLuint shader) {
        GLint r = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &r);
        if (r == GL_FALSE) {
            GLchar msg[4096] = {};
            GLsizei length;
            glGetShaderInfoLog(shader, sizeof(msg), &length, msg);
            THROW(Fmt("Compile shader failed: %s", msg));
        }
    }

    void CheckProgram(GLuint prog) {
        GLint r = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &r);
        if (r == GL_FALSE) {
            GLchar msg[4096] = {};
            GLsizei length;
            glGetProgramInfoLog(prog, sizeof(msg), &length, msg);
            THROW(Fmt("Link program failed: %s", msg));
        }
    }

    int64_t SelectColorSwapchainFormat(const std::vector<int64_t>& runtimeFormats) const override {
        // List of supported color swapchain formats.
        
#if ENABLE_HDR_SWAPCHAIN
        std::vector<int64_t> supportedColorSwapchainFormats{GL_RGBA16F};
#else
        std::vector<int64_t> supportedColorSwapchainFormats{GL_RGBA8, GL_RGBA8_SNORM};
#endif

        // In OpenGLES 3.0+, the R, G, and B values after blending are converted into the non-linear
        // sRGB automatically.
        if (m_contextApiMajorVersion >= 3) {
            supportedColorSwapchainFormats.push_back(GL_SRGB8_ALPHA8);
        }

        auto swapchainFormatIt = std::find_first_of(runtimeFormats.begin(), runtimeFormats.end(),
                                                    supportedColorSwapchainFormats.begin(), supportedColorSwapchainFormats.end());
        if (swapchainFormatIt == runtimeFormats.end()) {
            THROW("No runtime swapchain format supported for color swapchain");
        }

        return *swapchainFormatIt;
    }

    const XrBaseInStructure* GetGraphicsBinding() const override {
        return reinterpret_cast<const XrBaseInStructure*>(&m_graphicsBinding);
    }

    std::vector<XrSwapchainImageBaseHeader*> AllocateSwapchainImageStructs(
        uint32_t capacity, const XrSwapchainCreateInfo& /*swapchainCreateInfo*/) override {
        // Allocate and initialize the buffer of image structs (must be sequential in memory for xrEnumerateSwapchainImages).
        // Return back an array of pointers to each swapchain image struct so the consumer doesn't need to know the type/size.
        std::vector<XrSwapchainImageOpenGLESKHR> swapchainImageBuffer(capacity, {XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR});
        std::vector<XrSwapchainImageBaseHeader*> swapchainImageBase;
        for (XrSwapchainImageOpenGLESKHR& image : swapchainImageBuffer) {
            swapchainImageBase.push_back(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
        }

        // Keep the buffer alive by moving it into the list of buffers.
        m_swapchainImageBuffers.push_back(std::move(swapchainImageBuffer));

        return swapchainImageBase;
    }

#if ENABLE_QUAD_LAYER
	std::vector<XrSwapchainImageBaseHeader*> AllocateSwapchainQuadLayerImageStructs(
		uint32_t capacity, const XrSwapchainCreateInfo& /*swapchainCreateInfo*/) override {

		// Allocate and initialize the buffer of image structs (must be sequential in memory for xrEnumerateSwapchainImages).
		// Return back an array of pointers to each swapchain image struct so the consumer doesn't need to know the type/size.
		std::vector<XrSwapchainImageOpenGLESKHR> swapchainImageBuffer(capacity, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR });
		std::vector<XrSwapchainImageBaseHeader*> swapchainImageBase;
		for(XrSwapchainImageOpenGLESKHR& image : swapchainImageBuffer) {
			swapchainImageBase.push_back(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
		}

		// Keep the buffer alive by moving it into the list of buffers.
        m_swapchainQuadLayerImageBuffers.push_back(std::move(swapchainImageBuffer));

		return swapchainImageBase;
	}

	void RenderQuadLayer(const XrCompositionLayerQuad& layer, const XrSwapchainImageBaseHeader* swapchainImage,
		int64_t swapchainFormat, const std::vector<Cube>& cubes) override
	{
	}
#endif

    uint32_t GetDepthTexture(uint32_t colorTexture) {
        // If a depth-stencil view has already been created for this back-buffer, use it.
        auto depthBufferIt = m_colorToDepthMap.find(colorTexture);
        if (depthBufferIt != m_colorToDepthMap.end()) {
            return depthBufferIt->second;
        }

        // This back-buffer has no corresponding depth-stencil texture, so create one with matching dimensions.

        GLint width;
        GLint height;
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

        uint32_t depthTexture;
        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);

        m_colorToDepthMap.insert(std::make_pair(colorTexture, depthTexture));

        return depthTexture;
    }

    void RenderView(const XrCompositionLayerProjectionView& layerView, const XrSwapchainImageBaseHeader* swapchainImage,
                    int64_t swapchainFormat, const std::vector<Cube>& cubes) override {
        CHECK(layerView.subImage.imageArrayIndex == 0);  // Texture arrays not supported.
        UNUSED_PARM(swapchainFormat);                    // Not used in this function for now.

        glBindFramebuffer(GL_FRAMEBUFFER, m_swapchainFramebuffer);

        const uint32_t colorTexture = reinterpret_cast<const XrSwapchainImageOpenGLESKHR*>(swapchainImage)->image;

        glViewport(static_cast<GLint>(layerView.subImage.imageRect.offset.x),
                   static_cast<GLint>(layerView.subImage.imageRect.offset.y),
                   static_cast<GLsizei>(layerView.subImage.imageRect.extent.width),
                   static_cast<GLsizei>(layerView.subImage.imageRect.extent.height));

        const uint32_t depthTexture = GetDepthTexture(colorTexture);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

        // Clear swapchain and depth buffer.
        glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);
        glClearDepthf(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        if (!cubes.empty())
        {
            glFrontFace(GL_CW);
            glCullFace(GL_BACK);
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);

            // Set shaders and uniform variables.
            glUseProgram(m_program);


        const auto& pose = layerView.pose;
        XrMatrix4x4f proj;
        XrMatrix4x4f_CreateProjectionFov(&proj, GRAPHICS_OPENGL_ES, layerView.fov, 0.05f, 100.0f);

#if HARDCODE_PROJECTION_MATRIX
#endif

		XrMatrix4x4f toView;
		XrVector3f scale{1.f, 1.f, 1.f};
		XrMatrix4x4f_CreateTranslationRotationScale(&toView, &pose.position, &pose.orientation, &scale);

		XrMatrix4x4f view;
		XrMatrix4x4f_InvertRigidBody(&view, &toView);

#if HARDCODE_VIEW_MATRIX
		{
            static int eye = 1;
            const float ipd = 0.0680999979f;
            const float half_ipd = ipd / 2.0f;

            XrPosef hardcoded_pose;
            hardcoded_pose.position.x = half_ipd * ((eye == 0) ? -1.0f : 1.0f);
            hardcoded_pose.position.y = 1.0f;
            hardcoded_pose.position.z = 0.0f;

            hardcoded_pose.orientation.x = 0.0f;
            hardcoded_pose.orientation.y = 0.0f;
            hardcoded_pose.orientation.z = 0.0f;
            hardcoded_pose.orientation.w = 1.0f;

            XrMatrix4x4f_CreateTranslationRotationScale(&view, &hardcoded_pose.position, &hardcoded_pose.orientation, &scale);
            eye = 1 - eye;
        }
#endif

#if USE_THUMBSTICKS
		const XrPosef xr_local_eye_pose = layerView.pose;
		const BVR::GLMPose local_eye_pose = BVR::convert_to_glm_pose(xr_local_eye_pose);

		const glm::vec3 local_hmd_to_eye = local_eye_pose.translation_ - local_hmd_pose.translation_;
		const glm::vec3 world_hmd_to_eye = player_pose.rotation_ * local_hmd_to_eye;

		const glm::vec3 world_hmd_offset = player_pose.rotation_ * local_hmd_pose.translation_;
		const glm::vec3 world_hmd_position = player_pose.translation_ + world_hmd_offset;

		const glm::vec3 world_eye_position = world_hmd_position + world_hmd_to_eye;
		const glm::fquat world_orientation = glm::normalize(player_pose.rotation_ * local_hmd_pose.rotation_);

		BVR::GLMPose world_eye_pose;
		world_eye_pose.translation_ = world_eye_position;
		world_eye_pose.rotation_ = world_orientation;

		const glm::mat4 inverse_view_glm = world_eye_pose.to_matrix();
		const glm::mat4 view_glm = glm::inverse(inverse_view_glm);

		view = BVR::convert_to_xr_pose(view_glm);
#endif

		XrMatrix4x4f vp;
		XrMatrix4x4f_Multiply(&vp, &proj, &view);

		// Set cube primitive data.
		glBindVertexArray(m_vao);

		// Render each cube
		for (const Cube& cube : cubes) {

#if ENABLE_TINT
            glUniform4fv(tint_location_, 1, &cube.Colour.x);

#if ENABLE_BLENDING
            if (cube.enable_blend)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                glDepthMask(GL_FALSE);
            }
            else
            {
                glDepthMask(GL_TRUE);
                glDisable(GL_BLEND);
            }
#endif
            
#endif
            
            // Compute the model-view-projection transform and set it..
			XrMatrix4x4f model;
			XrMatrix4x4f_CreateTranslationRotationScale(&model, &cube.Pose.position, &cube.Pose.orientation, &cube.Scale);
			XrMatrix4x4f mvp;
			XrMatrix4x4f_Multiply(&mvp, &vp, &model);
			glUniformMatrix4fv(m_modelViewProjectionUniformLocation, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&mvp));

			// Draw the cube.
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(ArraySize(Geometry::c_cubeIndices)), GL_UNSIGNED_SHORT, nullptr);
		}

		glBindVertexArray(0);
		glUseProgram(0);
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void ClearView(const XrCompositionLayerProjectionView& layerView, const XrSwapchainImageBaseHeader* swapchainImage) override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_swapchainFramebuffer);

        const uint32_t colorTexture = reinterpret_cast<const XrSwapchainImageOpenGLESKHR*>(swapchainImage)->image;

        glViewport(static_cast<GLint>(layerView.subImage.imageRect.offset.x),
                   static_cast<GLint>(layerView.subImage.imageRect.offset.y),
                   static_cast<GLsizei>(layerView.subImage.imageRect.extent.width),
                   static_cast<GLsizei>(layerView.subImage.imageRect.extent.height));

        const uint32_t depthTexture = GetDepthTexture(colorTexture);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

        glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);
        glClearDepthf(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    uint32_t GetSupportedSwapchainSampleCount(const XrViewConfigurationView&) override { return 1; }

    void UpdateOptions(const std::shared_ptr<Options>& options) override { m_clearColor = options->GetBackgroundClearColor(); }

    void SaveScreenShot(const std::string& filename) override 
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_swapchainFramebuffer);

        GLint viewport[4] = {};
        glGetIntegerv(GL_VIEWPORT, viewport);

        int x = viewport[0];
        int y = viewport[1];

        int width = viewport[2];
        int height = viewport[3];

        int num_components = 4;  // 3 = RGB or 4 = RGBA
        char* data = (char*)malloc((size_t)(width * height * num_components));

        if (!data) 
        {
            return;
        }

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

        int write_status = stbi_write_png(filename.c_str(), width, height, num_components, data, 0);
        Log::Write(Log::Level::Info, Fmt("SaveScreenShot %s status = %d", filename.c_str(), write_status));

        free(data);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        check_gl_errors();
    }

   private:
#ifdef XR_USE_PLATFORM_ANDROID
    XrGraphicsBindingOpenGLESAndroidKHR m_graphicsBinding{XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR};
#endif

    std::list<std::vector<XrSwapchainImageOpenGLESKHR>> m_swapchainImageBuffers;
    GLuint m_swapchainFramebuffer{0};
    GLuint m_program{0};
    GLint m_modelViewProjectionUniformLocation{0};
    GLint m_vertexAttribCoords{0};
    GLint m_vertexAttribColor{0};
    GLuint m_vao{0};
    GLuint m_cubeVertexBuffer{0};
    GLuint m_cubeIndexBuffer{0};
    GLint m_contextApiMajorVersion{0};

    // Map color buffer to associated depth buffer. This map is populated on demand.
    std::map<uint32_t, uint32_t> m_colorToDepthMap;
    std::array<float, 4> m_clearColor;

#if ENABLE_QUAD_LAYER
	std::list<std::vector<XrSwapchainImageOpenGLESKHR>> m_swapchainQuadLayerImageBuffers;
	GLuint m_swapchainQuadLayerFramebuffer{ 0 };
#endif
};
}  // namespace

std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_OpenGLES(const std::shared_ptr<Options>& options,
                                                               std::shared_ptr<IPlatformPlugin> platformPlugin) {
    return std::make_shared<OpenGLESGraphicsPlugin>(options, platformPlugin);
}

#endif
