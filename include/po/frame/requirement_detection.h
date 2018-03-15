#pragma once
#include "../tool/tmp.h"
namespace PO::ECSFramework
{

	class context;

	namespace Implement
	{
		// system requirement
		template<typename type> struct is_readed_type :std::true_type {};
		template<typename type> struct is_readed_type<const type&> :std::true_type {};
		template<typename type> struct is_readed_type<const type&&> :std::true_type {};
		template<typename type> struct is_readed_type<type&> : std::false_type {};
		template<typename type> struct is_readed_type<type&&> : std::false_type {};

		template<typename ...component_t> struct ctl
		{
			template<template<typename ...> class out> using outpacket = out<component_t...>;
			constexpr static const size_t size = sizeof...(component_t);
		};

		template<typename T1, typename T2> struct link;
		template<typename ...T1, typename ...T2> struct link<ctl<T1...>, ctl<T2...>>
		{
			using type = ctl<T1..., T2...>;
		};

		template<typename T1, typename T2> using link_t = typename link<T1, T2>::type;

		template<typename read_resault, typename write_resault, typename ...input> struct collect_read_write_implement;
		template<typename ...read_resault, typename ...write_reault, typename this_input, typename ...input> 
		struct collect_read_write_implement<ctl<read_resault...>, ctl<write_reault...>, this_input, input...>
			: collect_read_write_implement<
				std::conditional_t<is_readed_type<this_input>::value && !Tmp::is_one_of<std::decay_t<this_input>, read_resault...>::value, ctl<read_resault..., std::decay_t<this_input>>, ctl<read_resault...>>,
				std::conditional_t<!is_readed_type<this_input>::value && !Tmp::is_one_of<std::decay_t<this_input>, write_reault...>::value, ctl<write_reault..., std::decay_t<this_input>>, ctl<write_reault...>>,
				input...
			>{};

		template<typename ...read_resault, typename ...write_reault>
		struct collect_read_write_implement<ctl<read_resault...>, ctl<write_reault...>>
		{
			using read = ctl<read_resault...>;
			using write = ctl<write_reault...>;
		};

		template<typename ...input> struct collect_read_write : collect_read_write_implement<ctl<>, ctl<>, input...> {};
		

		// singleton type
		template<typename singleton_type, typename ...component_type> struct system_requirement_analyzer_singleton;
		template<typename ...singleton_type, typename this_component_type, typename ...component_type>
		struct system_requirement_analyzer_singleton<ctl<singleton_type...>, this_component_type, component_type...>
			:system_requirement_analyzer_singleton<
			ctl<singleton_type..., this_component_type>,
			component_type...
			>
		{
			static_assert(!std::is_same_v<std::decay_t<this_component_type>, context>, "context should be at first");
			static_assert(!std::is_same_v<std::decay_t<this_component_type>, other_entity>, "singleton should be at last");
			static_assert(!Tmp::is_one_of<std::decay_t<this_component_type>, std::decay_t<singleton_type>...>::value, "system requirement form same entity or singleton can not repeat");
		};

		template<typename ...singleton_type>
		struct system_requirement_analyzer_singleton<ctl<singleton_type...>>
		{
			using singleton = ctl<singleton_type...>;
		};


		// second type
		template<typename second_type, typename ...component_type> struct system_requirement_analyzer_second;
		template<typename ...second_type, typename this_component_type, typename ...component_type>
		struct system_requirement_analyzer_second<ctl<second_type...>, this_component_type, component_type...> :
			system_requirement_analyzer_second<
			ctl<second_type..., this_component_type>, component_type... >
		{
			static_assert(!std::is_same_v<std::decay_t<this_component_type>, context>, "context should be at first");
			static_assert(!std::is_same_v<std::decay_t<this_component_type>, other_entity>, "only support 2 entity");
			static_assert(!Tmp::is_one_of<std::decay_t<this_component_type>, std::decay_t<second_type>...>::value, "system requirement form same entity or singleton can not repeat");
		};

		template< typename ...second_type, typename ...component_type>
		struct system_requirement_analyzer_second<ctl<second_type...>, singleton, component_type...> :
			system_requirement_analyzer_singleton<ctl<>, component_type...>
		{
			using second = ctl<second_type...>;
			constexpr static const bool have_singleton = true;
		};

		template<typename ...second_type>
		struct system_requirement_analyzer_second<ctl<second_type...>> :
			system_requirement_analyzer_singleton<ctl<>>
		{
			using second = ctl<second_type...>;
			constexpr static const bool have_singleton = false;
		};

		// first type
		template<typename first_type, typename ...component_type> struct system_requirement_analyzer_first;
		template<typename ...first_type, typename this_component_type, typename ...component_type>
		struct system_requirement_analyzer_first< ctl<first_type...>, this_component_type, component_type...> :
			system_requirement_analyzer_first<
			ctl<first_type..., this_component_type>, component_type...>
		{
			static_assert(!std::is_same_v<std::decay_t<this_component_type>, context>, "context should be at first");
			static_assert(!Tmp::is_one_of<std::decay_t<this_component_type>, std::decay_t<first_type>...>::value, "system requirement form same entity or singleton can not repeat");
		};

		template<typename ...first_type>
		struct system_requirement_analyzer_first<ctl<first_type...>> :
			system_requirement_analyzer_second<ctl<>>
		{
			using first = ctl<first_type...>;
			constexpr static const bool have_other_entity = false;
		};

		template< typename ...first_type, typename ...component_type>
		struct system_requirement_analyzer_first<ctl<first_type...>, other_entity, component_type...> :
			system_requirement_analyzer_second<ctl<>, component_type...>
		{
			using first = ctl<first_type...>;
			constexpr static const bool have_other_entity = true;
		};

		template<typename ...first_type, typename ...component_type>
		struct system_requirement_analyzer_first<ctl<first_type...>, singleton, component_type...> :
			system_requirement_analyzer_second<ctl<>, singleton, component_type...>
		{
			using first = ctl<first_type...>;
			constexpr static const bool have_other_entity = false;
		};

		// context
		template<typename ...component_type> struct system_requirement_analyzer_context : system_requirement_analyzer_first<ctl<>, component_type...>
		{
			constexpr static const bool need_context = false;
		};
		template<typename ...component_type> struct system_requirement_analyzer_context<context&, component_type...> : system_requirement_analyzer_first<ctl<>, component_type...>
		{
			constexpr static const bool need_context = true;
		};
		template<typename ...component_type> struct system_requirement_analyzer_context<const context&, component_type...> : system_requirement_analyzer_first<ctl<>, component_type...>
		{
			constexpr static const bool need_context = true;
		};

		// analyzer
		template<typename ...component_type>
		struct system_requirement_analyzer : system_requirement_analyzer_context<component_type...> {};

		template<typename system_type, typename = void> struct system_trigger_define_detector_implement : std::false_type { using type = void; };
		template<typename system_type> struct system_trigger_define_detector_implement<system_type, std::void_t<typename system_type::trigger_type>> : std::true_type { using type = typename system_type::trigger_type; };

	}
}