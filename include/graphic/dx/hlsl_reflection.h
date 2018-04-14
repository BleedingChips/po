#pragma once
#include <stdint.h>
#include <functional>
#include <iostream>
namespace PO::Dx
{
	namespace Implement
	{
		enum class HlslStandardHead : uint32_t
		{
			DXBC = 0x43425844 // Standard head
		};


		struct hlsl_head
		{
			HlslStandardHead head;
			uint32_t check_sum[4];
			uint32_t number_one;
			uint32_t total_size;
			uint32_t chunk_count;
		};

		enum class HlslChunkType : uint32_t
		{
			ICFE = 0x45464349, // Interface. Describes any interfaces, and implementing classes, present in the source HLSL.
			ISGN = 0x4E475349, // Input signature
			OSG5 = 0x3547534F, // Output signature (SM5)
			OSGN = 0x4E47534F, // Output signature
			PCSG = 0x47534350, // Patch constant signature
			RDEF = 0x46454452, // Resource definition. Describes constant buffers and resource bindings.
			SDGB = 0x42474453, // Shader debugging info (old-style)
			SFI0 = 0x30494653, // Not really sure¡­ it stores a value that indicates whether double-precision floating point operations are enabled, but I don¡¯t know why that needs to be in its own chunk
			SHDR = 0x52444853, // Shader (SM4). The shader itself.
			SHEX = 0x58454853, // Shader (SM5)
			SPDB = 0x42445053, // Shader debugging info (new-style)
			STAT = 0x54415453 // Statistics. Useful statistics about the shader, such as instruction count, declaration count, etc.
		};

		struct hlsl_chunk_scription
		{
			HlslChunkType type;
			uint32_t chunk_length;
		};

		enum class RDEFProgrameType : uint32_t
		{
			VS = 0xFFFE // vertex shader
		};

		struct RDEF_chunk_scription
		{
			uint32_t constant_buffer_count;
			uint32_t constant_byte_offset;
			uint32_t resource_binding_count;
			uint32_t resource_byte_offset;
			uint32_t minor_version_number;
			uint32_t major_version_number;
			RDEFProgrameType programe_type;
			uint32_t flag;
			uint32_t offset_to_creator;
		};

		enum class ResourceInputType : uint32_t
		{
			CB = 0,
			SRV = 2,
			RWURV = 4,
			SS = 3
		};

		enum class ResourceReturnType : uint32_t
		{
			NO = 0,
			FLOAT4 = 5,
			UINT4 = 4,
		};

		enum class ResourceViewDimension : uint32_t
		{
			NO = 0,
			T2D = 4
		};

		struct RDEF_resource_scription
		{
			uint32_t offset_to_name;
			ResourceInputType resource_input_type;
			ResourceReturnType resource_return_type;
			ResourceViewDimension resource_view_dimension;
			uint32_t number_of_samples;
			uint32_t bind_point;
			uint32_t bind_count;
			uint32_t shader_input_flags;
		};
	}
}

std::ostream& operator<<(std::ostream& s, PO::Dx::Implement::ResourceInputType type);

std::ostream& operator<<(std::ostream& s, PO::Dx::Implement::ResourceReturnType type);
std::ostream& operator<<(std::ostream& s, PO::Dx::Implement::ResourceViewDimension type);

std::ostream& operator<<(std::ostream& s, const PO::Dx::Implement::RDEF_resource_scription& dre);

void load_resource_binding(std::byte* hlsl_code, size_t code_size);