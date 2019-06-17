#include "hlsl.h"
#include <fstream>
#include <array>
#include "../../po/tool/character_encoding.h"
#include "../../po/tool/file_asset.h"
#include <d3dcompiler.h>
#include <Atlbase.h>

namespace PO::Tool
{
	Tool::binary_file_writer& operator<<(Tool::binary_file_writer& o, const std::tuple<std::string, uint32_t, uint32_t>& input)
	{
		o << std::get<0>(input) << std::get<1>(input) << std::get<2>(input);
		return o;
	}

	Tool::binary_file_reader& operator>>(Tool::binary_file_reader& o, std::tuple<std::string, uint32_t, uint32_t>& output)
	{
		o >> std::get<0>(output) >> std::get<1>(output) >> std::get<2>(output);
		return o;
	}
}



namespace
{
	/*
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
		SFI0 = 0x30494653, // Not really sure�� it stores a value that indicates whether double-precision floating point operations are enabled, but I don��t know why that needs to be in its own chunk
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

	struct RDEF_chunk_scription
	{
		uint32_t constant_buffer_count;
		uint32_t constant_byte_offset;
		uint32_t resource_binding_count;
		uint32_t resource_byte_offset;
		uint32_t minor_version_number;
		uint32_t major_version_number;
		PO::Dx::HLSL::StageType stage_type;
		uint32_t flag;
		uint32_t offset_to_creator;
	};

	struct RDEF_resource_scription
	{
		uint32_t offset_to_name;
		PO::Dx::HLSL::SoltInputType resource_input_type;
		PO::Dx::HLSL::SoltOutputType resource_return_type;
		PO::Dx::HLSL::SoltViewDimension resource_view_dimension;
		uint32_t number_of_samples;
		uint32_t bind_point;
		uint32_t bind_count;
		uint32_t shader_input_flags;
	};
	*/
}

//::ostream& operator<<(std::ostream& s, const ::RDEF_resource_scription& dre);

namespace PO::Dx::HLSL
{

	/*
	reflection::reflection(const std::byte* binary_code)
	{
		re_reflect(binary_code);
	}

	reflection::reflection(reflection&& re):reflection_map(std::move(re.reflection_map)), code_length(re.code_length)
	{
		re.code_length = 0;
	}

	void reflection::re_reflect(const std::byte* binary_code)
	{
		reflection_map.clear();
		code_length = 0;
		if (binary_code != nullptr)
		{
			auto head = reinterpret_cast<const hlsl_head*>(binary_code);
			if (binary_code != nullptr && head->head == HlslStandardHead::DXBC)
			{
				code_length = head->total_size;
				auto chunk_start_offset = reinterpret_cast<const uint32_t*>(binary_code + sizeof(hlsl_head));
				const std::byte* rdef_ptr = nullptr;
				for (size_t i = 0; i < head->chunk_count; ++i)
				{
					auto chunk_type = reinterpret_cast<const hlsl_chunk_scription*>(binary_code + chunk_start_offset[i]);
					if (chunk_type->type == HlslChunkType::RDEF)
					{
						rdef_ptr = reinterpret_cast<const std::byte*>(chunk_type + 1);
						const RDEF_chunk_scription* desc = reinterpret_cast<const RDEF_chunk_scription*>(rdef_ptr);
						uint64_t resource_binding_count = desc->resource_binding_count;
						auto resource_ptr = reinterpret_cast<const RDEF_resource_scription*>(reinterpret_cast<const std::byte*>(desc) + desc->resource_byte_offset);
						if (resource_binding_count > 0)
						{
							for (size_t i = 0; i < resource_binding_count; ++i)
							{
								const RDEF_resource_scription& res = resource_ptr[i];
								reflection_map.insert({
									reinterpret_cast<const char*>(rdef_ptr + resource_ptr[i].offset_to_name),
									bind_type{ res.resource_input_type, res.resource_return_type, res.resource_view_dimension, res.bind_point }
									});
							}
						}
						break;
					}
				}
			}
		}
	}
	*/

	Tool::intrusive_ptr<shader_code> shader_code::create(uint64_t length, const std::byte* code)
	{
		auto* ptr = Tool::aligna_buffer<alignof(shader_code)>::allocate(sizeof(shader_code) + length);
		shader_code* new_code = new (ptr) shader_code{};
		new_code->m_length = length;
		if (code != nullptr)
		{
			auto buffer = static_cast<std::byte*>(ptr) + sizeof(shader_code);
			std::memcpy(buffer, code, length);
			//new_code->m_reflection.re_reflect(buffer);
		}
		return new_code;
	}

	const std::byte* shader_code::code() const noexcept
	{
		return reinterpret_cast<const std::byte*>(this + 1);
	}

	std::byte* shader_code::code() noexcept
	{
		return reinterpret_cast<std::byte*>(this + 1);
	}

	void shader_code::release() noexcept
	{
		this->~shader_code();
		Tool::aligna_buffer<alignof(shader_code)>::release(this);
	}

	Tool::binary_file_writer& operator<<(Tool::binary_file_writer& o, const shader_code_ptr& input)
	{
		if (input)
		{
			uint64_t length = input->length();
			o << length;
			o.write(input->code(), length);
		}
		else {
			uint64_t size = 0;
			o << size;
		}
		return o;
	}

	Tool::binary_file_reader& operator>>(Tool::binary_file_reader& o, shader_code_ptr& input)
	{
		uint64_t length;
		o >> length;
		input = shader_code::create(length, nullptr);
		o.read(input->code(), length);
		//input->reflect().re_reflect(input->code());
		return o;
	}

	/*
	shader_reflection_dx11 shader_code::reflection_dx11() const noexcept
	{
		shader_reflection_dx11 tem;
		D3DReflect(code(), length(), __uuidof(ID3D11ShaderReflection), (void**)&tem);
		return tem;
	}
	*/

	enum class MscfbdxHead : uint64_t
	{
		Head = 0x12345678
	};
	
	struct include_implement : ID3DInclude
	{
		Tool::path m_mscf_path;
		Tool::path m_tmscf_path;
		Tool::path m_root_path;
		Graphic::time_mark& m_time;
		include_implement(const Tool::path& mscf_path, const Tool::path& tmscf_path, const Tool::path& root_path, Graphic::time_mark& time) 
			: m_mscf_path(mscf_path), m_tmscf_path(tmscf_path), m_root_path(root_path), m_time(time){
			m_time.mscf_path = Tool::relative_to(m_mscf_path, m_root_path);
			m_time.mscf_time = Tool::last_write_time_u64(mscf_path);
			if (!m_time.tmscf_path.empty())
			{
				m_time.tmscf_path = Tool::relative_to(m_tmscf_path, m_root_path);
				m_time.tmscf_time = Tool::last_write_time_u64(tmscf_path);
			}
			else {
				m_time.tmscf_time = 0;
			}
				
		}
		STDMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override
		{
			Tool::path file_name = pFileName;
			auto include_file_path = m_mscf_path;
			include_file_path.replace_filename(file_name);
			if (!Tool::fs::exists(include_file_path))
			{
				include_file_path = m_tmscf_path;
				include_file_path.replace_filename(file_name);
				if (!Tool::fs::exists(include_file_path))
					return -1;
			}
			uint64_t time = Tool::last_write_time_u64(include_file_path);
			auto relative_path = Tool::relative_to(include_file_path, m_root_path);
			m_time.dependence_time[relative_path] = time;
			Tool::utf_file_reader file{ include_file_path };
			assert(file.is_open());
			size_t code_length = file.estimation_utf8_count();
			char* code = new char[(code_length == 0) ? 1 : code_length];
			file.read_all_utf8(code, code_length);
			*ppData = code;
			*pBytes = static_cast<UINT>(code_length);
			return S_OK;
		}
		STDMETHOD(Close)(THIS_ LPCVOID pData)
		{
			delete[](pData);
			return S_OK;
		}
	};

	enum class ComplierType
	{
		VS,
		HS,
		DS,
		GS,
		PS,
		CS
	};

	D3D_SHADER_MACRO complier_type_to_marco(ComplierType input)
	{
		switch (input)
		{
		case ComplierType::VS:
			return D3D_SHADER_MACRO{ "COMPLIE_PS", "1" };
		case ComplierType::HS:
			return D3D_SHADER_MACRO{ "COMPLIE_HS", "1" };
		case ComplierType::DS:
			return D3D_SHADER_MACRO{ "COMPLIE_DS", "1" };
		case ComplierType::GS:
			return D3D_SHADER_MACRO{ "COMPLIE_GS", "1" };
		case ComplierType::PS:
			return D3D_SHADER_MACRO{ "COMPLIE_PS", "1" };
		case ComplierType::CS:
			return D3D_SHADER_MACRO{ "COMPLIE_CS", "1" };
		default :
			return D3D_SHADER_MACRO{nullptr, nullptr};
		}
	};

	const char* complier_type_to_version_5_0(ComplierType input)
	{
		switch (input)
		{
		case ComplierType::VS:
			return "vs_5_0";
		case ComplierType::HS:
			return "hs_5_0";
		case ComplierType::DS:
			return "ds_5_0";
		case ComplierType::GS:
			return "gs_5_0";
		case ComplierType::PS:
			return "ps_5_0";
		case ComplierType::CS:
			return "cs_5_0";
		default:
			return "";
		}
	}


	shader_code_ptr complier_shader(ComplierType type, const std::string & raw_code, const std::string& main, ID3DInclude* include)
	{
		//D3DCOMPILE_ENABLE_STRICTNESS
		std::string complie_shader = "COMPLIE_SHADER_" + main;
		D3D_SHADER_MACRO ps_maccro[] = { 
			complier_type_to_marco(type),
			D3D_SHADER_MACRO{ complie_shader.c_str(), "1" },
			D3D_SHADER_MACRO{ nullptr, nullptr }
		};
		CComPtr<ID3DBlob> binary;
		CComPtr<ID3DBlob> error;
		HRESULT res = D3DCompile(raw_code.data(), raw_code.size(), nullptr, ps_maccro, include, main.c_str(), complier_type_to_version_5_0(type),

#ifdef _DEBUG
			D3DCOMPILE_DEBUG
#else
			0
#endif	
			,

			0, &binary, &error
		);
		if (SUCCEEDED(res))
		{
			return shader_code::create(binary->GetBufferSize(), static_cast<const std::byte*>(binary->GetBufferPointer()));
			//return std::move(binary);
		}
		else
			std::cout << complier_type_to_version_5_0(type) << " : " << complie_shader  << "  " << reinterpret_cast<char*>(error->GetBufferPointer()) << std::endl;
		return {};
	}

	Tool::path mscfb_extension =
#ifdef DEBUG
		u".mscfbdx_d";
#else
		u".mscfbdx";
#endif // DEBUG

	std::array<ComplierType, 6> type_index = { ComplierType::VS , ComplierType::HS , ComplierType::DS , ComplierType::GS , ComplierType::PS, ComplierType::CS };

	bool complie(mscfbdx_tech& output, const Graphic::mscf_final& input, include_implement& include)
	{
		uint64_t index = 0;
		std::array<std::map<std::string, std::tuple<shader_code_ptr, uint64_t>>, 6> complied_shader;
		for (auto& ite : input.m_techs)
		{
			for (auto& ite2 : ite.second)
			{
				if (std::holds_alternative<Graphic::mscf_pass>(ite2))
				{
					auto& ref = std::get<Graphic::mscf_pass>(ite2);
					for (size_t i = 0; i < 5; ++i)
					{
						auto& main = ref.shader_main[i];
						if (!main.empty())
						{
							auto ite = complied_shader[i].find(main);
							if (ite == complied_shader[i].end())
							{
								auto ptr = complier_shader(type_index[i], input.code, main, &include);
								if (!ptr)
									return false;
								complied_shader[i].insert({ main,{ std::move(ptr), index++ } });
							}
						}
					}
				}
				else if (std::holds_alternative<Graphic::mscf_compute>(ite2))
				{
					auto& ref = std::get<Graphic::mscf_compute>(ite2);
					if (!ref.shader_cs.empty())
					{
						auto ite = complied_shader[5].find(ref.shader_cs);
						if (ite == complied_shader[5].end())
						{
							auto ptr = complier_shader(ComplierType::CS, input.code, ref.shader_cs, &include);
							complied_shader[5].insert({ ref.shader_cs,{ std::move(ptr), index++ } });
						}
					}
				}
			}
		}
		size_t all_size = 0;
		for (auto& ite : complied_shader)
			all_size += ite.size();
		output.all_code.resize(all_size);
		for (auto& ite : complied_shader)
			for (auto& ite2 : ite)
				output.all_code[std::get<uint64_t>(ite2.second)] = std::move(std::get<shader_code_ptr>(ite2.second));
		
		for (auto& ite : input.m_techs)
		{
			std::vector<std::variant<mscfbdx_pass, mscfbdx_compute>> tech;
			for (auto& ite2 : ite.second)
			{
				if (std::holds_alternative<Graphic::mscf_pass>(ite2))
				{
					mscfbdx_pass pass;
					auto& ref = std::get<Graphic::mscf_pass>(ite2);
					pass.ia = ref.ia;
					for (size_t i = 0; i < 5; ++i)
					{
						auto& main = ref.shader_main[i];
						if (!main.empty())
						{
							auto ite2 = complied_shader[i].find(main);
							pass.shader_index[i] = std::get<uint64_t>(ite2->second);
						}
						else
							pass.shader_index[i] = output.all_code.size();
					}
					pass.stream_out = ref.stream_out;
					/*
					if (!pass.stream_out.empty())
						pass.stream_out_element = ref.stream_out_element;
						*/
					pass.depth_stencil = ref.depth_stencil;
					pass.render_target = ref.render_target;
					tech.push_back(std::move(pass));
				}
				else if (std::holds_alternative<Graphic::mscf_compute>(ite2))
				{
					auto& ref = std::get<Graphic::mscf_compute>(ite2);
					if (!ref.shader_cs.empty())
					{
						auto ite = complied_shader[5].find(ref.shader_cs);
						tech.push_back(mscfbdx_compute{ ref.di, std::get<uint64_t>(ite->second) });
					}
				}
			}
			output.techs.insert({ ite.first, std::move(tech) });
		}
		return true;
	}


	


	Tool::binary_file_writer& operator<<(Tool::binary_file_writer& o, const mscfbdx_pass& input)
	{
		o << input.ia;
		for (size_t i = 0; i < 5; ++i)
		{
			o << input.shader_index[i];
		}
		o << input.depth_stencil << input.stream_out 
			//<< input.stream_out_element
			<< input.render_target;
		return o;
	}

	Tool::binary_file_reader& operator>>(Tool::binary_file_reader& o, mscfbdx_pass& output)
	{
		o >> output.ia;
		for (size_t i = 0; i < 5; ++i)
		{
			o >> output.shader_index[i];
		}
		o >> output.depth_stencil >> output.stream_out 
			//>> output.stream_out_element 
			>> output.render_target;
		return o;
	}

	Tool::binary_file_writer& operator<<(Tool::binary_file_writer& o, const mscfbdx_compute& input)
	{
		o << input.di << input.shader_cs;
		return o;
	}

	Tool::binary_file_reader& operator>>(Tool::binary_file_reader& o, mscfbdx_compute& output)
	{
		o >> output.di >> output.shader_cs;
		return o;
	}

	Tool::binary_file_writer& operator<<(Tool::binary_file_writer& o, const std::variant<mscfbdx_pass, mscfbdx_compute>& input)
	{
		if (std::holds_alternative<mscfbdx_pass>(input))
		{
			o << uint64_t(0);
			auto& ref = std::get<mscfbdx_pass>(input);
			o << ref;
		}
		else if (std::holds_alternative<mscfbdx_compute>(input))
		{
			o << uint64_t(1);
			auto& ref = std::get<mscfbdx_compute>(input);
			o << ref;
		}
		else {
			o << uint64_t(2);
		}
		return o;
	}

	Tool::binary_file_reader& operator>>(Tool::binary_file_reader& o, std::variant<mscfbdx_pass, mscfbdx_compute>& input)
	{
		uint64_t index;
		o >> index;
		if (index == 0)
		{
			mscfbdx_pass tem;
			o >> tem;
			input = tem;
		}
		else if (index == 1)
		{
			mscfbdx_compute index2;
			o >> index2;
			input = std::move(index2);
		}
		else {
			input = std::variant<mscfbdx_pass, mscfbdx_compute>{};
		}
		return o;
	}

	Tool::binary_file_writer& operator<<(Tool::binary_file_writer& o, const mscfbdx_tech& input)
	{
		o << input.all_code;
		return o << input.techs;
	}

	Tool::binary_file_reader& operator>>(Tool::binary_file_reader& o, mscfbdx_tech& input)
	{
		o >> input.all_code;
		return o >> input.techs;
	}

	bool save_mscfbdx_tech(const mscfbdx_tech& input, const Tool::path& path)
	{
		Tool::binary_file_writer bfw{ path };
		if (bfw.is_open())
		{
			bfw << input;
			return true;
		}
		return false;
	}

	bool load_mscfbdx_tech(mscfbdx_tech& output, const Tool::path& path)
	{
		Tool::binary_file_reader bfw{ path };
		if (bfw.is_open())
		{
			bfw >> output;
			return true;
		}
		return false;
	}
	
	bool complie_and_save(
		const Graphic::mscf_code& mscf, const Tool::path& mscf_path,
		const Graphic::tmscf_code& tmscf, const Tool::path& tmscf_path,
		const Tool::path& target_path, include_implement& in
		)
	{
		Graphic::mscf_final total;
		if (Graphic::complie_mscf(total, mscf, tmscf))
		{
			{
				auto pre_code_path = target_path;
				pre_code_path += ".pre_code";
				Tool::utf_file_writer file{ pre_code_path };
				file.write(total.code);
			}
			mscfbdx_tech tech;
			if (complie(tech, total, in))
			{
				{
					Tool::binary_file_writer file{ target_path };
					file << tech;
				}
				std::cout << "complie success." << std::endl;
			}
		}
		return true;
	}

	
	bool try_complie_mscf(const Graphic::mscf_code& setting, const Tool::path& mscf_path, const Tool::path& tmscf_path, const Tool::path& root_path)
	{
		if (!tmscf_path.empty())
			std::cout << "try complie :<" << Tool::relative_to(mscf_path, root_path).u8string() << "> with <" << Tool::relative_to(tmscf_path, root_path).u8string() << ">:";
		else
			std::cout << "try complie :<" << Tool::relative_to(mscf_path, root_path).u8string();
		auto target_path = mscf_path;
		target_path.replace_extension(mscfb_extension);
		Graphic::time_mark time;
		include_implement in{ mscf_path, tmscf_path, root_path, time };
		Graphic::tmscf_code tmscf;
		if (!tmscf_path.empty())
		{
			if (!Graphic::load_tmscf(tmscf, tmscf_path))
				return false;
		}
		if (complie_and_save(setting, mscf_path, tmscf, tmscf_path, target_path, in))
		{
			auto time_path = target_path;
			time_path += Graphic::time_mark_extension;

			time.mscfb_path = Tool::relative_to(target_path, root_path);
			time.mscfb_time = Tool::last_write_time_u64(target_path);
			Graphic::save_time_mark(time, time_path);
			return true;
		}
		return false;
	}
	
	void try_complie_all_mscf(const Tool::relative_path_map& rpm)
	{
		rpm.find_extension(u".mscf", [&](const Tool::relative_path_map::description& des) -> bool {
			Graphic::time_mark time;
			auto time_path = des.m_absolutely_path;
			time_path.replace_extension(mscfb_extension);
			time_path += Graphic::time_mark_extension;
			std::cout << "check :" << des.m_absolutely_path;
			if (Graphic::load_time_mark(time, time_path) && !Graphic::check_time_mark(time, rpm.root()))
			{
				std::cout << ": no need update" << std::endl;
				return true;
			}
			std::cout << ": need update" << std::endl;
			Graphic::mscf_code property;
			if (Graphic::load_mscf(property, des.m_absolutely_path))
			{
				if (!property.m_pattern.empty())
				{
					rpm.find_extension(u".tmscf", [&](const Tool::relative_path_map::description& des2) -> bool {
						if (des2.m_file_name_without_extension == property.m_pattern)
							HLSL::try_complie_mscf(property, des.m_absolutely_path, des2.m_absolutely_path, rpm.root());
						return false;
					});
				}
				else {
					return HLSL::try_complie_mscf(property, des.m_absolutely_path, {}, rpm.root());
				}
				return true;
			}
			else
				return false;
			
			
			
			/*
			//Tool::utf_file file{ des.m_absolutely_path };
			Graphic::mscf_code property;
			if (Graphic::load_mscf(property, des.m_absolutely_path))
			{
				if (!property.m_pattern.empty())
				{
					rpm.find_extension(u".tmscf", [&](const Tool::relative_path_map::description& des2) -> bool {
						if (des2.m_file_name_without_extension == property.m_pattern)
							HLSL::try_complie_mscf(property, des.m_absolutely_path, des2.m_absolutely_path, rpm.root());
						return false;
					});
				}
				else {
					HLSL::try_complie_mscf(property, des.m_absolutely_path, {}, rpm.root());
				}
			}
			return true;
			*/
		});
	}
}

/*
std::ostream& operator<<(std::ostream& s, PO::Dx::HLSL::SoltInputType type)
{
	switch (type)
	{
	case PO::Dx::HLSL::SoltInputType::CB:
		return s << "cbuffer";
		break;
	case PO::Dx::HLSL::SoltInputType::SRV:
		return s << "shader resource view";
		break;
	case PO::Dx::HLSL::SoltInputType::RWURV:
		return s << "RW unordered resource view";
		break;
	case PO::Dx::HLSL::SoltInputType::SS:
		return s << "sampler state";
		break;
	default:
		return s << "unknow{" << static_cast<uint32_t>(type) << "}";
		break;
	}
}

std::ostream& operator<<(std::ostream& s, PO::Dx::HLSL::SoltOutputType type)
{
	switch (type)
	{
	case PO::Dx::HLSL::SoltOutputType::NO:
		return s << "no";
		break;
	case PO::Dx::HLSL::SoltOutputType::FLOAT4:
		return s << "float4";
		break;
	case PO::Dx::HLSL::SoltOutputType::UINT4:
		return s << "uint4";
		break;
	default:
		return s << "unknow{" << static_cast<uint32_t>(type) << "}";
		break;
	}
}

std::ostream& operator<<(std::ostream& s, PO::Dx::HLSL::SoltViewDimension type)
{
	switch (type)
	{
	case PO::Dx::HLSL::SoltViewDimension::NO:
		return s << "NO";
		break;
	case PO::Dx::HLSL::SoltViewDimension::T2D:
		return s << "Texture2D";
		break;
	default:
		return s << "unknow{" << static_cast<uint32_t>(type) << "}";
		break;
	}
}

std::ostream& operator<<(std::ostream& s, const ::RDEF_resource_scription& dre)
{
	return s << "resource type: " << dre.resource_input_type << " ; return type: " << dre.resource_return_type << " ; bind point: " << dre.bind_point;
}
*/

/*
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
*/