#include "FloorGeneratorBase.h"

AFloorGeneratorBase::AFloorGeneratorBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFloorGeneratorBase::GenerateModule()
{
	//Base version does nothing. Derived classes override.
}