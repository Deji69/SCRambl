//**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "TokenInfo.h"

namespace SCRambl
{
	namespace Tokens
	{
		template<typename T>
		using Shared = std::shared_ptr < T >;

		/*\
		* Tokens::Token - Types of token information
		\*/
		enum class Type : char {
			None, Directive,
			Command, CommandDecl, CommandCall, CommandOverload, Variable,
			Identifier, Label, LabelRef, Number, Operator, String, Character, Delimiter
		};

		/*\
		 * Tokens::Symbol - Base for parsed token data
		\*/
		class Symbol
		{
		public:
			Symbol(Type type) : m_Type(type)
			{ }
			inline virtual ~Symbol()
			{ }

			inline Type GetType() const		{ return m_Type; }

			template<typename  T>
			inline T& Extend()				{ return static_cast<T&>(*this); }
			template<typename  T>
			inline const T& Extend() const	{ return static_cast<T&>(*this); }

		private:
			Type				m_Type;
		};
	}
}