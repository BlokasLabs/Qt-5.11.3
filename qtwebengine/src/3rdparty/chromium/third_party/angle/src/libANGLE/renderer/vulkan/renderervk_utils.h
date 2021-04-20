//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// renderervk_utils:
//    Helper functions for the Vulkan Renderer.
//

#ifndef LIBANGLE_RENDERER_VULKAN_RENDERERVK_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_RENDERERVK_UTILS_H_

#include <limits>

#include <vulkan/vulkan.h>

#include "common/Optional.h"
#include "common/debug.h"
#include "libANGLE/Error.h"
#include "libANGLE/renderer/renderer_utils.h"

#define ANGLE_GL_OBJECTS_X(PROC) \
    PROC(Buffer)                 \
    PROC(Context)                \
    PROC(Framebuffer)            \
    PROC(Program)                \
    PROC(Texture)                \
    PROC(VertexArray)

#define ANGLE_PRE_DECLARE_OBJECT(OBJ) class OBJ;

namespace egl
{
class Display;
}

namespace gl
{
struct Box;
struct Extents;
struct RasterizerState;
struct Rectangle;
struct VertexAttribute;
class VertexBinding;

ANGLE_GL_OBJECTS_X(ANGLE_PRE_DECLARE_OBJECT);
}

#define ANGLE_PRE_DECLARE_VK_OBJECT(OBJ) class OBJ##Vk;

namespace rx
{
class DisplayVk;
class RenderTargetVk;
class RendererVk;
class ResourceVk;
class RenderPassCache;

enum class DrawType
{
    Arrays,
    Elements,
};

ANGLE_GL_OBJECTS_X(ANGLE_PRE_DECLARE_VK_OBJECT);

const char *VulkanResultString(VkResult result);
bool HasStandardValidationLayer(const std::vector<VkLayerProperties> &layerProps);

extern const char *g_VkStdValidationLayerName;
extern const char *g_VkLoaderLayersPathEnv;

enum class TextureDimension
{
    TEX_2D,
    TEX_CUBE,
    TEX_3D,
    TEX_2D_ARRAY,
};

namespace vk
{
class CommandBufferNode;
struct Format;

template <typename T>
struct ImplTypeHelper;

// clang-format off
#define ANGLE_IMPL_TYPE_HELPER_GL(OBJ) \
template<>                             \
struct ImplTypeHelper<gl::OBJ>         \
{                                      \
    using ImplType = OBJ##Vk;          \
};
// clang-format on

ANGLE_GL_OBJECTS_X(ANGLE_IMPL_TYPE_HELPER_GL)

template <>
struct ImplTypeHelper<egl::Display>
{
    using ImplType = DisplayVk;
};

template <typename T>
using GetImplType = typename ImplTypeHelper<T>::ImplType;

template <typename T>
GetImplType<T> *GetImpl(const T *glObject)
{
    return GetImplAs<GetImplType<T>>(glObject);
}

class MemoryProperties final : angle::NonCopyable
{
  public:
    MemoryProperties();

    void init(VkPhysicalDevice physicalDevice);
    uint32_t findCompatibleMemoryIndex(uint32_t bitMask, uint32_t propertyFlags) const;

  private:
    VkPhysicalDeviceMemoryProperties mMemoryProperties;
};

class Error final
{
  public:
    Error(VkResult result);
    Error(VkResult result, const char *file, unsigned int line);
    ~Error();

    Error(const Error &other);
    Error &operator=(const Error &other);

    gl::Error toGL(GLenum glErrorCode) const;
    egl::Error toEGL(EGLint eglErrorCode) const;

    operator gl::Error() const;
    operator egl::Error() const;

    template <typename T>
    operator gl::ErrorOrResult<T>() const
    {
        return operator gl::Error();
    }

    bool isError() const;

    std::string toString() const;

  private:
    VkResult mResult;
    const char *mFile;
    unsigned int mLine;
};

template <typename ResultT>
using ErrorOrResult = angle::ErrorOrResultBase<Error, ResultT, VkResult, VK_SUCCESS>;

// Avoid conflicting with X headers which define "Success".
inline Error NoError()
{
    return Error(VK_SUCCESS);
}

// Unimplemented handle types:
// Instance
// PhysicalDevice
// Device
// Queue
// Event
// QueryPool
// BufferView
// DescriptorSet
// PipelineCache

#define ANGLE_HANDLE_TYPES_X(FUNC) \
    FUNC(Semaphore)                \
    FUNC(CommandBuffer)            \
    FUNC(Fence)                    \
    FUNC(DeviceMemory)             \
    FUNC(Buffer)                   \
    FUNC(Image)                    \
    FUNC(ImageView)                \
    FUNC(ShaderModule)             \
    FUNC(PipelineLayout)           \
    FUNC(RenderPass)               \
    FUNC(Pipeline)                 \
    FUNC(DescriptorSetLayout)      \
    FUNC(Sampler)                  \
    FUNC(DescriptorPool)           \
    FUNC(Framebuffer)              \
    FUNC(CommandPool)

#define ANGLE_COMMA_SEP_FUNC(TYPE) TYPE,

enum class HandleType
{
    Invalid,
    ANGLE_HANDLE_TYPES_X(ANGLE_COMMA_SEP_FUNC)
};

#undef ANGLE_COMMA_SEP_FUNC

#define ANGLE_PRE_DECLARE_CLASS_FUNC(TYPE) class TYPE;
ANGLE_HANDLE_TYPES_X(ANGLE_PRE_DECLARE_CLASS_FUNC)
#undef ANGLE_PRE_DECLARE_CLASS_FUNC

// Returns the HandleType of a Vk Handle.
template <typename T>
struct HandleTypeHelper;

// clang-format off
#define ANGLE_HANDLE_TYPE_HELPER_FUNC(TYPE)                     \
template<> struct HandleTypeHelper<TYPE>                        \
{                                                               \
    constexpr static HandleType kHandleType = HandleType::TYPE; \
};
// clang-format on

ANGLE_HANDLE_TYPES_X(ANGLE_HANDLE_TYPE_HELPER_FUNC)

#undef ANGLE_HANDLE_TYPE_HELPER_FUNC

class GarbageObject final
{
  public:
    template <typename ObjectT>
    GarbageObject(Serial serial, const ObjectT &object)
        : mSerial(serial),
          mHandleType(HandleTypeHelper<ObjectT>::kHandleType),
          mHandle(reinterpret_cast<VkDevice>(object.getHandle()))
    {
    }

    GarbageObject();
    GarbageObject(const GarbageObject &other);
    GarbageObject &operator=(const GarbageObject &other);

    bool destroyIfComplete(VkDevice device, Serial completedSerial);
    void destroy(VkDevice device);

  private:
    // TODO(jmadill): Since many objects will have the same serial, it might be more efficient to
    // store the serial outside of the garbage object itself. We could index ranges of garbage
    // objects in the Renderer, using a circular buffer.
    Serial mSerial;
    HandleType mHandleType;
    VkDevice mHandle;
};

template <typename DerivedT, typename HandleT>
class WrappedObject : angle::NonCopyable
{
  public:
    HandleT getHandle() const { return mHandle; }
    bool valid() const { return (mHandle != VK_NULL_HANDLE); }

    const HandleT *ptr() const { return &mHandle; }

    void dumpResources(Serial serial, std::vector<vk::GarbageObject> *garbageQueue)
    {
        if (valid())
        {
            garbageQueue->emplace_back(serial, *static_cast<DerivedT *>(this));
            mHandle = VK_NULL_HANDLE;
        }
    }

  protected:
    WrappedObject() : mHandle(VK_NULL_HANDLE) {}
    ~WrappedObject() { ASSERT(!valid()); }

    WrappedObject(WrappedObject &&other) : mHandle(other.mHandle)
    {
        other.mHandle = VK_NULL_HANDLE;
    }

    // Only works to initialize empty objects, since we don't have the device handle.
    WrappedObject &operator=(WrappedObject &&other)
    {
        ASSERT(!valid());
        std::swap(mHandle, other.mHandle);
        return *this;
    }

    HandleT mHandle;
};

class CommandPool final : public WrappedObject<CommandPool, VkCommandPool>
{
  public:
    CommandPool();

    void destroy(VkDevice device);

    Error init(VkDevice device, const VkCommandPoolCreateInfo &createInfo);
};

// Helper class that wraps a Vulkan command buffer.
class CommandBuffer : public WrappedObject<CommandBuffer, VkCommandBuffer>
{
  public:
    CommandBuffer();

    VkCommandBuffer releaseHandle();
    void destroy(VkDevice device, const vk::CommandPool &commandPool);
    Error init(VkDevice device, const VkCommandBufferAllocateInfo &createInfo);
    using WrappedObject::operator=;

    Error begin(const VkCommandBufferBeginInfo &info);

    Error end();
    Error reset();

    void singleImageBarrier(VkPipelineStageFlags srcStageMask,
                            VkPipelineStageFlags dstStageMask,
                            VkDependencyFlags dependencyFlags,
                            const VkImageMemoryBarrier &imageMemoryBarrier);

    void singleBufferBarrier(VkPipelineStageFlags srcStageMask,
                             VkPipelineStageFlags dstStageMask,
                             VkDependencyFlags dependencyFlags,
                             const VkBufferMemoryBarrier &bufferBarrier);

    void clearSingleColorImage(const vk::Image &image, const VkClearColorValue &color);

    void copyBuffer(const vk::Buffer &srcBuffer,
                    const vk::Buffer &destBuffer,
                    uint32_t regionCount,
                    const VkBufferCopy *regions);

    void copySingleImage(const vk::Image &srcImage,
                         const vk::Image &destImage,
                         const gl::Box &copyRegion,
                         VkImageAspectFlags aspectMask);

    void copyImage(const vk::Image &srcImage,
                   const vk::Image &dstImage,
                   uint32_t regionCount,
                   const VkImageCopy *regions);

    void beginRenderPass(const VkRenderPassBeginInfo &beginInfo, VkSubpassContents subpassContents);
    void endRenderPass();

    void draw(uint32_t vertexCount,
              uint32_t instanceCount,
              uint32_t firstVertex,
              uint32_t firstInstance);

    void drawIndexed(uint32_t indexCount,
                     uint32_t instanceCount,
                     uint32_t firstIndex,
                     int32_t vertexOffset,
                     uint32_t firstInstance);

    void bindPipeline(VkPipelineBindPoint pipelineBindPoint, const vk::Pipeline &pipeline);
    void bindVertexBuffers(uint32_t firstBinding,
                           uint32_t bindingCount,
                           const VkBuffer *buffers,
                           const VkDeviceSize *offsets);
    void bindIndexBuffer(const vk::Buffer &buffer, VkDeviceSize offset, VkIndexType indexType);
    void bindDescriptorSets(VkPipelineBindPoint bindPoint,
                            const vk::PipelineLayout &layout,
                            uint32_t firstSet,
                            uint32_t descriptorSetCount,
                            const VkDescriptorSet *descriptorSets,
                            uint32_t dynamicOffsetCount,
                            const uint32_t *dynamicOffsets);

    void executeCommands(uint32_t commandBufferCount, const vk::CommandBuffer *commandBuffers);
};

class Image final : public WrappedObject<Image, VkImage>
{
  public:
    Image();

    // Use this method if the lifetime of the image is not controlled by ANGLE. (SwapChain)
    void setHandle(VkImage handle);

    // Called on shutdown when the helper class *doesn't* own the handle to the image resource.
    void reset();

    // Called on shutdown when the helper class *does* own the handle to the image resource.
    void destroy(VkDevice device);

    Error init(VkDevice device, const VkImageCreateInfo &createInfo);

    void changeLayoutTop(VkImageAspectFlags aspectMask,
                         VkImageLayout newLayout,
                         CommandBuffer *commandBuffer);

    void changeLayoutWithStages(VkImageAspectFlags aspectMask,
                                VkImageLayout newLayout,
                                VkPipelineStageFlags srcStageMask,
                                VkPipelineStageFlags dstStageMask,
                                CommandBuffer *commandBuffer);

    void getMemoryRequirements(VkDevice device, VkMemoryRequirements *requirementsOut) const;
    Error bindMemory(VkDevice device, const vk::DeviceMemory &deviceMemory);

    VkImageLayout getCurrentLayout() const { return mCurrentLayout; }
    void updateLayout(VkImageLayout layout) { mCurrentLayout = layout; }

  private:
    VkImageLayout mCurrentLayout;
};

class ImageView final : public WrappedObject<ImageView, VkImageView>
{
  public:
    ImageView();
    void destroy(VkDevice device);

    Error init(VkDevice device, const VkImageViewCreateInfo &createInfo);
};

class Semaphore final : public WrappedObject<Semaphore, VkSemaphore>
{
  public:
    Semaphore();
    void destroy(VkDevice device);

    Error init(VkDevice device);
};

class Framebuffer final : public WrappedObject<Framebuffer, VkFramebuffer>
{
  public:
    Framebuffer();
    void destroy(VkDevice device);

    // Use this method only in necessary cases. (RenderPass)
    void setHandle(VkFramebuffer handle);

    Error init(VkDevice device, const VkFramebufferCreateInfo &createInfo);
};

class DeviceMemory final : public WrappedObject<DeviceMemory, VkDeviceMemory>
{
  public:
    DeviceMemory();
    void destroy(VkDevice device);

    Error allocate(VkDevice device, const VkMemoryAllocateInfo &allocInfo);
    Error map(VkDevice device,
              VkDeviceSize offset,
              VkDeviceSize size,
              VkMemoryMapFlags flags,
              uint8_t **mapPointer);
    void unmap(VkDevice device);
};

class RenderPass final : public WrappedObject<RenderPass, VkRenderPass>
{
  public:
    RenderPass();
    void destroy(VkDevice device);

    Error init(VkDevice device, const VkRenderPassCreateInfo &createInfo);
};

enum class StagingUsage
{
    Read,
    Write,
    Both,
};

class Buffer final : public WrappedObject<Buffer, VkBuffer>
{
  public:
    Buffer();
    void destroy(VkDevice device);

    Error init(VkDevice device, const VkBufferCreateInfo &createInfo);
    Error bindMemory(VkDevice device, const DeviceMemory &deviceMemory);
};

class ShaderModule final : public WrappedObject<ShaderModule, VkShaderModule>
{
  public:
    ShaderModule();
    void destroy(VkDevice device);

    Error init(VkDevice device, const VkShaderModuleCreateInfo &createInfo);
};

class Pipeline final : public WrappedObject<Pipeline, VkPipeline>
{
  public:
    Pipeline();
    void destroy(VkDevice device);

    Error initGraphics(VkDevice device, const VkGraphicsPipelineCreateInfo &createInfo);
};

class PipelineLayout final : public WrappedObject<PipelineLayout, VkPipelineLayout>
{
  public:
    PipelineLayout();
    void destroy(VkDevice device);

    Error init(VkDevice device, const VkPipelineLayoutCreateInfo &createInfo);
};

class DescriptorSetLayout final : public WrappedObject<DescriptorSetLayout, VkDescriptorSetLayout>
{
  public:
    DescriptorSetLayout();
    void destroy(VkDevice device);

    Error init(VkDevice device, const VkDescriptorSetLayoutCreateInfo &createInfo);
};

class DescriptorPool final : public WrappedObject<DescriptorPool, VkDescriptorPool>
{
  public:
    DescriptorPool();
    void destroy(VkDevice device);

    Error init(VkDevice device, const VkDescriptorPoolCreateInfo &createInfo);

    Error allocateDescriptorSets(VkDevice device,
                                 const VkDescriptorSetAllocateInfo &allocInfo,
                                 VkDescriptorSet *descriptorSetsOut);
};

class Sampler final : public WrappedObject<Sampler, VkSampler>
{
  public:
    Sampler();
    void destroy(VkDevice device);
    Error init(VkDevice device, const VkSamplerCreateInfo &createInfo);
};

class Fence final : public WrappedObject<Fence, VkFence>
{
  public:
    Fence();
    void destroy(VkDevice fence);
    using WrappedObject::operator=;

    Error init(VkDevice device, const VkFenceCreateInfo &createInfo);
    VkResult getStatus(VkDevice device) const;
};

// Helper class for managing a CPU/GPU transfer Image.
class StagingImage final : angle::NonCopyable
{
  public:
    StagingImage();
    StagingImage(StagingImage &&other);
    void destroy(VkDevice device);

    vk::Error init(VkDevice device,
                   uint32_t queueFamilyIndex,
                   const MemoryProperties &memoryProperties,
                   TextureDimension dimension,
                   VkFormat format,
                   const gl::Extents &extent,
                   StagingUsage usage);

    Image &getImage() { return mImage; }
    const Image &getImage() const { return mImage; }
    DeviceMemory &getDeviceMemory() { return mDeviceMemory; }
    const DeviceMemory &getDeviceMemory() const { return mDeviceMemory; }
    VkDeviceSize getSize() const { return mSize; }

    void dumpResources(Serial serial, std::vector<vk::GarbageObject> *garbageQueue);

  private:
    Image mImage;
    DeviceMemory mDeviceMemory;
    VkDeviceSize mSize;
};

// Similar to StagingImage, for Buffers.
class StagingBuffer final : angle::NonCopyable
{
  public:
    StagingBuffer();
    void destroy(VkDevice device);

    vk::Error init(ContextVk *contextVk, VkDeviceSize size, StagingUsage usage);

    Buffer &getBuffer() { return mBuffer; }
    const Buffer &getBuffer() const { return mBuffer; }
    DeviceMemory &getDeviceMemory() { return mDeviceMemory; }
    const DeviceMemory &getDeviceMemory() const { return mDeviceMemory; }
    size_t getSize() const { return mSize; }

    void dumpResources(Serial serial, std::vector<vk::GarbageObject> *garbageQueue);

  private:
    Buffer mBuffer;
    DeviceMemory mDeviceMemory;
    size_t mSize;
};

template <typename ObjT>
class ObjectAndSerial final : angle::NonCopyable
{
  public:
    ObjectAndSerial(ObjT &&object, Serial queueSerial)
        : mObject(std::move(object)), mQueueSerial(queueSerial)
    {
    }

    ObjectAndSerial(ObjectAndSerial &&other)
        : mObject(std::move(other.mObject)), mQueueSerial(std::move(other.mQueueSerial))
    {
    }
    ObjectAndSerial &operator=(ObjectAndSerial &&other)
    {
        mObject      = std::move(other.mObject);
        mQueueSerial = std::move(other.mQueueSerial);
        return *this;
    }

    Serial queueSerial() const { return mQueueSerial; }
    void updateSerial(Serial newSerial)
    {
        ASSERT(newSerial >= mQueueSerial);
        mQueueSerial = newSerial;
    }

    const ObjT &get() const { return mObject; }
    ObjT &get() { return mObject; }

  private:
    ObjT mObject;
    Serial mQueueSerial;
};

Optional<uint32_t> FindMemoryType(const VkPhysicalDeviceMemoryProperties &memoryProps,
                                  const VkMemoryRequirements &requirements,
                                  uint32_t propertyFlagMask);

Error AllocateBufferMemory(ContextVk *contextVk,
                           size_t size,
                           Buffer *buffer,
                           DeviceMemory *deviceMemoryOut,
                           size_t *requiredSizeOut);

struct BufferAndMemory final : private angle::NonCopyable
{
    vk::Buffer buffer;
    vk::DeviceMemory memory;
};

using RenderPassAndSerial = ObjectAndSerial<RenderPass>;

// Packed Vk resource descriptions.
// Most Vk types use many more bits than required to represent the underlying data.
// Since ANGLE wants cache things like RenderPasses and Pipeline State Objects using
// hashing (and also needs to check equality) we can optimize these operations by
// using fewer bits. Hence the packed types.
//
// One implementation note: these types could potentially be improved by using even
// fewer bits. For example, boolean values could be represented by a single bit instead
// of a uint8_t. However at the current time there are concerns about the portability
// of bitfield operators, and complexity issues with using bit mask operations. This is
// something likely we will want to investigate as the Vulkan implementation progresses.
//
// Second implementation note: the struct packing is also a bit fragile, and some of the
// packing requirements depend on using alignas and field ordering to get the result of
// packing nicely into the desired space. This is something we could also potentially fix
// with a redesign to use bitfields or bit mask operations.

struct alignas(4) PackedAttachmentDesc
{
    uint8_t flags;
    uint8_t samples;
    uint16_t format;
};

static_assert(sizeof(PackedAttachmentDesc) == 4, "Size check failed");

class RenderPassDesc final
{
  public:
    RenderPassDesc();
    ~RenderPassDesc();
    RenderPassDesc(const RenderPassDesc &other);
    RenderPassDesc &operator=(const RenderPassDesc &other);

    // Depth stencil attachments must be packed after color attachments.
    void packColorAttachment(const Format &format, GLsizei samples);
    void packDepthStencilAttachment(const Format &format, GLsizei samples);

    size_t hash() const;

    uint32_t attachmentCount() const;
    uint32_t colorAttachmentCount() const;
    uint32_t depthStencilAttachmentCount() const;
    const PackedAttachmentDesc &operator[](size_t index) const;

  private:
    void packAttachment(uint32_t index, const vk::Format &format, GLsizei samples);

    uint32_t mColorAttachmentCount;
    uint32_t mDepthStencilAttachmentCount;
    gl::AttachmentArray<PackedAttachmentDesc> mAttachmentDescs;
    uint32_t mPadding[4];
};

bool operator==(const RenderPassDesc &lhs, const RenderPassDesc &rhs);

static_assert(sizeof(RenderPassDesc) == 64, "Size check failed");

struct alignas(8) PackedAttachmentOpsDesc final
{
    uint8_t loadOp;
    uint8_t storeOp;
    uint8_t stencilLoadOp;
    uint8_t stencilStoreOp;

    // 16-bits to force pad the structure to exactly 8 bytes.
    uint16_t initialLayout;
    uint16_t finalLayout;
};

static_assert(sizeof(PackedAttachmentOpsDesc) == 8, "Size check failed");

class AttachmentOpsArray final
{
  public:
    AttachmentOpsArray();
    ~AttachmentOpsArray();
    AttachmentOpsArray(const AttachmentOpsArray &other);
    AttachmentOpsArray &operator=(const AttachmentOpsArray &other);

    const PackedAttachmentOpsDesc &operator[](size_t index) const;
    PackedAttachmentOpsDesc &operator[](size_t index);

    // Initializes an attachment op with whatever values. Used for compatible RenderPass checks.
    void initDummyOp(size_t index, VkImageLayout finalLayout);

    size_t hash() const;

  private:
    gl::AttachmentArray<PackedAttachmentOpsDesc> mOps;
};

bool operator==(const AttachmentOpsArray &lhs, const AttachmentOpsArray &rhs);

static_assert(sizeof(AttachmentOpsArray) == 80, "Size check failed");

Error InitializeRenderPassFromDesc(VkDevice device,
                                   const RenderPassDesc &desc,
                                   const AttachmentOpsArray &ops,
                                   RenderPass *renderPass);

struct alignas(8) PackedShaderStageInfo final
{
    uint32_t stage;
    uint32_t moduleSerial;
    // TODO(jmadill): Do we want specialization constants?
};

static_assert(sizeof(PackedShaderStageInfo) == 8, "Size check failed");

struct alignas(4) PackedVertexInputBindingDesc final
{
    // Although techncially stride can be any value in ES 2.0, in practice supporting stride
    // greater than MAX_USHORT should not be that helpful. Note that stride limits are
    // introduced in ES 3.1.
    uint16_t stride;
    uint16_t inputRate;
};

static_assert(sizeof(PackedVertexInputBindingDesc) == 4, "Size check failed");

struct alignas(8) PackedVertexInputAttributeDesc final
{
    uint16_t location;
    uint16_t format;
    uint32_t offset;
};

static_assert(sizeof(PackedVertexInputAttributeDesc) == 8, "Size check failed");

struct alignas(8) PackedInputAssemblyInfo
{
    uint32_t topology;
    uint32_t primitiveRestartEnable;
};

static_assert(sizeof(PackedInputAssemblyInfo) == 8, "Size check failed");

struct alignas(32) PackedRasterizationStateInfo
{
    // Padded to ensure there's no gaps in this structure or those that use it.
    uint32_t depthClampEnable;
    uint32_t rasterizationDiscardEnable;
    uint16_t polygonMode;
    uint16_t cullMode;
    uint16_t frontFace;
    uint16_t depthBiasEnable;
    float depthBiasConstantFactor;
    // Note: depth bias clamp is only exposed in a 3.1 extension, but left here for completeness.
    float depthBiasClamp;
    float depthBiasSlopeFactor;
    float lineWidth;
};

static_assert(sizeof(PackedRasterizationStateInfo) == 32, "Size check failed");

struct alignas(16) PackedMultisampleStateInfo final
{
    uint8_t rasterizationSamples;
    uint8_t sampleShadingEnable;
    uint8_t alphaToCoverageEnable;
    uint8_t alphaToOneEnable;
    float minSampleShading;
    uint32_t sampleMask[gl::MAX_SAMPLE_MASK_WORDS];
};

static_assert(sizeof(PackedMultisampleStateInfo) == 16, "Size check failed");

struct alignas(16) PackedStencilOpState final
{
    uint8_t failOp;
    uint8_t passOp;
    uint8_t depthFailOp;
    uint8_t compareOp;
    uint32_t compareMask;
    uint32_t writeMask;
    uint32_t reference;
};

static_assert(sizeof(PackedStencilOpState) == 16, "Size check failed");

struct PackedDepthStencilStateInfo final
{
    uint8_t depthTestEnable;
    uint8_t depthWriteEnable;
    uint8_t depthCompareOp;
    uint8_t depthBoundsTestEnable;
    // 32-bits to pad the alignments.
    uint32_t stencilTestEnable;
    float minDepthBounds;
    float maxDepthBounds;
    PackedStencilOpState front;
    PackedStencilOpState back;
};

static_assert(sizeof(PackedDepthStencilStateInfo) == 48, "Size check failed");

struct alignas(8) PackedColorBlendAttachmentState final
{
    uint8_t blendEnable;
    uint8_t srcColorBlendFactor;
    uint8_t dstColorBlendFactor;
    uint8_t colorBlendOp;
    uint8_t srcAlphaBlendFactor;
    uint8_t dstAlphaBlendFactor;
    uint8_t alphaBlendOp;
    uint8_t colorWriteMask;
};

static_assert(sizeof(PackedColorBlendAttachmentState) == 8, "Size check failed");

struct PackedColorBlendStateInfo final
{
    // Padded to round the strut size.
    uint32_t logicOpEnable;
    uint32_t logicOp;
    uint32_t attachmentCount;
    float blendConstants[4];
    PackedColorBlendAttachmentState attachments[gl::IMPLEMENTATION_MAX_DRAW_BUFFERS];
};

static_assert(sizeof(PackedColorBlendStateInfo) == 96, "Size check failed");

using ShaderStageInfo       = std::array<PackedShaderStageInfo, 2>;
using VertexInputBindings   = gl::AttribArray<PackedVertexInputBindingDesc>;
using VertexInputAttributes = gl::AttribArray<PackedVertexInputAttributeDesc>;

class PipelineDesc final
{
  public:
    // Use aligned allocation and free so we can use the alignas keyword.
    void *operator new(std::size_t size);
    void operator delete(void *ptr);

    PipelineDesc();
    ~PipelineDesc();
    PipelineDesc(const PipelineDesc &other);
    PipelineDesc &operator=(const PipelineDesc &other);

    size_t hash() const;
    bool operator==(const PipelineDesc &other) const;

    void initDefaults();
    Error initializePipeline(RendererVk *renderer, ProgramVk *programVk, Pipeline *pipelineOut);

    void updateViewport(const gl::Rectangle &viewport, float nearPlane, float farPlane);

    // Shader stage info
    void updateShaders(ProgramVk *programVk);

    // Vertex input state
    void resetVertexInputState();
    void updateVertexInputInfo(uint32_t attribIndex,
                               const gl::VertexBinding &binding,
                               const gl::VertexAttribute &attrib);

    // Input assembly info
    void updateTopology(GLenum drawMode);

    // Raster states
    void updateCullMode(const gl::RasterizerState &rasterState);
    void updateFrontFace(const gl::RasterizerState &rasterState);
    void updateLineWidth(float lineWidth);

    // RenderPass description.
    void updateRenderPassDesc(const RenderPassDesc &renderPassDesc);

  private:
    // TODO(jmadill): Handle Geometry/Compute shaders when necessary.
    ShaderStageInfo mShaderStageInfo;
    VertexInputBindings mVertexInputBindings;
    VertexInputAttributes mVertexInputAttribs;
    PackedInputAssemblyInfo mInputAssemblyInfo;
    // TODO(jmadill): Consider using dynamic state for viewport/scissor.
    VkViewport mViewport;
    VkRect2D mScissor;
    PackedRasterizationStateInfo mRasterizationStateInfo;
    PackedMultisampleStateInfo mMultisampleStateInfo;
    PackedDepthStencilStateInfo mDepthStencilStateInfo;
    PackedColorBlendStateInfo mColorBlendStateInfo;
    // TODO(jmadill): Dynamic state.
    // TODO(jmadill): Pipeline layout
    RenderPassDesc mRenderPassDesc;
};

// Verify the packed pipeline description has no gaps in the packing.
// This is not guaranteed by the spec, but is validated by a compile-time check.
// No gaps or padding at the end ensures that hashing and memcmp checks will not run
// into uninitialized memory regions.
constexpr size_t PipelineDescSumOfSizes =
    sizeof(ShaderStageInfo) + sizeof(VertexInputBindings) + sizeof(VertexInputAttributes) +
    sizeof(PackedInputAssemblyInfo) + sizeof(VkViewport) + sizeof(VkRect2D) +
    sizeof(PackedRasterizationStateInfo) + sizeof(PackedMultisampleStateInfo) +
    sizeof(PackedDepthStencilStateInfo) + sizeof(PackedColorBlendStateInfo) +
    sizeof(RenderPassDesc);

static_assert(sizeof(PipelineDesc) == PipelineDescSumOfSizes, "Size mismatch");

}  // namespace vk

namespace gl_vk
{
VkPrimitiveTopology GetPrimitiveTopology(GLenum mode);
VkCullModeFlags GetCullMode(const gl::RasterizerState &rasterState);
VkFrontFace GetFrontFace(GLenum frontFace);
}  // namespace gl_vk

// This is a helper class for back-end objects used in Vk command buffers. It records a serial
// at command recording times indicating an order in the queue. We use Fences to detect when
// commands finish, and then release any unreferenced and deleted resources based on the stored
// queue serial in a special 'garbage' queue. Resources also track current read and write
// dependencies. Only one command buffer node can be writing to the Resource at a time, but many
// can be reading from it. Together the dependencies will form a command graph at submission time.
class ResourceVk
{
  public:
    ResourceVk();
    virtual ~ResourceVk();

    void updateQueueSerial(Serial queueSerial);
    Serial getQueueSerial() const;

    // Returns true if any tracked read or write nodes match |currentSerial|.
    bool isCurrentlyRecording(Serial currentSerial) const;

    // Returns the active write node, and asserts |currentSerial| matches the stored serial.
    vk::CommandBufferNode *getCurrentWriteNode(Serial currentSerial);

    // Allocates a new write node and calls setWriteNode internally.
    vk::CommandBufferNode *getNewWriteNode(RendererVk *renderer);

    // Called on an operation that will modify this ResourceVk.
    void setWriteNode(Serial serial, vk::CommandBufferNode *newCommands);

    // Allocates a write node via getNewWriteNode and returns a started command buffer.
    // The started command buffer will render outside of a RenderPass.
    vk::Error recordWriteCommands(RendererVk *renderer, vk::CommandBuffer **commandBufferOut);

    // Sets up the dependency relations. |readNode| has the commands that read from this object.
    void updateDependencies(vk::CommandBufferNode *readNode, Serial serial);

  private:
    Serial mStoredQueueSerial;
    std::vector<vk::CommandBufferNode *> mCurrentReadNodes;
    vk::CommandBufferNode *mCurrentWriteNode;
};

}  // namespace rx

#define ANGLE_VK_TRY(command)                                          \
    {                                                                  \
        auto ANGLE_LOCAL_VAR = command;                                \
        if (ANGLE_LOCAL_VAR != VK_SUCCESS)                             \
        {                                                              \
            return rx::vk::Error(ANGLE_LOCAL_VAR, __FILE__, __LINE__); \
        }                                                              \
    }                                                                  \
    ANGLE_EMPTY_STATEMENT

#define ANGLE_VK_CHECK(test, error) ANGLE_VK_TRY(test ? VK_SUCCESS : error)

std::ostream &operator<<(std::ostream &stream, const rx::vk::Error &error);

// Introduce a std::hash for a RenderPassDesc
namespace std
{
template <>
struct hash<rx::vk::RenderPassDesc>
{
    size_t operator()(const rx::vk::RenderPassDesc &key) const { return key.hash(); }
};

template <>
struct hash<rx::vk::AttachmentOpsArray>
{
    size_t operator()(const rx::vk::AttachmentOpsArray &key) const { return key.hash(); }
};
}  // namespace std

#endif  // LIBANGLE_RENDERER_VULKAN_RENDERERVK_UTILS_H_
