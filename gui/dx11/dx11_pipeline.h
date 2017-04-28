#pragma once
#include "dx11_creator.h"
namespace PO
{
	namespace Dx11
	{
		namespace Implement
		{
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

				/*
				template<typename T>
				bool write(cbuffer& cb, T&& t)
				{
					//return cb.write(cp, std::forward<T>(t));
				}
				*/
				/*
				template<typename T>
				bool write_cbuffer(shader_d& sd, size_t solt, T&& t)
				{
					if (cp == nullptr || sd.cbuffer_array.size() >= solt || sd.cbuffer_array[solt] == nullptr) return false;
					auto ptr = sd.cbuffer_array[solt];
					D3D11_MAPPED_SUBRESOURCE DMS;
					HRESULT re = cp->Map(ptr, 0, D3D11_MAP_WRITE_DISCARD, 0, &DMS);
					if (re != S_OK)
						return false;
					try {
						t(DMS.pData, DMS.RowPitch, DMS.DepthPitch);
					}
					catch (...)
					{
						cp->Unmap(ptr, 0);
						throw;
					}
					cp->Unmap(ptr, 0);
					return true;
				}
				*/
			};

			struct input_assember_context_t
			{
				context_ptr& cp;
				input_assember_context_t(context_ptr& c) : cp(c) {}
				size_t max_buffer_solt = 0;
				void bind(input_assember_d& id);
				void unbind();
			};

			struct vertex_shader_context_t : shader_resource_context_t
			{
				using shader_resource_context_t::shader_resource_context_t;
				size_t max_cbuffer = 0;
				void bind(const vertex_shader_d&);
				void unbind();

			};

			struct raterizer_context_t
			{
				context_ptr& cp;
				size_t max_view = 0;
				raterizer_context_t(context_ptr& c) : cp(c) {}
				void bind(const raterizer_d& rs);
				void unbind();
			};

			struct pixel_shader_context_t : shader_resource_context_t
			{
				using shader_resource_context_t::shader_resource_context_t;
				size_t max_cbuffer = 0;
				void bind(const pixel_shader_d&);
				void unbind();
			};

			struct output_merge_context_t
			{
				//todo
				context_ptr& cp;
				output_merge_context_t(context_ptr& c) : cp(c) {}
				void clear_render_target(output_merge_d& omd, size_t solt, float4 color);
				void clear_render_target(output_merge_d& omd, float4 color);
				void clear_depth(output_merge_d& omd, float depth);
				void clear_stencil(output_merge_d& omd, uint8_t ref);
				void clear_depth_stencil(output_merge_d& omd, float depth, uint8_t ref);
				void bind(const output_merge_d&);
				void unbind();
			};

			struct draw_range_context_t
			{
				context_ptr& cp;
				draw_range_context_t(context_ptr& c) : cp(c) {}
				void draw(const draw_range_d& d);
			};

			struct compute_shader_context_t : shader_resource_context_t
			{
				using shader_resource_context_t::shader_resource_context_t;
				void bind(const compute_d& cd);
				void unbind();
			};
		}

		struct pipe_line
		{
			context_ptr ptr;
			using IA_t = Implement::input_assember_context_t; IA_t IA;
			using VS_t = Implement::vertex_shader_context_t; VS_t VS;
			using RA_t = Implement::raterizer_context_t; RA_t RA;
			using PS_t = Implement::pixel_shader_context_t; PS_t PS;
			using OM_t = Implement::output_merge_context_t;	OM_t OM;
			using CS_t = Implement::compute_shader_context_t; CS_t CS;
			using DR_t = Implement::draw_range_context_t; DR_t DR;

			pipe_line(context_ptr cp) : ptr(cp), IA(ptr), VS(ptr), RA(ptr), PS(ptr), OM(ptr), CS(ptr), DR(ptr) {}
			void unbing() {
				IA.unbind(); VS.unbind(); RA.unbind(); PS.unbind(); OM.unbind(); CS.unbind();
			}
		};
	}



}