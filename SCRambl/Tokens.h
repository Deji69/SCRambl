//**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

namespace SCRambl
{
	namespace Tokens
	{
		/*\
		* Tokens::Token - Types of token information
		\*/
		enum class Type {
			None, Directive, Command, CommandOverload, Identifier, Label, Number, Operator, String, Character
		};

		/*\
		* Tokens::Symbol - Base for parsed token data
		\*/
		class Symbol
		{
		public:
			using Shared = std::shared_ptr < Symbol >;

			inline Type GetType() const		{ return m_Type; }

			template<typename  T>
			inline T& Extend()				{ return *this; }
			template<typename  T>
			inline const T& Extend() const	{ return *this; }

		protected:
			Symbol(Type type) : m_Type(type)
			{ }
			inline virtual ~Symbol()
			{ }

		private:
			Type				m_Type;
		};
	}
}