// Copyright by MykeUhu


#include "Data/StoneCharacterClassInfo.h"

FStoneCharacterClassDefaultInfo UStoneCharacterClassInfo::GetClassDefaultInfo(EStoneCharacterClass CharacterClass)
{
	return CharacterClassInformation.FindChecked(CharacterClass);
}