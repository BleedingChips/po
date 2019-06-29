#include "path.h"
#include "tool.h"
#include <assert.h>
namespace
{
	constexpr wchar_t sperator = L'\\';
	bool is_sperator(wchar_t input) { return input == L'/' || input == L'\\'; }
	bool is_colon(wchar_t input) { return input == L':'; }
	bool is_point(wchar_t input) { return input == L'.'; }

	enum class PathState
	{
		LinuxRoot,
		Root,
		Current,
		Upper,
		Name,
		MappingName,
	};

	struct path_state_machine
	{
		enum class State
		{
			Start,
			RootCurrent,
			RootUpper,
			FileName,
			RootMapping,
			RootDone,
		};

		PathState state(size_t index) {
			PO::Tool::scope_guard se{ [this]() noexcept {m_state = State::Start; } };
			switch (m_state)
			{
			case State::Start:
				return PathState::LinuxRoot;
			case State::FileName:
				return PathState::Name;
			case State::RootCurrent:
				return PathState::Current;
			case State::RootMapping:
				return PathState::MappingName;
			case State::RootUpper:
				return PathState::Upper;
			default:
				assert(false);
				return PathState::LinuxRoot;
			}
		}
		void handle(wchar_t input, size_t index)
		{
			switch (m_state)
			{
			case State::RootDone: break;
			case State::Start:
				if (input == L'.') { m_state = State::RootCurrent; break; }
				else { m_state = State::FileName; break; }
				break;
			case State::RootCurrent:
				if (input == L'.') { m_state = State::RootUpper; }
				else m_state = State::FileName;
				break;
			case State::RootUpper:
				m_state = State::FileName;
				break;
			case State::FileName:
				if (input == L':') { m_state = State::RootMapping; }
				else m_state = State::FileName;
				break;
			case State::RootMapping:
				m_state = State::FileName;
				break;
			default:
				assert(false);
				break;
			}
		}
	private:
		State m_state = State::Start;
	};

	std::tuple<std::wstring, std::vector<std::tuple<std::wstring::iterator, PathState>>> 
		simplify(std::wstring path)
	{
		bool sperator_reach = false;
		auto ite_avalible = path.begin();
		auto ite_current = ite_avalible;
		auto ite_end = path.end();
		std::vector<std::tuple<std::wstring::iterator, PathState>> block_list;
		path_state_machine state_machine;
		size_t index = 0;
		while (ite_current != ite_end)
		{
			bool is_sperator = (*ite_current == L'/' || *ite_current == L'\\');
			if (is_sperator)
			{
				if (sperator_reach)
				{
					++ite_current;
					continue;
				}
				else {
					sperator_reach = true;
					auto state = state_machine.state(index);
					if (
						state == PathState::Upper && !block_list.empty()
						&& std::get<1>(*block_list.rbegin()) == PathState::Name
						)
					{
						if (block_list.size() == 1)
							ite_avalible = path.begin();
						else
							ite_avalible = std::get<0>(*(++block_list.rbegin()));
						block_list.pop_back();
					}
					else {
						block_list.push_back({ ite_avalible , state });
					}
				}
			}
			else {
				sperator_reach = false;
				state_machine.handle(*ite_current, index);
			}
			if (ite_avalible != ite_current)
				*ite_avalible = *ite_current;
			++ite_avalible;
			++ite_current;
			index++;
		}
		path.erase(ite_avalible, path.end());
		return {std::move(path), std::move(block_list)};
	}

}

namespace PO::Path
{

	root_mapping& root_mapping::gobal()
	{
		static root_mapping mapping;
		return mapping;
	}

	path::path(std::wstring path) : m_path(std::move(path)) {}

	path path::operator/(const path& input) const
	{
		if (
			!m_path.empty() && is_sperator(*m_path.rbegin())
			|| !input.m_path.empty() && is_sperator(*input.m_path.rbegin())
			|| input.m_path.empty()
			)
			return m_path + input.m_path;
		else{
			std::wstring temporary;
			temporary.reserve(m_path.size() + input.m_path.size() + 1);
			((temporary += m_path) += sperator) += input.m_path;
			return std::move(temporary);
		}
	}

	path& path::operator/=(const path& input)
	{
		std::wstring temporary;
		if (
			!m_path.empty() && is_sperator(*m_path.rbegin())
			|| !input.m_path.empty() && is_sperator(*input.m_path.rbegin())
			|| input.m_path.empty()
			)
		{
			m_path += input.m_path;
			return *this;
		}
		else {
			m_path.reserve(m_path.size() + input.m_path.size() + 1);
			(m_path += L'/') += input.m_path;
			return *this;
		}
	}
}