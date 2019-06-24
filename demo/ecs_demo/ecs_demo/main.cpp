#include "../../..//include/po/frame/ecs/implement.h"
#include "..//..//..//include/win32/form.h"
#include "../..//..//include/dx11/context.h"
#include "../..//..//include/po/tool/document.h"
#include <random>
#include <iostream>
#include <math.h>

using namespace std;
using namespace PO;
using namespace PO::ECS;

struct Location
{
	float x, y;
};

struct Collision
{
	float Range;
};

struct Velocity
{
	float x, y;
};

template<typename Type> using ComPtr = Dx11::ComPtr<Type>;
using ComWrapper = Dx11::ComWrapper;

std::mutex cout_mutex;

template<typename Type> struct CallRecord
{
	CallRecord()
	{
		std::lock_guard lg(cout_mutex);
		std::cout << "thread id<" << std::this_thread::get_id() << "> : " << typeid(Type).name() << " - start" << std::endl;
	}

	~CallRecord()
	{
		std::lock_guard lg(cout_mutex);
		std::cout << "thread id<" << std::this_thread::get_id() << "> : " << typeid(Type).name() << " - end" << std::endl;
	}
};

struct CollisionSystem
{
	std::vector<std::pair<Entity, Entity>> all_entity;
	void operator()(Filter<const Location, const Collision>& f, EventProvider<uint64_t>& ep)
	{
		all_entity.clear();
		CallRecord<CollisionSystem> record;
		uint64_t collition_count = 0;
		for (auto ite = f.begin(); ite != f.end(); ++ite)
		{
			auto& [l, c] = ite.components();
			auto ite2 = ite;
			for (++ite2; ite2 != f.end(); ++ite2)
			{
				auto& [l2, c2] = ite2.components();
				auto xs = l.x - l2.x;
				auto ys = l.y - l2.y;
				auto rs = c.Range + c2.Range;
				if ((xs * xs + ys * ys) < rs * rs)
				{
					all_entity.push_back({ ite.entity(), ite2.entity() });
					++collition_count;
				}
					
			}
		}
		ep.push(collition_count);
		{
			std::lock_guard lg(cout_mutex);
			std::cout << " Event Count : " << collition_count << std::endl;
		}
	}
};

struct DestorySystem
{
	void operator()(SystemFilter<const CollisionSystem>& s, Context& con)
	{
		if (s)
		{
			CallRecord<DestorySystem> record;
			for (auto& ite : s->all_entity)
			{
				if (ite.first.have<Velocity>())
					con.destory_entity(ite.first);
				if (ite.second.have<Velocity>())
					con.destory_entity(ite.second);
			}
		}
	}
};

struct RenderSystem
{
	void operator()(Filter<const Collision, const Location>& f, GobalFilter<Dx11::FormRenderer>& render)
	{
		CallRecord<RenderSystem> record;
		if (render->ready_to_update())
		{
			float p[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			(*render)->ClearRenderTargetView(*render, p);
			if (f.count() > 0)
			{
				size_t count = f.count();
				struct Poi
				{
					float lx, ly;
					float range, pro;
				};
				std::vector<Poi> data;
				data.resize(count);
				size_t i = 0;
				for (auto ite : f)
				{
					auto& [entity, tup] = ite;
					auto& [col, loc] = tup;
					data[i].lx = loc.x;
					data[i].ly = loc.y;
					data[i].range = col.Range;
					data[i].pro = entity.have<Velocity>() ? 0.0f : 1.0f;
					++i;
				}
				D3D11_BUFFER_DESC DBD{ sizeof(Poi) * data.size(), D3D11_USAGE_IMMUTABLE , D3D11_BIND_VERTEX_BUFFER, 0, 0, sizeof(Poi) };
				ComPtr<ID3D11Buffer> ins_buffer;
				D3D11_SUBRESOURCE_DATA DSD{ data.data(), 0, 0 };
				HRESULT re = (*context)->CreateBuffer(&DBD, &DSD, ComWrapper::ref(ins_buffer));
				assert(SUCCEEDED(re));

				(*render)->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				(*render)->IASetInputLayout(layout);
				ID3D11Buffer* tem_buffer[2] = { buffer, ins_buffer };
				UINT str[2] = { sizeof(float) * 2, sizeof(float) * 4 };
				UINT offset[2] = { 0, 0 };
				(*render)->IASetVertexBuffers(0, 2, tem_buffer, str, offset);
				(*render)->VSSetShader(vs, nullptr, 0);
				(*render)->PSSetShader(ps, nullptr, 0);
				ID3D11RenderTargetView* view[1] = { (*render) };
				(*render)->OMSetRenderTargets(1, view, nullptr);
				D3D11_VIEWPORT viewport{ 0.0, 0.0, 1024.0f, 768.0f, 0.0, 1.0 };
				(*render)->RSSetViewports(1, &viewport);
				(*render)->DrawInstanced(13 * 3, count, 0, 0);
			}
			(*render).replaceable_updates();
		}
	}
	RenderSystem(Dx11::ContextPtr input_context)
	{
		context = std::move(input_context);
		assert(context);
		struct Point
		{
			float x;
			float y;
		};
		std::array<Point, 13 * 3> all_buffer;
		for (size_t i = 0; i < 13; ++i)
		{
			all_buffer[i * 3] = Point{0.0f, 0.0f};
			all_buffer[i * 3 + 1] = Point{ sinf(3.141592653f * 2.0f/ 13 * i), cos(3.141592653f * 2.0f / 13 * i) };
			all_buffer[i * 3 + 2] = Point{ sinf(3.141592653f * 2.0f/ 13 * (i + 1)), cos(3.141592653f * 2.0f / 13 * (i+1)) };
		}
		D3D11_BUFFER_DESC DBD{sizeof(Point) * 13 * 3, D3D11_USAGE_IMMUTABLE , D3D11_BIND_VERTEX_BUFFER, 0, 0, sizeof(Point) };
		D3D11_SUBRESOURCE_DATA DSD{ all_buffer .data(), 0, 0};
		HRESULT re = (*context)->CreateBuffer(&DBD, &DSD, ComWrapper::ref(buffer));
		assert(SUCCEEDED(re));
		Doc::loader_binary vsb_doc(L"VertexShader.cso");
		assert(vsb_doc.is_open());
		std::vector<std::byte> vsb;
		vsb.resize(vsb_doc.last_size());
		vsb_doc.read(vsb.data(), vsb.size());
		re = (*context)->CreateVertexShader(vsb.data(), vsb.size(), nullptr, ComWrapper::ref(vs));
		assert(SUCCEEDED(re));
		Doc::loader_binary psb_doc(L"PixelShader.cso");
		assert(psb_doc.is_open());
		std::vector<std::byte> psb;
		psb.resize(psb_doc.last_size());
		psb_doc.read(psb.data(), psb.size());
		re = (*context)->CreatePixelShader(psb.data(), psb.size(), nullptr, ComWrapper::ref(ps));
		assert(SUCCEEDED(re));
		D3D11_INPUT_ELEMENT_DESC input_desc[] = {
			D3D11_INPUT_ELEMENT_DESC {"VERPOSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			D3D11_INPUT_ELEMENT_DESC {"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			D3D11_INPUT_ELEMENT_DESC {"RANGE", 0, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, 1, sizeof(float) * 2, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			D3D11_INPUT_ELEMENT_DESC {"PROPERTY", 0, DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT, 1, sizeof(float) * 3, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
		re = (*context)->CreateInputLayout(input_desc, 4, vsb.data(), vsb.size(), ComWrapper::ref(layout));
		assert(SUCCEEDED(re));
	}
private:
	Dx11::ContextPtr context;
	ComPtr<ID3D11Buffer> buffer;
	ComPtr<ID3D11VertexShader> vs;
	ComPtr<ID3D11PixelShader> ps;
	ComPtr<ID3D11InputLayout> layout;
};

struct FormUpdateSystem
{
	void operator()(GobalFilter<Dx11::FormRenderer>& render, Context& c)
	{
		if (!render)
			return;
		CallRecord<FormUpdateSystem> record;
		MSG msg;
		while (render->pook_event(msg))
		{
			if (msg.message == WM_CLOSE)
			{
				c.exit();
				break;
			}
		}
	}

	TickOrder tick_order(const TypeLayout& layout) const noexcept
	{
		if (layout == TypeLayout::create<RenderSystem>())
			return TickOrder::After;
		return TickOrder::Mutex;
	}
};



int main()
{
	{
		auto context = PO::Dx11::Context::create();
		ContextImplement imp;
		imp.create_gobal_component<Dx11::FormRenderer>(context->create_form());
		imp.set_thread_reserved(2);
		imp.set_duration(duration_ms{100});
		imp.create_system<CollisionSystem>();
		imp.create_system<DestorySystem>();
		imp.create_system<RenderSystem>(context);
		imp.create_system<FormUpdateSystem>();
		//imp.create_system<MarkStartSystem>();
		//imp.create_system<MarkEndSystem>();

		imp.create_system([&]() {
			std::lock_guard lg(cout_mutex);
			std::cout << "loop start --------------" << std::endl;
		}, TickPriority::HighHigh, TickPriority::HighHigh);
		imp.create_system([&]() {
			std::lock_guard lg(cout_mutex);
			std::cout << "loop end --------------" << std::endl;
		}, TickPriority::LowLow, TickPriority::LowLow);

		std::random_device r_dev;
		std::default_random_engine engine(r_dev());
		std::uniform_real_distribution<float> location(-0.9f, 0.9f);
		std::uniform_real_distribution<float> range(0.03f, 0.05f);
		std::uniform_real_distribution<float> vel(-0.4f, 0.4f);

		for (size_t i = 0; i < 400; ++i)
		{
			auto entity = imp.create_entity();
			auto& re = imp.create_component<Location>(entity, location(engine), location(engine));
			imp.create_component<Collision>(entity, range(engine));
			imp.create_component<Velocity>(entity, vel(engine), vel(engine));
		}

		auto handle = LoadLibrary("dll_expand.dll");
		if (handle)
		{
			void (*init)(PO::ECS::Context*, std::mutex*) = (void(*)(PO::ECS::Context*, std::mutex*))GetProcAddress(handle, "init");
			init(&imp, &cout_mutex);
		}

		try {
			imp.loop();
		}
		catch (PO::ECS::Error::SystemOrderConflig& soc)
		{
			std::cout << "conflig : " << soc.si << " " << soc.ti << std::endl;
		}

		if (handle)
		{
			FreeLibrary(handle);
			handle = nullptr;
		}
	}
}