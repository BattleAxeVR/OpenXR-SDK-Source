// Copyright (c) 2017-2025 The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#if 1
typedef glm::vec2 Vector2;
typedef glm::vec3 Vector3;
typedef glm::vec4 Vector4;

typedef Vector3 Position;
typedef Vector3 Normal;
typedef Vector4 Colour;
typedef Vector2 TexCoord;

const Colour white(1.0f, 1.0f, 1.0f, 1.0f);
const Colour black(0.0f, 0.0f, 0.0f, 1.0f);

const Colour transparent_white(1.0f, 1.0f, 1.0f, 0.0f);
const Colour transparent_black(0.0f, 0.0f, 0.0f, 0.0f);

const Colour semi_transparent_white(1.0f, 1.0f, 1.0f, 0.5f);
const Colour semi_transparent_black(0.0f, 0.0f, 0.0f, 0.5f);

const Colour red(1.0f, 0.0f, 0.0f, 1.0f);
const Colour green(0.0f, 1.0f, 0.0f, 1.0f);
const Colour blue(0.0f, 0.0f, 1.0f, 1.0f);
#endif

struct Cube 
{
    XrPosef Pose;
    XrVector3f Scale;
    XrVector4f Colour = {1.0f, 1.0f, 1.0f, 1.0f};
    bool enable_blend_ = false;

    int eye_relevance_ = BOTH_EYE_RELEVANCE;

#if ENABLE_HDR_SWAPCHAIN
    float intensity_ = HDR_BASE_INTENSITY;
#else
    float intensity_ = 1.0f;
#endif
};

// Wraps a graphics API so the main openxr program can be graphics API-independent.
struct IGraphicsPlugin {
    virtual ~IGraphicsPlugin() = default;

    // OpenXR extensions required by this graphics API.
    virtual std::vector<std::string> GetInstanceExtensions() const = 0;

    // Create an instance of this graphics api for the provided instance and systemId.
    virtual void InitializeDevice(XrInstance instance, XrSystemId systemId) = 0;

    // Select the preferred swapchain format from the list of available formats.
    virtual int64_t SelectColorSwapchainFormat(const std::vector<int64_t>& runtimeFormats) const = 0;

    // Get the graphics binding header for session creation.
    virtual const XrBaseInStructure* GetGraphicsBinding() const = 0;

    // Allocate space for the swapchain image structures. These are different for each graphics API. The returned
    // pointers are valid for the lifetime of the graphics plugin.
    virtual std::vector<XrSwapchainImageBaseHeader*> AllocateSwapchainImageStructs(
        uint32_t capacity, const XrSwapchainCreateInfo& swapchainCreateInfo) = 0;

    // Render to a swapchain image for a projection view.
    virtual void RenderView(const XrCompositionLayerProjectionView& layerView, const XrSwapchainImageBaseHeader* swapchainImage,
                            int64_t swapchainFormat, const std::vector<Cube>& cubes) = 0;

    virtual void ClearView(const XrCompositionLayerProjectionView& layerView, const XrSwapchainImageBaseHeader* swapchainImage) = 0;

#if ENABLE_QUAD_LAYER
	virtual std::vector<XrSwapchainImageBaseHeader*> AllocateSwapchainQuadLayerImageStructs(
		uint32_t capacity, const XrSwapchainCreateInfo& /*swapchainCreateInfo*/) = 0;

	virtual void RenderQuadLayer(const XrCompositionLayerQuad& layer, const XrSwapchainImageBaseHeader* swapchainImage,
		int64_t swapchainFormat, const std::vector<Cube>& cubes) = 0;
#endif

    // Get recommended number of sub-data element samples in view (recommendedSwapchainSampleCount)
    // if supported by the graphics plugin. A supported value otherwise.
    virtual uint32_t GetSupportedSwapchainSampleCount(const XrViewConfigurationView& view) {
        return view.recommendedSwapchainSampleCount;
    }

    // Perform required steps after updating Options
    virtual void UpdateOptions(const std::shared_ptr<struct Options>& options) = 0;
    virtual void SaveScreenShot(const std::string& filename) = 0;

};

// Create a graphics plugin for the graphics API specified in the options.
std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin(const std::shared_ptr<struct Options>& options,
                                                      std::shared_ptr<struct IPlatformPlugin> platformPlugin);
