/*
	File: gpu.hpp
	Author: Philip Haynes
	A high-level API for writing gpu-based applications that uses Vulkan as a backend.
*/

#ifndef AZCORE_GPU_HPP
#define AZCORE_GPU_HPP

#include "memory.hpp"
#include "Memory/Result.hpp"

namespace AzCore::io {
	struct Window;
} // namespace AzCore::io

namespace AzCore::GPU {

// Usable as an enum or as bitfields
enum class ShaderStage : u32 {
	VERTEX   = 0x01,
	FRAGMENT = 0x02,
	COMPUTE  = 0x04,
};

enum class ShaderValueType : u16 {
	U32=0,
	I32,
	IVEC2,
	IVEC3,
	IVEC4,
	F32,
	VEC2,
	VEC3,
	VEC4,
};

enum class ImageComponentType : u16 {
	SRGB=0, // A UNORM in the sRGB color space (alpha channel is regular UNORM)
	UNORM,  // An unsigned integer representing values between 0 and 1 inclusive
	SNORM,  // A signed integer representing values between -1 and 1 inclusive (NOTE: the minimum int value, e.g. -128, is out of bounds and will be clamped to -1.0)
	USCALED,// An unsigned integer value that's converted to float by the sampler (i.e. 255 will be converted to 255.0f)
	SSCALED,// A signed integer value that's converted to float by the sampler (i.e. -128 will be converted to -128.0f)
	UINT,   // An unsigned integer
	SINT,   // A signed integer
	UFLOAT, // An unsigned floating point number
	SFLOAT, // A signed floating point number
};
extern Str imageComponentTypeStrings[9];

// TODO: Support more oddball bit layouts
enum class ImageBits : u16 {
	// Can be UNORM, SNORM, USCALED, SSCALED, UINT, SINT, SRGB
	R8,
	R8G8,
	R8G8B8,
	R8G8B8A8, // For SRGB, A is just regular UNORM
	// Can be UNORM, SNORM, USCALED, SSCALED, UINT, SINT, SFLOAT
	R16,
	R16G16,
	R16G16B16,
	R16G16B16A16,
	// Can be UINT, SINT, SFLOAT
	R32,
	R32G32,
	R32G32B32,
	R32G32B32A32,
	R64,
	R64G64,
	R64G64B64,
	R64G64B64A64,
	// Special UNORM-only
	R4G4,
	R4G4B4A4,
	R5G6B5,
	R5G5B5A1,
	// Special other
	A2R10G10B10, // Can be UNORM, SNORM, USCALED, SSCALED, UINT, SINT
	B10G11R11, // Can only be UFLOAT
	E5B9G9R9, // Can only be UFLOAT (E represents a shared exponent)
	// TODO: Compressed formats
};
extern Str imageBitsStrings[23];

enum class BlendMode {
	OPAQUE,
	TRANSPARENT,
	TRANSPARENT_PREMULT,
	ADDITION,
	// TODO: Add more common blend modes
};

// Base device from which everything else is allocated
struct Device;

// Single-thread context for commands
struct Context;

// Shaders, resource bindings
struct Pipeline;

// GPU-side buffer which can be memory mapped host-side and committed (transferred) on a Context
struct Buffer;

// GPU-side image which can be memory mapped host-side and committed (transferred) on a Context
struct Image;

// Describes the ability to render to an Image or a Window surface
struct Framebuffer;

// Describes a window with a surface
struct Window;


// Global settings


void SetAppName(Str appName);

void EnableValidationLayers();



// Initialize the API
[[nodiscard]] Result<VoidResult_t, String> Initialize();

void Deinitialize();


// Window


// Returns the index of the window, used for making a framebuffer, or an error message in a String
// This can only error if Initialize was called before AddWindow
[[nodiscard]] Result<Window*, String> AddWindow(io::Window *window, Str tag = Str());

void FramebufferAddWindow(Framebuffer *framebuffer, Window *window);

void SetVSync(Window *window, bool enable);

[[nodiscard]] Result<VoidResult_t, String> WindowUpdate(Window *window);

[[nodiscard]] Result<VoidResult_t, String> WindowPresent(Window *window);


// Creating new objects


[[nodiscard]] Device* NewDevice(Str tag = Str());

[[nodiscard]] Context* NewContext(Device *device, Str tag = Str());

[[nodiscard]] Pipeline* NewGraphicsPipeline(Device *device, Str tag = Str());

[[nodiscard]] Pipeline* NewComputePipeline(Device *device, Str tag = Str());

// Add a buffer that describes vertex data
[[nodiscard]] Buffer* NewVertexBuffer(Device *device, Str tag = Str());

// Add a buffer that describes indices into a vertex buffer
[[nodiscard]] Buffer* NewIndexBuffer(Device *device, Str tag = Str());

// Add a buffer that can hold a large amount of memory, and supports read, write, and atomic operations
[[nodiscard]] Buffer* NewStorageBuffer(Device *device, Str tag = Str());

// Add a buffer that can hold a small amount of memory, and supports read operations
[[nodiscard]] Buffer* NewUniformBuffer(Device *device, Str tag = Str());

[[nodiscard]] Image* NewImage(Device *device, Str tag = Str());

[[nodiscard]] Framebuffer* NewFramebuffer(Device *device, Str tag = Str());


// Buffer, Image


[[nodiscard]] Result<VoidResult_t, String> ImageSetFormat(Image *image, ImageBits imageBits, ImageComponentType componentType);

[[nodiscard]] Result<VoidResult_t, String> ImageSetSize(Image *image, i32 width, i32 height);

// anisotropy is the number of samples taken in anisotropic filtering (only relevant with mipmapping enabled)
void ImageSetMipmapping(Image *image, bool enableMipmapping, i32 anisotropy = 1);

// shaderStages is a bitmask of ShaderStage
void ImageSetUsageSampled(Image *image, u32 shaderStages);


[[nodiscard]] Result<VoidResult_t, String> BufferSetSize(Buffer *buffer, i64 sizeBytes);

// Pipeline


void PipelineAddShader(Pipeline *pipeline, Str filename, ShaderStage stage);

void PipelineAddBuffer(Pipeline *pipeline, Buffer *buffer);

void PipelineAddImage(Pipeline *pipeline, Image *image);

void PipelineAddVertexInputs(Pipeline *pipeline, ArrayWithBucket<ShaderValueType, 8> inputs);

void PipelineSetBlendMode(Pipeline *pipeline, BlendMode blendMode);


// Context, Commands


[[nodiscard]] Result<VoidResult_t, String> ContextBeginRecording(Context *context);

[[nodiscard]] Result<VoidResult_t, String> ContextBeginRecordingSecondary(Context *context, Framebuffer *framebuffer, i32 subpass);

[[nodiscard]] Result<VoidResult_t, String> ContextEndRecording(Context *context);

[[nodiscard]] Result<VoidResult_t, String> SubmitCommands(Context *context);

// Returns true if Context is still executing
[[nodiscard]] Result<bool, String> ContextIsExecuting(Context *context);

// Returns true if we timed out
[[nodiscard]] Result<bool, String> ContextWaitUntilFinished(Context *context, Nanoseconds timeout=Nanoseconds(INT64_MAX));

[[nodiscard]] Result<VoidResult_t, String> CmdExecuteSecondary(Context *primary, Context *secondary);

[[nodiscard]] Result<VoidResult_t, String> CmdCopyDataToBuffer(Context *context, Buffer *dst, void *src, i64 dstOffset=0, i64 size=0);

[[nodiscard]] Result<VoidResult_t, String> CmdCopyDataToImage(Context *context, Image *dst, void *src);


void CmdBindFramebuffer(Context *context, Framebuffer *framebuffer);

void CmdBindPipeline(Context *context, Pipeline *pipeline);

void CmdBindVertexBuffer(Context *context, Buffer *buffer);

void CmdBindIndexBuffer(Context *context, Buffer *buffer);

void CmdBindUniformBuffer(Context *context, Buffer *buffer, i32 set, i32 binding);

void CmdBindStorageBuffer(Context *context, Buffer *buffer, i32 set, i32 binding);

void CmdBindImageSampler(Context *context, Image *image, i32 set, i32 binding);

// Before recording draw commands, you have to commit all your bindings at once
[[nodiscard]] Result<VoidResult_t, String> CmdCommitBindings(Context *context);

void CmdDrawIndexed(Context *context, i32 count, i32 indexOffset, i32 vertexOffset, i32 instanceCount=1, i32 instanceOffset=0);

} // namespace AzCore::GPU

namespace AzCore {

inline void AppendToString(String &string, GPU::ImageComponentType imageComponentType) {
	AppendToString(string, GPU::imageComponentTypeStrings[(u32)imageComponentType]);
}

inline void AppendToString(String &string, GPU::ImageBits imageBits) {
	AppendToString(string, GPU::imageBitsStrings[(u32)imageBits]);
}

};

#endif // AZCORE_GPU_HPP