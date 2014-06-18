/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

namespace SCRambl
{
	/*\
	 - Sincerely yours, the smooth operator
	\*/
	class Operator
	{
	public:
		enum Type
		{
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

		/*\
		 - Operator::Table - the place where operators go
		 - Uses the Symbols vector of CodeLine to assign new operators, and retrieve them later
		\*/
		template<typename T, T max>
		class Table
		{
			struct Cell
			{
				T						m_Operator = max;
				std::vector<Cell>		m_Cells;
			};
			
			std::vector<Cell>		m_Cells;

		public:
			Table() : m_Cells(max)
			{}

			void AddOperator(CodeLine code, T type)
			{
				std::vector<Cell> * pRow = &m_Cells;
				Cell * pCell = nullptr;

				// use the grapheme from each symbol to walk down the row of cells
				for (auto c : code.Symbols())
				{
					ASSERT(c.HasGrapheme());
					pCell = &(*pRow)[c.GetGrapheme()];
					pRow = &pCell->m_Cells;
					pRow->resize(max);
				}

				// assign an operator to that cell
				pCell->m_Operator = type;
			}

			T GetOperator()
			{

			}
		};

		template<typename T, T max>
		class Scanner : public Lexer::Scanner
		{
			Operator::Table<T, max>		&	m_Table;
			T								m_Operator;

		public:
			Scanner(Operator::Table<T, max> & table) : m_Table(table)
			{}

			bool Scan(Lexer::State & state, Script::Position & pos)
			{
				switch (state)
				{
				case Lexer::State::before:
					if (pos->HasGrapheme())
					{

					}
					return false;

				case Lexer::State::inside:
					return false;

				case Lexer::State::after:
					return false;
				}
				return false;
			}

			T GetOperator() const			{ return m_Operator; }
		};
	};
}