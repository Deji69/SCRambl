/*
	Example of applying a custom license plate texture generated by the .exe to a vehicle
	Written in SCR for SCRambl 1.0.40+
*/

//COMMAND SCRIPT_NAME:0x3A4(TEXT_LABEL[8])
//VAR_INT var1
//var1 = 0 + 1
/*WaIT 0
WAiT 500
WAIt -1
wAIT -9999
waIT 0xFF
WAit 0xFFFF
WAIT 0xffffff
WAIT 0x7FfFfFfF
WAIT -0x7FfFfFfF
WAIT -0xFF
WAIT 0x7F*/
start:
{
	VAR_INT var1 var2
	IF var1 == 0
		var1 = 2 * var1 + (2 + 1) + 5 + var2
	ENDIF
	WAIT var1
	LVAR_INT lvar1 lvar2
	var1 += lvar2
	lvar1 = var1
	lvar1 *= -1
	LINE 1.0 -2.0 3.14159 4.444444445 55555.0 -666.666
	WAIT lvar1
	WAIT lvar2
	SCRIPT_NAME "tool"
}
{
	//LVAR_INT lvar3 lvar4
	LVAR_INT lvar1 lvar2
	//WAIT lvar3
	//WAIT lvar4
	WAIT lvar1
	WAIT lvar2
}
GOTO start
GOTO end
end:
/*{
	LVAR_INT lvar1[2] pVehicleStruct
	#define CEntity_wModelIndex 0x22
	lvar1[1] = LVAR_INT:0x54BA98[0][WORD:pVehicleStruct[/*0x22*/CEntity_wModelIndex]]
}

#register_var 0x8 VAR_INT player
#register_var 0xC VAR_INT scplayer
#define CCustomCarPlateMgr_CreatePlateTexture 0x6FDEA0
#define CModelInfo_ms_modelInfoPtrs 0xA9B0C8
#define RwTextureDestroy 0x7F3820
#define PLATE_STYLE_LS 0

// Useful SCRambl abilities - we can pretty much create 'inline' commands and re-order/automate params :)
#register_command 0xAA7 _CREATE_CAR_PLATE_TEXTURE(INT,INT,INT,INT,STRING,=INT,0)
#define CREATE_CAR_PLATE_TEXTURE(a,b,c) _CREATE_CAR_PLATE_TEXTURE CCustomCarPlateMgr_CreatePlateTexture 2 2 b a c

{
LVAR_INT hCar[2] hCarLastUpdated pCar pTexture pCarPlateTexture
WAIT 0

hCarLastUpdated = -1

mainloop:
WAIT 100

IF IS_PLAYER_PLAYING player
	IF IS_CHAR_IN_ANY_CAR scplayer
		
		GET_CAR_CHAR_IS_USING (scplayer) hCar
		
		IF NOT hCarLastUpdated == hCar
			GET_CAR_POINTER (hCar) pCar
			
			pCarPlateTexture = pCar + 0x588
		
			// Get the currently applied texture
			READ_MEMORY ( pCarPlateTexture 4 FALSE ) pTexture
			//PRINT_FORMATTED_NOW "0x%X" 100 pTexture
			
			/*
				If there IS a texture we have to check if it's custom, which are ones applied to all models
				or default, which are generated on the vehicles creation and destroyed when the vehicle ceases to exist
			*/
			IF NOT pTexture == 0
				LVAR_INT pVehicleModelInfo pData pTextureRefs nRefs nCarModel
				
				// Get the model info pointer, where the default texture is stored
				GET_CAR_MODEL ( hCar ) nCarModel
				nCarModel *= 4
				pVehicleModelInfo = CModelInfo_ms_modelInfoPtrs + nCarModel
				
				// Read the vehicle model info pointer
				READ_MEMORY (pVehicleModelInfo 4 FALSE) pData
				
				// Get the custom plate material for this vehicle model
				pData = pVehicleModelInfo + 0x24
				READ_MEMORY ( pData 4 FALSE ) pData
				
				// If that went well, get the texture pointer from the material pointer
				IF NOT pData == 0
					READ_MEMORY ( pData 4 FALSE ) pData
				ENDIF
				
				/*
					If the texture we got from the vehicle matches that of the one that's used as the default for all the ones of this model,
					it means that 0674 or something else has set the texture (you might not want to mess with it in this case), otherwise
					the texture has just been generated randomly meaning we need to destroy it before applying our own texture (otherwise
					the game doesn't destroy it when the vehicles existence ceases and instead only destroys ours - memory leak!)
					
					For this example, it will be destroyed either way to demonstrate how to properly do it...
				*/
				IF pTexture == pData
					/*
						Ok, this plate has been globally created - it may be shared by other vehicles if we
						just destroyed it, other vehicles might be still referencing it and the game will crash.
						So just de-reference it and let the texture storage system figure out when it can be destroyed.
					*/
					pTextureRefs = pTexture + 0x54
					
					// Get the number of references, decrease by 1, and set
					READ_MEMORY ( pTextureRefs 4 FALSE ) nRefs
					--nRefs
					WRITE_MEMORY ( pTextureRefs 4 nRefs FALSE )
				ELSE
					// Otherwise, it must be automatically generated and we can happily just DESTROY
					CALL_FUNCTION RwTextureDestroy 1 1 pTexture
				ENDIF
				
				/*
					If you change this code, remember to consider what you're leaving for the game.
					In this case, after we've destroyed this texture, we're about to replace it with another.
					If this weren't to happen, the inexistant texture would still be referenced and the game may
					attempt to use or destroy it, causing a crash. Clearing this field can prevent the game doing that:
					WRITE_MEMORY pCarPlateTexture 4 0 FALSE
				*/
			ENDIF
			
			// Generate a car plate texture
			//CALL_FUNCTION_RETURN ( CCustomCarPlateMgr_CreatePlateTexture 2 2 PLATE_STYLE_LS (STRING:"D3J1_W1N5") ) pTexture
			CREATE_CAR_PLATE_TEXTURE "D3J1_W1N5" PLATE_STYLE_LS pTexture
			PRINT_FORMATTED_NOW "0x%X" 100 pTexture
			
			// Write the texture to the vehicles plate texture field
			WRITE_MEMORY pCarPlateTexture 4 pTexture FALSE
			
			/*
				Try to make sure we dont do it repeatedly attempt while still in the same vehicle.
				Note that this isn't the most solid method of preventing un-necessary recreations of 
				the texture, however we'd need to set up our own array of vehicle data to do it 'properly'...
			*/
			hCarLastUpdated = hCar
		ELSE
			WAIT 1500
		ENDIF
	ENDIF
ENDIF
GOTO mainloop
}*/