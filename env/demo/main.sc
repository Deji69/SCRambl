#define FALSE 0

#if FALSE
  ALPHA Checklist:
	[*] Basic Syntax
		[x] Inline & block comments
		[*] Non-literal strings
		[*] Literal strings
		[*] Labels / offsets
	[*] Scripts
		[*] Scope
		[*] Sub-scripts
		[*] MAIN header
	[*] Control structures
		[*] IF..ENDIF
		[*] WHILE..ENDWHILE
		[*] REPEAT..ENDREPEAT
		[*] CASE..ENDCASE
	[*] Variables
		[*] Variable declarations
		[*] Variable scope
		[*] Variable arrays
		[*] Predefined vars / Timers
	[*] Commands
		[*] Arg-less commands
		[*] Commands with args
		[*] Command overloading
		[*] Argument type detection
	[*] Operators
		[*] Basic operations
		[*] Incremental operations
		[*] Complex operations
		[*] Multiple operations
		[*] Operation overloading
		[*] Operand type detection
	[*] Preprocessor
		[*] In-script command registering + overloading
		[*] In-script variable registering
		[*] Empty macros
		[*] Code macros
		[*] Macros with args
		[*] Preprocessor logic
		[*] Preprocessor blocks
		[*] Hex input
		[*] ASM input
	[*] Miscellaneous
		[*] Case-sensitive
		[*] Translations
		[*] Type extension
		[*] STRICT/BASIC/NONE type matching
		[*] Console
		[*] Optimisations
	46/48
#endif

#define HEY
#undef HEY
#if defined(HEY)
	WAIT 1000
#endif

#register_command 0FFF SUM_COMMAND(i,=f)

GOTO End

VAR_INT var1 var2
VAR_FLOAT array[4]

var1 = var2

#register_var 38 LVAR_INT custom_var 0 true

custom_var = 38
//SUM_COMMAND(var1, array[var1])  // FF 0F 02 08 00 07 0C 00 08 00 04 81

timera = 0

IF NOT var1 = 0
OR NOT array[var1] = 0
	++var1
ELSE
	var1--
ENDIF

CASE var1
	WHEN 0
		GOTO End
	WHEN 1
		++var1
	ELSE
		var1 = 0
ENDCASE

WHILE NOT var1 = 0
OR NOT array[var1] = 0
	++var1
	var1--
ENDWHILE

REPEAT 5 var1
	PRINTINT2 "__SCRAMBL_VER" __SCRAMBL_VER
ENDREPEAT

array[var1] = 1.0

SAVE_STRING_TO_DEBUG_FILE "HALLO!"

PRINT_BIG(SCRambl,5000,2)
PRINT_BIG(SCRamble,5000,2)

{
	LVAR_INT scr_lvar
	LVAR_FLOAT scr_fvar
	
	var1 = 5 + 5 + 5		/* compiled:" aaa"
								var1 = 5
								var1 += 10/**/
								(could be improved to simply var = 15)
								*/
	var1 = var1 + 4			// compiled: var1 += 4
	var1 = 0 + 5
	
	scr_fvar = 1.0
	scr_fvar += 5.565656
	scr_fvar *= -2.0
	
	var1 += 999999
	++var1
	var1++
	var1 =# 11
	++ var1
	var1 ++
	
	#hex
	SCRIPT_NAME 09 4D41494E00000000 /* 'MAIN' */
	DO_FADE 04 00 04 00
	2C04 05 9300// 5B
	0D03 05 BB00
	GOTO 01 hex_label
	0600 03 scr_lvar 01 hex_label

	hex_label:
	PRINTSTRING 0E 08 "SCRambl" 00
	#endhex
	
	ABSF(scr_fvar)
}
{
	LVAR_INT lvar1 lvar2
	lvar1 = lvar2 + 5000
	lvar1 = 0
	lvar2 = 366
	var1 = 0
}
End: