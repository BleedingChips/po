#pragma once
#include "dx11_creator.h"
namespace PO
{
	namespace Dx11
	{
		namespace Implement
		{
			/*
			struct shader_resource_context_t
			{
				context_ptr& cp;
				shader_resource_context_t(context_ptr& c) : cp(c) { }
				template<typename T> bool write(shader_d& b, size_t o, T&& t)
				{
					if (b.cbuffer_array.size() <= o) return false;
					auto& buffer = b.cbuffer_array[o];
					if (buffer == nullptr) return false;
					D3D11_MAPPED_SUBRESOURCE DMS;
					if (SUCCEEDED(cp->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &DMS)))
					{
						Tool::at_scope_exit ate([&, this]() { cp->Unmap(buffer, 0); });
						return t(DMS.pData, DMS.RowPitch, DMS.DepthPitch), true;
					}
					return false;
				}
			};*/

			struct input_assember_context_t
			{
				size_t max_buffer_solt = 0;
				void bind(context_ptr& cp, const input_assember_d& id);
				void unbind(context_ptr& cp);
			};

			struct vertex_shader_context_t
			{
				size_t max_cbuffer = 0;
				void bind(context_ptr& cp, const vertex_shader_d&);
				void unbind(context_ptr& cp);
			};

			struct raterizer_context_t
			{
				size_t max_view = 0;
				void bind(context_ptr& cp, const raterizer_d& rs);
				void unbind(context_ptr& cp);
			};

			struct pixel_shader_context_t
			{
				size_t max_cbuffer = 0;
				void bind(context_ptr& cp, const pixel_shader_d&);
				void unbind(context_ptr& cp);
			};

			struct output_merge_context_t
			{
				void clear_render_target(context_ptr& cp, output_merge_d& omd, size_t solt, float4 color);
				void clear_render_target(context_ptr& cp, output_merge_d& omd, float4 color);
				void clear_depth(context_ptr& cp, output_merge_d& omd, float depth);
				void clear_stencil(context_ptr& cp, output_merge_d& omd, uint8_t ref);
				void clear_depth_stencil(context_ptr& cp, output_merge_d& omd, float depth, uint8_t ref);
				void bind(context_ptr& cp, const output_merge_d&);
				void unbind(context_ptr& cp);
			};

			struct draw_range_context_t
			{
				enum class type
				{
					NONE,
					COMPUTE,
					RENDERER
				};
				type draw_type = type::NONE;
				void draw(context_ptr& cp, const draw_range_d& d);
				type unbind(context_ptr& cp);
			};

			struct compute_shader_context_t
			{
				size_t max_cbuffer = 0;
				void bind(context_ptr& cp, const compute_d& cd);
				void unbind(context_ptr& cp);
			};
		}

		struct pipe_line
		{
			context_ptr ptr;
			Implement::input_assember_context_t IA;
			Implement::vertex_shader_context_t VS;
			Implement::raterizer_context_t RA;
			Implement::pixel_shader_context_t PS;
			Implement::output_merge_context_t OM;

			Implement::compute_shader_context_t CS;

			Implement::draw_range_context_t DR;

			pipe_line(context_ptr cp) :ptr(cp) {}
			void unbind();

			void clear_bind() {
				CS.unbind(ptr); IA.unbind(ptr); VS.unbind(ptr); RA.unbind(ptr);
				PS.unbind(ptr); OM.unbind(ptr);
			}

			void draw(const draw_range_d& d) {
				DR.draw(ptr, d); 
			}
			void bind(const input_assember_d& d) { IA.bind(ptr, d); }
			void bind(const vertex_shader_d& d) { VS.bind(ptr, d); }
			void bind(const pixel_shader_d& d) { PS.bind(ptr, d); }
			void bind(const output_merge_d& d) { OM.bind(ptr, d); }
			void bind(const raterizer_d& rs) { RA.bind(ptr, rs); }
			void bind(const compute_d& cd) { CS.bind(ptr, cd); }

			void clear_render_target(output_merge_d& omd, size_t solt, float4 color) { OM.clear_render_target(ptr, omd, solt, color); }
			void clear_render_target(output_merge_d& omd, float4 color){ OM.clear_render_target(ptr, omd, color); }
			void clear_depth(output_merge_d& omd, float depth){ OM.clear_depth(ptr, omd, depth); }
			void clear_stencil(output_merge_d& omd, uint8_t ref){ OM.clear_stencil(ptr, omd, ref); }
			void clear_depth_stencil(output_merge_d& omd, float depth, uint8_t ref) { OM.clear_depth_stencil(ptr, omd, depth, ref); }

			template<typename T> bool write_cbuffer(cbuffer& b, T&& t)
			{
				if (b.ptr == nullptr) return false;
				D3D11_MAPPED_SUBRESOURCE DMS;
				if (SUCCEEDED(ptr->Map(b.ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &DMS)))
				{
					Tool::at_scope_exit ate([&, this]() { ptr->Unmap(b.ptr, 0); });
					return t(DMS.pData, DMS.RowPitch, DMS.DepthPitch), true;
				}
				return false;
			}

			template<typename T> bool write_cbuffer(shader_d& b, size_t o, T&& t)
			{
				if (b.cbuffer_array.size() <= o) return false;
				cbuffer ptr;
				ptr.ptr = b.cbuffer_array[o];
				return write_cbuffer(ptr, t);
			}
		};
	}



}