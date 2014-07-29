// comments, trigraph, newline escaping, #include directive
/??/
*aaaa
*/
// #include(\\n)"include.sch"
#include\
"include.sch"		// removal of the closing quote will result in 'unterminated string literal' - SCRambl will ignore the include and proceed
					// (TODO: make SCRambl automatically detect the string termination and successfully include the file, while still noting an error)
//#define FALSE !0==0
#define FALSE !0==0

// unterminated block comments will be treated like single line comments
/* error handling (!)*/		// works (095f1ad9)
// lines beginning with invalid directives are completely skipped
//#invalid_directive		// works (r55)

// #if defined(FALSE) && 0
// error handling complient	(19e7c2d5)
#if defined(FALSE) && 0
// (skip me) #elif * 1
#elif 1
// this is preprocessed
#endif

//#if FALSE
#if FALSE
  Blabadadooo!
#endif

//#define HEY
#define HEY
#undef HEY
//#if defined(HEY)
	//WAIT (1000 + 500)
//#endif

//WAIT 0
WAIT 0

//#if 0
#if 0
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
#endif