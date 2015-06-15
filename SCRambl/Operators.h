/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include "Configuration.h"
#include "Types.h"
#include "Numbers.h"
#include "Commands.h"
#include "Variables.h"

namespace SCRambl
{
	namespace Operators
	{
		// Safe(ish) references usable as values
		using OperatorRef = VecRef<class Operator>;
		using OperationRef = VecRef<class Operation>;
		
		/*\ Operators::Type - built-in operator types \*/
		class Type {
		public:
			enum ID {
				add,					// +
				sub,					// -
				mult,					// *
				div,					// /
				mod,					// %

				inc,					// ++
				dec,					// --

				bit_and,				// &
				bit_or,					// |
				bit_xor,				// ^
				bit_shl,				// <<
				bit_shr,				// >>
				bit_not,				// ~ (unary)

				comp_add,				// +=
				comp_sub,				// -=
				comp_mult,				// *=
				comp_div,				// /=
				comp_mod,				// %=
				comp_bit_and,			// &=
				comp_bit_or,			// |=
				comp_bit_xor,			// ^=
				comp_bit_shl,			// <<=
				comp_bit_shr,			// >>=
				uncomp_cast,			// =#

				//add_timed,				// +@
				//sub_timed,				// -@

				eq,						// ==
				neq,					// !=
				gt,						// >
				lt,						// <
				geq,					// >=
				leq,					// <=

				not,					// !
				and,					// &&
				or,						// ||
				cond,					// ?
				condel,					// :

				max_operator,
			};

			Type() = default;
			Type(ID id) : m_ID(id)
			{ }

			inline bool OK() const { return Get() != max_operator; }
			inline ID Get() const { return m_ID; }
			inline operator ID() const { return Get(); }
			inline explicit operator bool() const { return OK(); }

			bool operator==(const Type& v) const { return m_ID == v.m_ID; }
			bool operator==(ID v) const { return m_ID == v; }

		private:
			ID m_ID = max_operator;
		};

		/*\
		 - Operators::Table - the place where operators go
		 - Uses the Symbols vector of CodeLine to assign new operators, and retrieve them later
		\*/
		template<typename T>
		class Table
		{
		public:
			/*\
			- Operator::Table::Cell - If it doesnt store an operator or another non-useless cell, it's useless (recursive)
			- Get the first cell via the Operator::Table
			\*/
			class Cell {
				friend class Table;

				T m_Operator;
				std::vector<Cell> m_Cells;

			public:
				// For reservation of cells for any grapheme character - wastes a lot of space, but we can access things quickly :)
				void ReserveCells(size_t num_cells) {
					if (m_Cells.size() < num_cells)
						m_Cells.resize(num_cells);
				}

				// Obviously doesnt work if theres no way to get an operator by adding that grapheme, of course
				bool Next(Grapheme graph, const Cell *& next_out) const {
					// ensure we have a cell for this grapheme
					if (m_Cells.size() >= (unsigned)graph) {
						// give it the next cell
						next_out = &m_Cells[graph];
						return true;
					}
					return false;
				}

				// A problem shared...
				inline bool Next(Grapheme graph, Cell *& next_out) {
					return Next(graph, const_cast<const Cell*&>(next_out));
				}

				// So you think this Cell has an operator? Good luck...
				inline T GetOperator() const { return m_Operator; }
			};

		private:
			std::vector<Cell> m_Cells;

		public:
			Table() = default;
			Table(size_t num_cells) : m_Cells(num_cells)
			{ }

			/*\
			- Allocates cells for the path of graphemes leading to an operator
			- Adding <, << and <=, for example, would allocate one initial cell for '<' and then two cells for '<' and '='
			- Each final cell will have the operator type ID assigned to it, which can be retrieved once the cell is obtained
			\*/
			void AddOperator(CodeLine code, T op) {
				Cell* pCell = nullptr;
				bool got_first_cell = false;

				// use the grapheme from each symbol to walk down the row of cells
				for (auto c : code) {
					ASSERT(c->HasGrapheme() && "Only symbols with graphemes (specially recognized symbols) can be added as operators");
					if (!got_first_cell) {
						auto minsize = static_cast<unsigned>(c->GetGrapheme()) + 1;
						if (m_Cells.size() < minsize) m_Cells.resize(minsize);
						pCell = &m_Cells[c->GetGrapheme()];
						got_first_cell = true;
					}
					else {
						pCell->ReserveCells(c->GetGrapheme() + 1);
						pCell->Next(c->GetGrapheme(), pCell);
					}

				}

				ASSERT(got_first_cell && "Use a CodeLine that actually contains Symbols");

				if (got_first_cell) {
					// assign an operator to that cell
					pCell->m_Operator = op;
				}
			}

			/*\
			- Get the first cell of an operator string
			- e.g. if the grapheme was 'plus' (+), there could be two possible following cells: 'plus' (+) or 'equals' (=)
			- it all depends on which operators are in this table, of course
			\*/
			inline const Cell& GetCell(Grapheme graph) const {
				static const Cell default_cell;
				//ASSERT(m_Cells.size() >= (unsigned)graph && "Something went wrong internally - or an out of range grapheme ID was used");
				return m_Cells.size() >= (unsigned)graph ? m_Cells[graph] : default_cell;
			}
		};

		/* Operators::Scanner - Operator scanner for lexage */
		template<typename T>
		class Scanner : public Lexer::Scanner {
			using OperatorCell = typename Table<T>::Cell;
			Table<T>& m_Table;
			Scripts::Position m_LastOperatorPos;
			const OperatorCell* m_Cell;
			const OperatorCell* m_LastOperatorCell;

		public:
			Scanner(Table<T>& table) : m_Table(table)
			{}
			bool Scan(Lexer::State& state, Scripts::Position& pos) {
				switch (state)
				{
					// Check, check, check fo da cell dat sells
				case Lexer::State::before:
					if (!pos->HasGrapheme()) return false;
					m_Cell = &m_Table.GetCell(pos->GetGrapheme());
					if (m_Cell->GetOperator()) {
						m_LastOperatorCell = m_Cell;
						m_LastOperatorPos = pos;
					}
					else m_LastOperatorCell = nullptr;
					state = Lexer::State::inside;
					++pos;
					return true;

				case Lexer::State::inside:
					for (; pos && pos->HasGrapheme(); ++pos)
					{
						if (!m_Cell->Next(pos->GetGrapheme(), m_Cell)) break;
						if (m_Cell->GetOperator()) {
							m_LastOperatorCell = m_Cell;
							m_LastOperatorPos = pos;
						}
					}
					if (m_LastOperatorCell)
					{
						m_Cell = m_LastOperatorCell;
						pos = m_LastOperatorPos + 1;
						state = Lexer::State::after;
						return true;
					}
					return false;

				case Lexer::State::after:
					return true;
				}
				return false;
			}

			T GetOperator() const { ASSERT(m_Cell && "Can only get the operator after a succesful scan");  return m_Cell->GetOperator(); }
		};

		/*\ Operators::Operation \*/
		class Operation {
			friend class Operator;

			Operator* m_Operator;
			size_t m_Index;
			Types::Type* m_RHS = nullptr, * m_LHS = nullptr;
			bool m_HasLHV = false, m_HasRHV = false, m_Swapped = false;
			int m_LHV = 0, m_RHV = 0;

		public:
			Operation() = delete;
			Operation(Operator* op, size_t id, Types::Type* lhs, Types::Type* rhs = nullptr) : m_Operator(op), m_Index(id),
				m_LHS(lhs), m_RHS(rhs), m_HasLHV(false), m_HasRHV(false), m_Swapped(false)
			{ }

			Operator* GetOperator() const { return m_Operator; }
			size_t GetIndex() const { return m_Index; }
			Types::Type* GetLHS() const { return m_LHS; }
			Types::Type* GetRHS() const { return m_RHS; }
			bool HasLHS() const { return m_LHS != nullptr; }
			bool HasRHS() const { return m_RHS != nullptr; }
			bool HasLHV() const { return m_HasLHV; }
			bool HasRHV() const { return m_HasRHV; }
			int GetLHV() const { return m_LHV; }
			int GetRHV() const { return m_RHV; }
			void SetLHS(Types::Type* type) { m_LHS = type; }
			void SetRHS(Types::Type* type) { m_RHS = type; }
			void SetLHV(int v) {
				m_LHV = v;
				m_HasLHV = true;
			}
			void SetRHV(int v) {
				m_RHV = v;
				m_HasRHV = true;
			}
			void SetSwapped(bool v) { m_Swapped = v; }
		};

		/*\ Smooth Operator */
		class Operator
		{
			friend class Operators;

			std::string m_Op;
			std::vector<Operation> m_Operations;
			bool m_IsConditional = false;

		public:
			Operator(std::string op, bool iscond) : m_Op(op), m_IsConditional(iscond)
			{ }
			Operator(const Operator& v) : m_Op(v.m_Op), m_IsConditional(v.m_IsConditional) {
				for (auto& op : v.m_Operations) {
					m_Operations.emplace_back(op);
					m_Operations.back().m_Operator = this;
				}
			}

			bool IsConditional() const { return m_IsConditional; }

			Operation& AddOperation(size_t id, Types::Type* lhs, Types::Type* rhs) {
				m_Operations.emplace_back(this, id, lhs, rhs);
				return m_Operations.back();
			}
			size_t GetNumOperations() const {
				return m_Operations.size();
			}
			Operation* GetOperation(size_t idx) {
				return idx < m_Operations.size() ? &m_Operations[idx] : nullptr;
			}
		
			Operation* GetUnaryOperation(Variable*, bool rhs_var = false);
			Operation* GetOperation(Variable*, Types::Type*);
		};
		
		const Type max_operator = Type::max_operator;
		using OperatorTable = Table<OperatorRef>;

		/*\	Operators - storage, config & utility \*/
		class Operators {
		public:
			Operators();
			void Init(Build&);
			template<typename... TArgs>
			OperatorRef Insert(TArgs&&... args) {
				m_Storage.emplace_back(args...);
				return OperatorRef(m_Storage, m_Storage.size() - 1);
			}
			OperatorRef Add(std::string op, bool is_conditional = false) {
				auto ref = Insert(op, is_conditional);
				if (ref) {
					m_OpMap.emplace(op, ref);
					m_Table.AddOperator(op, ref);
				}
				return ref;
			}
			void Add(std::string op, OperatorRef ref) {
				if (ref) m_OpMap.emplace(op, ref);
			}
			OperatorRef Get(std::string op) {
				static const OperatorRef def;
				auto it = m_OpMap.find(op);
				return it != m_OpMap.end() ? it->second : def;
			}
			OperatorTable& GetTable() { return m_Table; }
			const OperatorTable& GetTable() const { return m_Table; }
			size_t Size() const { return m_Storage.size(); }

		private:
			static const OperatorRef s_InvalidOperatorRef;

			XMLConfiguration* m_Config;
			std::vector<Operator> m_Storage;
			std::unordered_map<std::string, OperatorRef> m_OpMap;
			OperatorTable m_Table;
		};
	}
}