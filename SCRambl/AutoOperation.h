/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <string>
#include <unordered_map>
#include "Types.h"
#include "Operators.h"

namespace SCRambl
{
	template<typename T>
	static T auto_add(T a, T b) { return a + b; }
	template<typename T>
	static T auto_sub(T a, T b) { return a - b; }
	template<typename T>
	static T auto_mul(T a, T b) { return a * b; }
	template<typename T>
	static T auto_div(T a, T b) { return a / b; }
	template<typename T = int>
	static T auto_mod(T a, T b) { return a % b; }
	template<typename T>
	static T auto_neg(T a, T b) { return -a; }
	template<typename T>
	static bool auto_cmplt(T a, T b) { return a < b; }
	template<typename T>
	static bool auto_cmpgt(T a, T b) { return a > b; }
	template<typename T>
	static bool auto_cmplte(T a, T b) { return a <= b; }
	template<typename T>
	static bool auto_cmpgte(T a, T b) { return a >= b; }
	template<typename T>
	static bool auto_cmpeq(T a, T b) { return a == b; }
	template<typename T>
	static bool auto_cmpneq(T a, T b) { return a != b; }
	template<typename T>
	static bool auto_cmpnn(T a, T b = 0) { return static_cast<bool>(a); }
	template<typename T>
	static bool auto_cmpn(T a, T b = 0) { return !static_cast<bool>(a); }
	template<typename T1, typename T2>
	static T1 auto_cast(T2 v, T2 b = 0) { return static_cast<T1>(v); }

	class AutoOperation {
	public:
		enum Type {
			add, sub, mul, div, mod, neg,
			cmplt, cmpgt, cmplte, cmpgte, cmpeq, cmpneq, cmpn, cmpnn,
			cast,
		};

	public:
		AutoOperation(Type type, VecRef<Types::Type> ltype, VecRef<Types::Type> rtype = nullptr, Operators::OperationRef operation = nullptr) {
			m_Type = type;
			m_LHS = ltype;
			m_RHS = rtype;
			m_Operation = operation;
		}

		inline VecRef<Types::Type> GetLHS() const { return m_LHS; }
		inline VecRef<Types::Type> GetRHS() const { return m_RHS; }

	private:
		Type m_Type;
		VecRef<Types::Type> m_LHS, m_RHS;
		Operators::OperationRef m_Operation;
	};

	class AutoOperations {
	public:
		template<typename TAutoOp>
		AutoOperation& AddOperation(std::string name, TAutoOp op) {
			auto pr = m_Operations.emplace(name, op);
			return pr.first.second;
		}

		AutoOperation* GetOperation(std::string name) {
			auto it = m_Operations.find(name);
			return it != m_Operations.end() ? &it->second : nullptr;
		}
		
	private:
		std::unordered_map<std::string, AutoOperation> m_Operations;
	};

	/*class AutoOperation {
	public:
		AutoOperation(std::string name) : m_Name(name) {
		}

	private:
		std::string m_Name;
	};*/
}