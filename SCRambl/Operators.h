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
		
		// Built-in operator types
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
		class Table {
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

		// Operator scanner for lexage
		template<typename T>
		class Scanner : public Lexing::Scanner {
			using OperatorCell = typename Table<T>::Cell;
			Table<T>& m_Table;
			Scripts::Position m_LastOperatorPos;
			const OperatorCell* m_Cell;
			const OperatorCell* m_LastOperatorCell;

		public:
			Scanner(Table<T>& table) : m_Table(table)
			{}
			bool Scan(Lexing::State& state, Scripts::Position& pos) {
				switch (state)
				{
					// Check, check, check fo da cell dat sells
				case Lexing::State::before:
					if (!pos->HasGrapheme()) return false;
					m_Cell = &m_Table.GetCell(pos->GetGrapheme());
					if (m_Cell->GetOperator()) {
						m_LastOperatorCell = m_Cell;
						m_LastOperatorPos = pos;
					}
					else m_LastOperatorCell = nullptr;
					state = Lexing::State::inside;
					++pos;
					return true;

				case Lexing::State::inside:
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
						state = Lexing::State::after;
						return true;
					}
					return false;

				case Lexing::State::after:
					return true;
				}
				return false;
			}

			T GetOperator() const { ASSERT(m_Cell && "Can only get the operator after a succesful scan");  return m_Cell->GetOperator(); }
		};

		// Operation
		class Operation {
			friend class Operator;

			OperatorRef m_Operator;
			OperationRef m_Ref;
			size_t m_Index;
			VecRef<Types::Type> m_RHS, m_LHS;
			bool m_HasLHV = false, m_HasRHV = false, m_Swapped = false, m_IsAuto = false;
			long m_LHV = 0, m_RHV = 0;

		public:
			using Attributes = Attributes<Types::DataAttributeID, Types::DataAttributeSet>;

			Operation() = delete;
			Operation(OperationRef ref, OperatorRef op, size_t id, VecRef<Types::Type> lhs, VecRef<Types::Type> rhs = {}) : m_Ref(ref), m_Operator(op),
				m_Index(id), m_LHS(lhs), m_RHS(rhs), m_HasLHV(false), m_HasRHV(false), m_Swapped(false)
			{ }

			Attributes GetAttributes() const;

			OperationRef GetRef() const { return m_Ref; }
			OperatorRef GetOperator() const { return m_Operator; }
			size_t GetIndex() const { return m_Index; }
			Types::Type* LHS() const { return m_LHS.Ptr(); }
			Types::Type* RHS() const { return m_RHS.Ptr(); }
			bool IsAuto() const { return m_IsAuto; }
			bool HasLHS() const { return LHS() != nullptr; }
			bool HasRHS() const { return RHS() != nullptr; }
			bool HasLHV() const { return m_HasLHV; }
			bool HasRHV() const { return m_HasRHV; }
			long GetLHV() const { return m_LHV; }
			long GetRHV() const { return m_RHV; }
			void SetLHS(VecRef<Types::Type> type) { m_LHS = type; }
			void SetRHS(VecRef<Types::Type> type) { m_RHS = type; }
			void SetLHV(long v) {
				m_LHV = v;
				m_HasLHV = true;
			}
			void SetRHV(long v) {
				m_RHV = v;
				m_HasRHV = true;
			}
			void SetSwapped(bool v) { m_Swapped = v; }
		};

		// Smooth Operator
		class Operator {
			friend class Master;

			enum class Sign {
				none, negative, positive
			};

		public:
			Operator(OperatorRef ref, std::string op, VecRef<Types::Type> type, bool iscond, bool isass = false, Sign sign = Sign::none) : m_Ref(ref), m_Op(op),
				m_Type(type), m_IsConditional(iscond), m_IsAssignment(isass), m_Sign(sign)
			{ }
			Operator(const Operator& v) : m_Op(v.m_Op), m_Type(v.m_Type), m_IsConditional(v.m_IsConditional), m_IsAssignment(v.m_IsAssignment), m_Sign(v.m_Sign),
				m_Ref(v.m_Ref)
			{
				for (auto& pr : v.m_Operations) {
					m_Operations.emplace_back(pr);
					m_Operations.back().m_Operator = m_Ref;
					m_Operations.back().m_Ref = { m_Operations, m_Operations.size() - 1 };
				}
				for (auto& pr : v.m_Autos) {
					m_Autos.emplace_back(pr);
					m_Autos.back().m_Operator = m_Ref;
					m_Autos.back().m_Ref = { m_Autos, m_Autos.size() - 1 };
				}
			}
			Operator(Operator&& v) : m_Op(v.m_Op), m_Type(std::move(v.m_Type)), m_IsConditional(v.m_IsConditional), m_IsAssignment(v.m_IsAssignment), m_Sign(v.m_Sign),
				m_Ref(std::move(v.m_Ref))
			{
				for (auto& pr : v.m_Operations) {
					m_Operations.emplace_back(pr);
					m_Operations.back().m_Operator = m_Ref;
					m_Operations.back().m_Ref = { m_Operations, m_Operations.size() - 1 };
				}
				for (auto& pr : v.m_Autos) {
					m_Autos.emplace_back(pr);
					m_Autos.back().m_Operator = m_Ref;
					m_Autos.back().m_Ref = { m_Autos, m_Autos.size() - 1 };
				}
				v.m_Operations.clear();
				v.m_Autos.clear();
			}

			OperatorRef GetRef() { return m_Ref; }

			const std::string& Name() const { return m_Op; }
			VecRef<Types::Type> Type() const { return m_Type; }
			bool IsAssignment() const { return m_IsAssignment; }
			bool IsConditional() const { return m_IsConditional; }
			bool HasAuto() const { return !m_Autos.empty(); }
			bool IsSign() const { return m_Sign != Sign::none; }
			bool IsNegative() const { return m_Sign == Sign::negative; }
			bool IsPositive() const { return m_Sign == Sign::positive; }

			OperationRef AddAuto(VecRef<Types::Type> lhs, VecRef<Types::Type> rhs, size_t id = -1) {
				m_Autos.emplace_back(OperationRef(m_Autos), m_Ref, id, lhs, rhs);
				m_Autos.back().m_IsAuto = true;
				return m_Autos.back().GetRef();
			}
			OperationRef AddOperation(size_t id, VecRef<Types::Type> lhs, VecRef<Types::Type> rhs) {
				m_Operations.emplace_back(OperationRef(m_Operations), m_Ref, id, lhs, rhs);
				return m_Operations.back().GetRef();
			}
			size_t GetNumOperations() const {
				return m_Operations.size();
			}
			size_t GetNumAutos() const {
				return m_Autos.size();
			}
			OperationRef GetAuto(size_t idx) {
				return idx < m_Autos.size() ? m_Autos[idx].GetRef() : OperationRef();
			}
			OperationRef GetOperation(size_t idx) {
				return idx < m_Operations.size() ? m_Operations[idx].GetRef() : OperationRef();
			}
			
			OperationRef GetOperation(Variable*, const Types::Type*);
			OperationRef GetUnaryOperation(Variable*, bool rhs_var = false);

		private:
			static Sign GetSign(std::string);

		private:
			std::string m_Op;
			OperatorRef m_Ref;
			VecRef<Types::Type> m_Type;
			std::vector<Operation> m_Operations;
			std::vector<Operation> m_Autos;
			bool m_IsConditional = false,
				m_IsAssignment = false;
			Sign m_Sign = Sign::none;
		};
		
		const Type max_operator = Type::max_operator;
		using OperatorTable = Table<OperatorRef>;

		enum class OperatorType {
			None, Inline, Compound, Not
		};

		/*\ OperationValue \*/
		class OperationValue : public CommandValue {

		public:
			OperationValue(Types::Storage& types, size_t type_idx, size_t size, std::string valueid, Types::DataType datatype) : CommandValue(types, type_idx, size, valueid, datatype, Types::ValueSet::Operation)
			{ }
			virtual ~OperationValue() = default;

			size_t GetValueSize(const Operation::Attributes&) const;
		};

		/*\	Master - storage, config & utility \*/
		class Master {
		public:
			Master();
			void Init(Build&);
			template<typename... TArgs>
			OperatorRef Insert(TArgs&&... args) {
				m_Storage.emplace_back(m_Storage, args...);
				return OperatorRef(m_Storage, m_Storage.size() - 1);
			}
			OperatorRef Add(std::string op, VecRef<Types::Type> type, bool is_conditional = false, bool is_assignment = false) {
				return Add(op, type, Operator::Sign::none, is_conditional, is_assignment);
			}
			void Add(std::string op, OperatorRef ref, OperatorType type) {
				if (ref) {
					m_OpMap.emplace(op, std::make_pair(ref, type));
					m_Table.AddOperator(op, ref);
				}
			}
			std::pair<OperatorRef, OperatorType> Get(std::string op, bool cond = false) {
				//static const std::pair<OperatorRef, OperatorType> def;
				auto rg = m_OpMap.equal_range(op);
				std::pair<OperatorRef, OperatorType> pr;
				for (auto it = rg.first; it != rg.second; ++it) {
					if (it->second.first->IsConditional() != cond)
						continue;
					pr = it->second;
				}
				return pr;
			}
			OperatorTable& GetTable() { return m_Table; }
			const OperatorTable& GetTable() const { return m_Table; }
			size_t Size() const { return m_Storage.size(); }

		private:
			OperatorRef Add(std::string op, VecRef<Types::Type> type, Operator::Sign sign, bool is_conditional = false, bool is_assignment = false) {
				auto ref = Insert(op, type, is_conditional, is_assignment, sign);
				if (ref) {
					m_OpMap.emplace(op, std::make_pair(ref, OperatorType::Inline));
					m_Table.AddOperator(op, ref);
				}
				return ref;
			}

		private:
			static const OperatorRef s_InvalidOperatorRef;

			XMLConfiguration* m_Config;
			std::vector<Operator> m_Storage;
			std::unordered_multimap<std::string, std::pair<OperatorRef, OperatorType>> m_OpMap;
			OperatorTable m_Table;
		};
	}
}