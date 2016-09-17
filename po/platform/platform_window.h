#pragma once
#include <Unknwnbase.h>
#include <windows.h>
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include "platform_event.h"
namespace PO
{
	namespace Platform
	{

		namespace Error
		{
			struct win32_init_error :std::exception
			{
				std::string scription;
				const char* what() const { return scription.c_str(); }
				win32_init_error(const std::string& scr) noexcept;
				win32_init_error(const win32_init_error&) = default;
				win32_init_error(win32_init_error&&) = default;
			};
		}

		struct window_style
		{
			std::u16string title;
			long window_shift_x;
			long window_shift_y;
			long window_width;
			long window_height;
			DWORD ex_style;
			DWORD style;
			window_style();
			window_style(const window_style&) = default;
			window_style(window_style&&) = default;
			window_style& operator=(const window_style&) = default;
			window_style& operator=(window_style&&) = default;
		};

		class window_instance
		{
			HWND win;
			std::atomic_bool window_exist;// = true;

		public:
			HWND get_index() const { return win; }
			void close_window() { window_exist = false; }

			window_instance();
			window_instance(const window_style&);

			virtual bool respond_event(const window_event&) { return true; };

			virtual ~window_instance();
			bool is_exist() const { return window_exist; }
			void one_frame_loop();
			LRESULT main_event_respond(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		};

		


		/*
		class app_context
		{
			std::mutex window_mutex;
			std::unordered_map < window_instance::index, std::shared_ptr<window_instance> > window_map;
			std::unordered_map<std::thread::id, std::thread> thread_map;
		public:
			//std::this_thread::
			app_context();
			~app_context();
			template<typename T,typename ...AT>
			std::shared_ptr<window<T>> create_window(const window_style& ws,AT&&... at)
			{
				 
			}
			template<typename T, typename ...AT>
			std::shared_ptr<window<T>> create_window(AT&&... at)
			{

			}
		};
		*/
	}
}


/*
namespace Platform
{

	template<typename T, size_t i> class base_image_no_destructor_
	{
		T img;
		bool inside_resource;
		virtual void clear() = 0;
	protected:
		bool need_destructor_() const {return !inside_resource && img != NULL; }
	public:
		operator T () { return img; }
		operator bool() const { return img != NULL; }
		explicit base_image_no_destructor_(int t) : img((T)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(t), i, 0, 0, LR_SHARED)), inside_resource(true) {}
		base_image_no_destructor_(const std::u16string& t) :img((HICON)LoadImage(NULL, (wchar_t*)t.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE)), inside_resource(false) {}
		base_image_no_destructor_(base_image_no_destructor_&& bi) :img(bi.img), inside_resource(bi.inside_resource) { bi.inside_resource = false; img = NULL; }
		base_image_no_destructor_() :img(NULL), inside_resource(false) { }
		base_image_no_destructor_(const base_image_no_destructor_&) = delete;
		base_image_no_destructor_(const char16_t* ty) :img((T)LoadImage(NULL, (wchar_t*)ty, i, 0, 0, LR_LOADFROMFILE)), inside_resource(true) {}
		void operator=(base_image_no_destructor_&& bi) { clear(); base_image_no_destructor_ tem(std::move(bi)); img = tem.img; inside_resource = tem.inside_resource; tem.img = NULL; tem.inside_resource = false; }
		virtual ~base_image_no_destructor_() {};
	};

	class icon_struct : public base_image_no_destructor_<HICON, IMAGE_ICON>
	{
		using base_image_no_destructor_<HICON, IMAGE_ICON>::base_image_no_destructor_;
		void clear() override { if( need_destructor_() ) DestroyIcon(static_cast<HICON>(*this)); }
	public:
		~icon_struct() { clear(); }
	};

	class cursor_struct : public base_image_no_destructor_<HCURSOR, IMAGE_CURSOR>
	{
		void clear() override { DestroyCursor(static_cast<HCURSOR>(*this)); }
		using base_image_no_destructor_<HCURSOR, IMAGE_CURSOR>::base_image_no_destructor_;
	public:
		~cursor_struct() { clear(); }
	};

	using icon_ptr = std::shared_ptr<icon_struct>;
	using cursor_ptr = std::shared_ptr<icon_struct>;

	template<typename T> auto icon(T&& t) { return std::make_shared<icon_struct>(std::forward<T>(t)); }
	template<typename T> auto cursor(T&& t) { return std::make_shared<cursor_struct>(std::forward<T>(t)); }

	bool is_app_still_running();
	void close_app();

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	
	inline int screen_width() { return GetSystemMetrics(SM_CXSCREEN); }
	inline int screen_height() { return GetSystemMetrics(SM_CYSCREEN); }

	
	class app_context
	{
		WNDCLASSEX wc;
		ATOM index;
		const std::u16string name;
		friend class window;
	public:
		template<typename T> app_context(const std::u16string& ui, T&& t) : app_context(ui.c_str(), std::forward<T>(t)) {}
		template<typename T> app_context(const char16_t* str, T&& t):wc({ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW , WndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, (const wchar_t*)(str), NULL }),name(str)
		{
			t(wc);
			index = RegisterClassEx(&wc);
		}
		app_context(const std::u16string& ui) : app_context(ui.c_str()) {}
		app_context(const char16_t* str) : wc({ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW , WndProc, 0, 0, GetModuleHandle(0), NULL,NULL, 0, NULL, (const wchar_t*)(str), NULL }), name(str)
		{
			index = RegisterClassEx(&wc);
		}
		~app_context() { UnregisterClass((const wchar_t*)(name.c_str()), GetModuleHandle(0)); }
	};


	struct style
	{
		const DWORD ex_style;
		const DWORD nor_style;

		static const style default_;
		static const style full_window;
	};

	struct area { int left; int top; int width; int height; area(int l = 0, int t = 0, int w = 0, int h = 0) :left(l), top(t), width(w), height(h) {} };

	class window
	{
		HWND win;
	public:
		operator bool() const { return win != NULL; }
		operator HWND() { return win; }
		window(app_context& appc, const char16_t* str, area ar, style ws);
		window(app_context& appc, const char16_t* str, area ar, style ws, PO::Tool::auto_adapter<bool(area&, style&)> fun);
		~window() { if( win!=NULL ) DestroyWindow(win); }
		LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};

	struct app_context
	{
		static const ATOM index;
	};



}*/

std::ostream& operator<<(std::ostream& os, const std::u16string& st) noexcept;
std::ostream& operator<<(std::ostream& os, const char16_t* st) noexcept;