//**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
namespace Basic { class Error; };
#include "Preprocessor.h"
#include "Parser.h"

namespace SCRambl
{
	namespace Basic
	{
		/*\
		 - Basic::Error - wrapper for varying error types
		\*/
		class Error
		{
		public:
			enum Type { preprocessor, parser, compiler, linker };
			
		private:
			class Payload
			{
			protected:
				inline virtual ~Payload() { }
			};

			template<class T>
			class Info : public Payload
			{
				T			m_Error;

			public:
				Info(const T & err) : m_Error(err)
				{ }
				inline T & Get()					{ return m_Error; }
				inline const T & Get() const		{ return m_Error; }
			};

			Type						m_Type;
			std::shared_ptr<Payload>	m_Payload;

		public:
			Error(const Preprocessor::Error & err): m_Type(preprocessor),
				m_Payload(std::make_unique<Info<Preprocessor::Error>>(err))
			{ }
			Error(const Parser::Error & err) : m_Type(parser),
				m_Payload(std::make_unique<Info<Parser::Error>>(err))
			{ }
			//Error(const Error &) = delete;

			inline Type GetType() const			{ return m_Type; }
			
			template<typename T>
			inline T & Get();
			template<typename T>
			inline const T & Get() const;

			template<> inline Preprocessor::Error & Get<Preprocessor::Error>()	{
				ASSERT(m_Type == preprocessor);
				return static_cast<Info<Preprocessor::Error>&>(*m_Payload).Get();
			}
			template<> inline const Preprocessor::Error & Get<Preprocessor::Error>() const	{
				return Get<Preprocessor::Error>();
			}

			template<> inline Parser::Error & Get<Parser::Error>()	{
				ASSERT(m_Type == parser);
				return static_cast<Info<Parser::Error>&>(*m_Payload).Get();
			}
			template<> inline const Parser::Error & Get<Parser::Error>() const	{
				return Get<Parser::Error>();
			}
		};
	}
}