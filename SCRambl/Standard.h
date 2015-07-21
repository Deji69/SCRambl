//**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Preprocessor.h"
#include "Parser.h"
#include "Compiler.h"

namespace SCRambl {
	namespace Basic {
		// Wrapper for varying error types
		class Error
		{
		public:
			enum Type { preprocessor, parser, compiler, linker };

			Error(const Error&) = delete;
			Error(Error&&);
			Error& operator=(Error&&);
			Error& operator=(const Error&) = delete;
			Error(const Preprocessor::Error&);
			Error(const Parser::Error&);

			inline Type GetType() const { return m_Type; }
			
			template<typename T>
			inline T& Get();
			template<typename T>
			inline const T& Get() const;

			template<> inline Preprocessor::Error& Get<Preprocessor::Error>()	{
				ASSERT(m_Type == preprocessor);
				return static_cast<Info<Preprocessor::Error>&>(*m_Payload).Get();
			}
			template<> inline const Preprocessor::Error& Get<Preprocessor::Error>() const	{
				return Get<Preprocessor::Error>();
			}
			template<> inline Parser::Error& Get<Parser::Error>()	{
				ASSERT(m_Type == parser);
				return static_cast<Info<Parser::Error>&>(*m_Payload).Get();
			}
			template<> inline const Parser::Error& Get<Parser::Error>() const	{
				return Get<Parser::Error>();
			}
			
		private:
			class Payload {
			public:
				inline virtual ~Payload() { }
			};
			template<class T>
			class Info : public Payload {
				T m_Error;

			public:
				Info(const T& err) : m_Error(err)
				{ }
				inline T& Get() { return m_Error; }
				inline const T& Get() const { return m_Error; }
			};

			Type m_Type;
			std::unique_ptr<Payload> m_Payload;
		};
	}
}