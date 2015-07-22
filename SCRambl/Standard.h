//**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <memory>

namespace SCRambl {
	namespace Basic {
		// Wrapper for varying error types
		class Error {
		public:
			Error(const Error&) = delete;
			Error(Error&& o) : m_Payload(std::move(o.m_Payload))
			{ }
			Error& operator=(Error&& o) {
				if (this != &o) m_Payload = std::move(o.m_Payload);
				return *this;
			}
			Error& operator=(const Error&) = delete;
			template<typename T>
			Error(const T& err) : m_Payload(std::make_unique<Info<T>>(err))
			{ }
			
			template<typename T>
			inline T& Get() {
				return static_cast<Info<T>&>(*m_Payload).Get();
			}
			template<typename T>
			inline const T& Get() const {
				return Get<T>();
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

			std::unique_ptr<Payload> m_Payload;
		};
	}
}