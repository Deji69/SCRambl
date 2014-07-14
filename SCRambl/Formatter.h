/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>

namespace SCRambl
{
	template<typename> class Formatter;

	class IFormatter
	{
	public:
		virtual ~IFormatter() { };

		template<typename T>
		inline Formatter<T> & Qualify() {
			ASSERT(&typeid(Formatter<T>) == &typeid(*this));
			return *static_cast<Formatter<T>*>(this);
		}
	};

	// Format stuff - std::function wrapper
	template<typename T>
	class Formatter : public IFormatter
	{
	public:
		typedef std::function<std::string(T)> FunctionType;

	private:
		FunctionType		m_Function;
		FunctionType		m_Overriden;

	public:
		Formatter() = default;
		template<typename F>
		Formatter(F & func) : m_Function(func)
		{ }
		virtual ~Formatter()		{ }

		inline void Override()							{ (m_Function ? m_Function : m_Overridden) = nullptr; }
		inline void Override(FunctionType & func)		{ (m_Function ? m_Function : m_Overridden) = func; }

		inline std::string operator()(const T& param) const		{ return m_Overriden ? m_Overriden(param) : m_Function(param); }
	};
}