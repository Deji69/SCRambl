//**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Preprocessor.h"

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
			public:
				virtual ~Payload() { }
			};

			class PreprocessorError : public Payload
			{
				Preprocessor::Error		m_Error;

			public:
				PreprocessorError(Preprocessor::Error err) : m_Error(err)
				{ }

				Preprocessor::Error & Get()					{ return m_Error; }
				const Preprocessor::Error & Get() const		{ return m_Error; }
			};

			Type						m_Type;
			std::shared_ptr<Payload>	m_Payload;

		public:
			Error(const Preprocessor::Error & err): m_Type(preprocessor),
				m_Payload(std::make_unique<PreprocessorError>(err))
			{
			}
			//Error(const Error &) = delete;

			inline Type GetType() const			{ return m_Type; }
			
			template<typename T>
			inline T & Get();
			template<typename T>
			inline const T & Get() const;

			template<> inline Preprocessor::Error & Get<Preprocessor::Error>()	{
				ASSERT(m_Type == preprocessor);
				return static_cast<PreprocessorError&>(*m_Payload).Get();
			}
			template<> inline const Preprocessor::Error & Get<Preprocessor::Error>() const	{
				ASSERT(m_Type == preprocessor);
				return static_cast<PreprocessorError&>(*m_Payload).Get();
			}
		};
	}
}