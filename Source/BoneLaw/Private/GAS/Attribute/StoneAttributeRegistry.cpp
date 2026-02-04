#include "GAS/Attribute/StoneAttributeRegistry.h"

#include "AttributeSet.h"

bool UStoneAttributeRegistry::TryGet(EStoneAttrId Id, FGameplayAttribute& OutAttr) const
{
	BuildCaches();
	if (const FGameplayAttribute* Found = IdToAttr.Find(Id))
	{
		OutAttr = *Found;
		return OutAttr.IsValid();
	}
	return false;
}

bool UStoneAttributeRegistry::TryGetByStableName(const FName& StableName, FGameplayAttribute& OutAttr) const
{
	BuildCaches();
	if (const FGameplayAttribute* Found = NameToAttr.Find(StableName))
	{
		OutAttr = *Found;
		return OutAttr.IsValid();
	}
	return false;
}

void UStoneAttributeRegistry::BuildCaches() const
{
	if (bCachesBuilt) return;

	IdToAttr.Reset();
	NameToAttr.Reset();

	for (const FStoneAttrBinding& B : Bindings)
	{
		if (!B.Attribute.IsValid() || B.StableName.IsNone())
		{
			continue;
		}
		IdToAttr.Add(B.Id, B.Attribute);
		NameToAttr.Add(B.StableName, B.Attribute);
	}

	bCachesBuilt = true;
}
