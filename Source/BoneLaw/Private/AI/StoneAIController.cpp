// Copyright by MykeUhu

#include "AI/StoneAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/Character/StoneSettlerChar.h"
#include "Core/Components/StoneSettlerActionComponent.h"

AStoneAIController::AStoneAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
	check(Blackboard);
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");
	check(BehaviorTreeComponent);
}

UStoneSettlerActionComponent* AStoneAIController::GetActionComponent() const
{
	const AStoneSettlerChar* Settler = Cast<AStoneSettlerChar>(GetPawn());
	if (!Settler)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[StoneAIController] GetActionComponent: Controlled pawn is not AStoneSettlerChar. Controller=%s Pawn=%s"),
			*GetNameSafe(this), *GetNameSafe(GetPawn()));
		return nullptr;
	}
	return Settler->GetActionComponent();
}
