#header

__SEG0:
GOTO __SEG1

#hex
	@App.ID
	#each @Variables $1:Type
		#if $1 & VAR
			00 00 00 00
		#endif
	#endeach
#endhex

__SEG1:
#hex
	01 @Script.NumModels
	
	#if @Script.NumModels
		#each @Script.Models $1:Name
			$1
			#each 24 - $1.Length
				00
			#endeach
		#endeach
	#endif
#endhex

__SEG2:
{
	LVAR_INT HighestNumLVARs
	#hex
		02 @Script.Size @Script.LargestMissionSize @Script.NumMissions
		#each @SubScripts $1:NumLVARs
		#endeach
	#endhex
}
GOTO __SEG1

#endheader