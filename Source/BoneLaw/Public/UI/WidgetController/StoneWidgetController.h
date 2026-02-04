#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "UObject/Object.h"
#include "StoneWidgetController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangedSignature, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FStoneAbilityInfo&, Info);

class UStoneAttributeSet;
class UStoneAbilitySystemComponent;
class AStonePlayerState;
class AStonePlayerController;
class UStoneRunSubsystem;
class APlayerController;
class UAbilityInfo;

USTRUCT(BlueprintType)
struct FStoneWidgetControllerParams
{
	GENERATED_BODY()
	/**
	 * Default constructor required by UHT whenever this struct is used as a UFUNCTION parameter.
	 * Keep members initialized to nullptr.
	 */
	FStoneWidgetControllerParams() {};

	FStoneWidgetControllerParams(
		APlayerController* PC,
		APlayerState* PS,
		UAbilitySystemComponent* ASC,
		UAttributeSet* AS,
		UStoneRunSubsystem* SSys
	)
		: PlayerController(PC)
		, PlayerState(PS)
		, AbilitySystemComponent(ASC)
		, AttributeSet(AS)
		, RunSubsystem (SSys)
	{
	}
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetController")
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetController")
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UStoneRunSubsystem> RunSubsystem = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class BONELAW_API UStoneWidgetController : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Stone|UI|WidgetController")
	void SetWidgetControllerParams(const FStoneWidgetControllerParams& WCParams);
	
	UFUNCTION(BlueprintCallable, Category = "Stone|UI|WidgetController")
	virtual void BroadcastInitialValues();
	virtual void BindCallbacksToDependencies();
	
	void BroadcastAbilityInfo();
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<AStonePlayerController> StonePlayerController;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<AStonePlayerState> StonePlayerState;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UStoneAbilitySystemComponent> StoneAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UStoneAttributeSet> StoneAttributeSet;

	AStonePlayerController* GetStonePC();
	AStonePlayerState* GetStonePS();
	UStoneAbilitySystemComponent* GetStoneASC();
	UStoneAttributeSet* GetStoneAS();

	// ============================================================
	// Controller
	// Read-only accessors for UI and Blueprints.
	// Derived controllers inherit these automatically (Overlay, Crafting, etc).
	// ============================================================
 	UFUNCTION(BlueprintPure, Category = "WidgetController")
	APlayerController* GetPlayerController() const { return PlayerController; }

	UFUNCTION(BlueprintPure, Category = "WidgetController")
	APlayerState* GetPlayerState() const { return PlayerState; }

	UFUNCTION(BlueprintPure, Category = "WidgetController")
	UAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystemComponent; }

	UFUNCTION(BlueprintPure, Category = "WidgetController")
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UStoneRunSubsystem> RunSubsystem;
};
