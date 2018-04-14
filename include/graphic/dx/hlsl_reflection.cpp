#include "hlsl_reflection.h"
namespace PO::Dx
{

}

std::ostream& operator<<(std::ostream& s, PO::Dx::Implement::ResourceInputType type)
{
	switch (type)
	{
	case PO::Dx::Implement::ResourceInputType::CB:
		return s << "cbuffer";
		break;
	case PO::Dx::Implement::ResourceInputType::SRV:
		return s << "shader resource view";
		break;
	case PO::Dx::Implement::ResourceInputType::RWURV:
		return s << "RW unordered resource view";
		break;
	case PO::Dx::Implement::ResourceInputType::SS:
		return s << "sampler state";
		break;
	default:
		return s << "unknow{" << static_cast<uint32_t>(type) << "}";
		break;
	}
}

std::ostream& operator<<(std::ostream& s, PO::Dx::Implement::ResourceReturnType type)
{
	switch (type)
	{
	case PO::Dx::Implement::ResourceReturnType::NO:
		return s << "no";
		break;
	case PO::Dx::Implement::ResourceReturnType::FLOAT4:
		return s << "float4";
		break;
	case PO::Dx::Implement::ResourceReturnType::UINT4:
		return s << "uint4";
		break;
	default:
		return s << "unknow{" << static_cast<uint32_t>(type) << "}";
		break;
	}
}

std::ostream& operator<<(std::ostream& s, PO::Dx::Implement::ResourceViewDimension type)
{
	switch (type)
	{
	case PO::Dx::Implement::ResourceViewDimension::NO:
		return s << "NO";
		break;
	case PO::Dx::Implement::ResourceViewDimension::T2D:
		return s << "Texture2D";
		break;
	default:
		return s << "unknow{" << static_cast<uint32_t>(type) << "}";
		break;
	}
}

std::ostream& operator<<(std::ostream& s, const PO::Dx::Implement::RDEF_resource_scription& dre)
{
	return s << "resource type: " << dre.resource_input_type << " ; return type: " << dre.resource_return_type << " ; bind point: " << dre.bind_point;
}

void load_resource_binding(std::byte* hlsl_code, size_t code_size)
{
	std::cout << code_size << std::endl;
	auto head = reinterpret_cast<PO::Dx::Implement::hlsl_head*>(hlsl_code);
	if (head->head != PO::Dx::Implement::HlslStandardHead::DXBC)
		__debugbreak();
	uint32_t chunk_count = head->chunk_count;
	std::cout << chunk_count << std::endl;
	auto data_start = hlsl_code + sizeof(PO::Dx::Implement::hlsl_head);
	for (uint32_t i = 0; i < chunk_count; ++i)
	{
		auto chunk = reinterpret_cast<PO::Dx::Implement::hlsl_chunk_scription*>(hlsl_code + *reinterpret_cast<uint32_t*>(data_start));
		if (chunk->type == PO::Dx::Implement::HlslChunkType::RDEF)
		{
			auto chunk_start = reinterpret_cast<PO::Dx::Implement::RDEF_chunk_scription*>(chunk + 1);
			auto resource_bind = reinterpret_cast<PO::Dx::Implement::RDEF_resource_scription*>(reinterpret_cast<std::byte*>(chunk_start) + chunk_start->resource_byte_offset);
			uint32_t resource_count = chunk_start->resource_binding_count;
			for (size_t i = 0; i < resource_count; ++i)
			{
				std::cout << (reinterpret_cast<char*>(chunk_start) + (resource_bind + i)->offset_to_name) <<
					" : " << *(resource_bind + i) << std::endl;
			}
			break;
		}
	}
}