#pragma once
#include "../po/tool/asset.h"
#include <stdint.h>
#include <functional>
#include <iostream>
#include <string>
#include <d3dcompiler.h>
#include <Atlbase.h>
#include "../po/tool/document.h"
//#include "../../graphic/interface/material.h"
namespace PO::Dx::HLSL
{
	//extern Tool::path mscfb_extension;

	/*
	enum class StageType : uint32_t
	{
		VS = 0xFFFE, // vertex shader
		PS = 0x00D0,
		GS = 0x003C
	};

	enum class SoltInputType : uint32_t
	{
		CB = 0,
		SRV = 2,
		RWURV = 4,
		SS = 3
	};

	enum class SoltOutputType : uint32_t
	{
		NO = 0,
		FLOAT4 = 5,
		UINT4 = 4,
	};

	enum class SoltViewDimension : uint32_t
	{
		NO = 0,
		T2D = 4
	};

	struct bind_type
	{
		SoltInputType input;
		SoltOutputType output;
		SoltViewDimension view;
		uint32_t solt;
	};

	struct reflection
	{
		reflection(const std::byte* binary_code = nullptr);
		reflection(const reflection&) = default;
		reflection(reflection&&);
		void re_reflect(const std::byte* binary_code);
		uint32_t size() const noexcept { return code_length; }
		operator bool() const noexcept { return code_length != 0; }
	private:
		std::map<std::string, bind_type> reflection_map;
		uint32_t code_length;
	};
	*/

	/*
	struct gscfbdx_asset : Tool::intrusive_object_base
	{
		static Tool::intrusive_ptr<gscfbdx_asset> create(const Tool::path& path);
		const std::byte* code() const noexcept { return reinterpret_cast<const std::byte*>(this + 1); }
		uint32_t size() const { return m_reflect.size(); }
		const reflection& reflect() const noexcept { return m_reflect; }
	private:
		void release() noexcept override;
		gscfbdx_asset(const std::byte* code);
		reflection m_reflect;
	};
	*/

	//using shader_reflection_dx10 = CComPtr<ID3D10ShaderReflection>;
	//using shader_reflection_dx11 = CComPtr<ID3D11ShaderReflection>;

	/*
	struct shader_code : Tool::intrusive_object<shader_code>
	{
		static Tool::intrusive_ptr<shader_code> create(uint64_t, const std::byte*);
		uint64_t length() const noexcept { return m_length; }
		//reflection& reflect() noexcept { return m_reflection; }
		//const reflection& reflect() const noexcept { return m_reflection; }
		const std::byte* code() const noexcept;
		std::byte* code() noexcept;
		void release() noexcept;
		//shader_reflection_dx11 reflection_dx11() const noexcept;
	private:
		uint64_t m_length;
	};

	using shader_code_ptr = Tool::intrusive_ptr<shader_code>;

	Tool::binary_file_writer& operator<<(Tool::binary_file_writer& o, const shader_code_ptr& input);
	Tool::binary_file_reader& operator>>(Tool::binary_file_reader& o, shader_code_ptr& input);

	struct mscfbdx_pass
	{
		std::string ia;
		uint64_t shader_index[5];
		std::string depth_stencil;
		std::string stream_out;
		//std::vector<std::tuple<std::string, uint32_t, uint32_t>> stream_out_element;
		std::vector<std::string> render_target;
	};

	struct mscfbdx_compute
	{
		std::string di;
		uint64_t shader_cs;
	};

	struct mscfbdx_tech
	{
		std::vector<shader_code_ptr> all_code;
		std::map<std::string, std::vector<std::variant<mscfbdx_pass, mscfbdx_compute>>> techs;
	};

	bool save_mscfbdx_tech(const mscfbdx_tech& input, const Tool::path& path);
	bool load_mscfbdx_tech(mscfbdx_tech& output, const Tool::path& path);

	//bool try_complie_mscf(const );


	//bool load_mscfbdx(mscfbdx_tech& mscfbdx, const Tool::path&);

	//bool try_complie_mscf(const Graphic::mscf_code& setting, const Tool::path& mscf_path, const Tool::path& tmscf_path);
	void try_complie_all_mscf(const Tool::relative_path_map& rpm);
	*/
	

	

	//void load_mscf(const Tool::path&, Graphic::mscf_techs&, std::map<std::string, >)

	//bool try_rebuild_mscfbdx(const Tool::path& path, const Tool::relative_path_map& a);
	//void try_rebuild_gscfbdx(const Tool::path& path, const Tool::asset_interface& a);
	/*
	struct hlsl_asset
	{
		const mscfbdx_asset* find_mscf(Tool::asset::id) const noexcept;
		const gscfbdx_asset* find_gscf(Tool::asset::id) const noexcept;
		const mscfbdx_asset* find_mscf(const Tool::path& p, const Tool::asset& a) const noexcept;
		const gscfbdx_asset* find_gscf(const Tool::path& p, const Tool::asset& a) const noexcept;
		size_t load_asset(const Tool::asset_preload& ap);
		bool load_asset(Tool::asset::id);
	private:
		std::map<Tool::asset::id, gscfbdx_asset> m_gscfbdx_map;
		std::map<Tool::asset::id, mscfbdx_asset> m_mscfbdx_map;
	};
	*/
}

//std::ostream& operator<<(std::ostream& s, PO::Dx::HLSL::SoltInputType type);
//std::ostream& operator<<(std::ostream& s, PO::Dx::HLSL::SoltOutputType type);
//std::ostream& operator<<(std::ostream& s, PO::Dx::HLSL::SoltViewDimension type);