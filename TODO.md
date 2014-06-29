# SCRambl

Deadline: V?

## Engine ##
- [ ] Task System
	- [x] Event System
	- [ ] Error Reporting
- [x] Lexer Engine
- [x] Preprocessor Tasks
- [ ] Parser Tasks
- [ ] Compiler Tasks
- [ ] Linker Tasks

## Lexer ##
- [x] Scanner Engine
- [ ] Numeric Scanner
	- [x] Integer Scanning
	- [ ] Float Scanning
- [x] Identifier Scanner
- [x] Directive Scanner
- [x] Operator Scanner
- [x] String Literal Scanner
- [x] Comment Scanner

## Preprocessor ##
- [ ] Preprocessor Features
	- [ ] Macros
		- [x] Macro Definition (#define)
		- [x] Macro Undefinition (#undef)
		- [x] Macro Expansion
		- [ ] Function-like Macros
		- [ ] Predefined Macros
	- [x] Expressions
		- [x] Arithmetic Operations
		- [x] Binary Operations
		- [x] Comparison Operations
		- [x] Logical Operations
		- [x] Unary 'defined' Operator
		- [x] Expression Grouping
	- [x] Conditional Compilation
		- [x] #if
		- [x] #ifdef
		- [x] #ifndef
		- [x] #else
		- [x] #elif
		- [x] #endif
	- [ ] Other Directives
		- [x] #include
		- [ ] #error
		- [ ] #warning
		- [ ] # ?
- [ ] Basic Syntax
	- [x] Comment Stripping
	- [x] Line Splicing
	- [x] Character Mapping
	- [ ] Tokenization

## Parser ##
- [ ] SCR Elements
	- [ ] Commands
	- [ ] Control Structures
	- [ ] Labels
	- [ ] Operators
	- [ ] Variables
		- [ ] Arrays
		- [ ] Predefined / Timers
	- [ ] Types
		- [ ] Basic Types
		- [ ] Extended Types
- [ ] Control Structures
	- [ ] IF..ENDIF
	- [ ] WHILE..ENDWHILE
	- [ ] REPEAT..ENDREPEAT
	- [ ] SWITCH..CASE..DEFAULT..ENDSWITCH
- [ ] Command Syntax
	- [ ] Command Overloading
	- [ ] Argument Type Detection
- [ ] Label Syntax
	- [ ] Scope
- [ ] Variable Syntax
	- [ ] Scope
- [ ] Operator Syntax
	- [ ] Basic Operations
	- [ ] Incremental Operations
	- [ ] Complex Operations
	- [ ] Chained Operations
	- [ ] Operator Overloading
	- [ ] Operand Type Detection
- [ ] Other
	- [ ] Constant Expressions ?

## Compiler ##
- [ ] Translation
- [ ] Compilation

## Linker ##
- [ ] Linker Formats
	- [ ] SCM
	- [ ] CLEO Script
	- [ ] CLEO++ ?
- [ ] Linkage Elements/Targets
	- [ ] Commands <> Game
	- [ ] Labels <> Sub-scripts
	- [ ] Functions <> CLEO Scripts
	- [ ] Text Labels <> GXT ?
	- [ ] Library ?
