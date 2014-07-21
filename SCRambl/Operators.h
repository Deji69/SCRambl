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
		/*\
		 - Operator::Type - built-in operator types
		\*/
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
		public:
			/*\
			 - Operator::Table::Cell - If it doesnt store an operator or another non-useless cell, it's useless (recursive)
			 - Get the first cell via the Operator::Table
			\*/
			class Cell
			{
				friend class Table;

				T						m_Operator = max;
				std::vector<Cell>		m_Cells;

			public:
				// For reservation of cells for any grapheme character - wastes a lot of space, but we can access things quickly :)
				void ReserveCells() {
					m_Cells.resize(max);
				}

				// Obviously doesnt work if theres no way to get an operator by adding that grapheme, of course
				bool Next(Grapheme graph, const Cell *& next_out) const
				{
					// ensure we have a cell for this grapheme
					if (m_Cells.size() >= (unsigned)graph)
					{
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
				inline T GetOperator() const		{ return m_Operator; }
			};
			
		private:
			std::vector<Cell>		m_Cells;

		public:
			Table() : m_Cells(max)
			{}

			/*\
			 - Allocates cells for the path of graphemes leading to an operator
			 - Adding <, << and <=, for example, would allocate one initial cell for '<' and then two cells for '<' and '='
			 - Each final cell will have the operator type ID assigned to it, which can be retrieved once the cell is obtained
			\*/
			void AddOperator(CodeLine code, T type)
			{
				Cell * pCell = nullptr;
				bool got_first_cell = false;

				// use the grapheme from each symbol to walk down the row of cells
				for (auto c : code.Symbols())
				{
					ASSERT(c.HasGrapheme() && "Only symbols with graphemes (specially recognized symbols) can be added as operators");
					if (!got_first_cell)
					{
						pCell = &m_Cells[c.GetGrapheme()];
						got_first_cell = true;
					}
					else
					{
						pCell->ReserveCells();
						pCell->Next(c.GetGrapheme(), pCell);
					}
						
				}

				ASSERT(got_first_cell && "Use a CodeLine that actually contains Symbols");

				if (got_first_cell)
				{
					// assign an operator to that cell
					pCell->m_Operator = type;
				}
			}

			/*\
			 - Get the first cell of an operator string
			 - e.g. if the grapheme was 'plus' (+), there could be two possible following cells: 'plus' (+) or 'equals' (=)
			 - it all depends on which operators are in this table, of course
			\*/
			inline const Cell & GetCell(Grapheme graph) const
			{
				ASSERT(m_Cells.size() >= (unsigned)graph && "Something went wrong internally - or an out of range grapheme ID was used");
				return m_Cells[graph];
			}
		};

		/*\
		 - Operator::Scanner - Operator scanner for lexage
		\*/
		template<typename T, T max>
		class Scanner : public Lexer::Scanner
		{
							Operator::Table<T, max>			&	m_Table;
			typename const	Operator::Table<T, max>::Cell	*	m_Cell;

		public:
			Scanner(Operator::Table<T, max> & table) : m_Table(table)
			{}

			bool Scan(Lexer::State & state, Script::Position & pos)
			{
				switch (state)
				{
					// Check, check, check fo da cell dat sells
				case Lexer::State::before:
					if (!pos->HasGrapheme()) return false;
					m_Cell = &m_Table.GetCell(pos->GetGrapheme());
					state = Lexer::State::inside;
					++pos;
					return true;

				case Lexer::State::inside:
					for (; pos && pos->HasGrapheme(); ++pos)
					{
						if (!m_Cell->Next(pos->GetGrapheme(), m_Cell))
							return false;
					}
					if (m_Cell->GetOperator() != max)
					{
						state = Lexer::State::after;
						return true;
					}
					return false;

				case Lexer::State::after:
					return true;
				}
				return false;
			}

			T GetOperator() const		{ ASSERT(m_Cell && "Can only get the operator after a succesful scan");  return m_Cell->GetOperator(); }
		};
	};
}