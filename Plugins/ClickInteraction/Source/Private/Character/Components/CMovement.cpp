#define CROUCHING_HEIGHT 0.3f
#define CROUCH_SPEED 0.2f

#include "CMovement.h"

//#include "../Private/Character/CharacterController.h"
#include "Runtime/Engine/Classes/GameFramework/Controller.h"
#include "Runtime/Engine/Classes/GameFramework/CharacterMovementComponent.h"



// Sets default values for this component's properties
UCMovement::UCMovement()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	
	MinMovementSpeed = 0.05f;
	MaxMovementSpeed = 0.25f;
	

	// DefaultSpeed = MaxMovementSpeed;
	// ...
}


// Called when the game starts
void UCMovement::BeginPlay()
{
	Super::BeginPlay();

	CurrentSpeed = MaxMovementSpeed;

	Character = Cast<ACharacter>(GetOwner()); // Setup player

	if (Character != nullptr) {
		DefaultHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	SetMovable(true);
}

void UCMovement::MoveForward(const float Val) {
	if (bCanMove == false) return;



	if ((Character->Controller != nullptr) && (Val != 0.0f))
	{
		// Find out which way is forward
		FRotator Rotation = Character->Controller->GetControlRotation();
		// Limit pitch when walking or falling
		if (Character->GetCharacterMovement()->IsMovingOnGround() || Character->GetCharacterMovement()->IsFalling())
		{
			Rotation.Pitch = 0.0f;
		}
		// add movement in that direction
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		Character->AddMovementInput(Direction, Val * CurrentSpeed);
	}
}

void UCMovement::MoveRight(const float Val) {
	if (bCanMove == false) return;

	if ((Character->Controller != nullptr) && (Val != 0.0f))
	{
		// Find out which way is right
		const FRotator Rotation = Character->Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		// add movement in that direction
		Character->AddMovementInput(Direction, Val * CurrentSpeed);
	}
}

void UCMovement::AddControllerPitchInput(const float Val) {
	if (bCanMove == false) return;

	if (Character != nullptr)
	{
		Character->AddControllerPitchInput(Val);
	}
}

void UCMovement::AddControllerYawInput(const float Val) {
	if (bCanMove == false) return;
	if (Character != nullptr)
	{
		Character->AddControllerYawInput(Val);
	}
}

void UCMovement::ToggleCrouch()
{
	if (bIsCrouching) {
		// We stopped crouching
		bIsCrouching = false;
		GetOwner()->GetWorldTimerManager().SetTimer(CrouchTimer, this, &UCMovement::SmoothStandUp, 0.001f, true);
	}
	else {
		// We started crouching
		bIsCrouching = true;
		GetOwner()->GetWorldTimerManager().SetTimer(CrouchTimer, this, &UCMovement::SmoothCrouch, 0.001f, true);
	}
}

void UCMovement::SmoothCrouch()
{
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();

	const float CurrHeight = Capsule->GetScaledCapsuleHalfHeight();
	Capsule->SetCapsuleHalfHeight(CurrHeight - CROUCHING_HEIGHT);

	if (CurrHeight <= (DefaultHeight * CROUCHING_HEIGHT)) {
		GetOwner()->GetWorldTimerManager().ClearTimer(CrouchTimer);
	}
}

void UCMovement::SmoothStandUp()
{
	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();

	const float CurrHeight = Capsule->GetScaledCapsuleHalfHeight();
	Capsule->SetCapsuleHalfHeight(CurrHeight + CROUCHING_HEIGHT);

	if (CurrHeight >= DefaultHeight) {
		GetOwner()->GetWorldTimerManager().ClearTimer(CrouchTimer);
	}
}


// Called every frame
void UCMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCMovement::SetupKeyBindings(UInputComponent * PlayerInputComponent)
{
	/* Setup Input
	/* In Project setings -> Input
	/* Axis Mappings:
	/*   MoveForward: W 1.0, S -1,0
	/*   MoveRight: D 1.0, A -1.0
	/*   CameraPitch: MouseY -1.0
	/*   CameraYaw: MouseX 1.0
	*/

	// Set up gameplay key bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &UCMovement::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &UCMovement::MoveRight);
	// Default Camera view bindings
	PlayerInputComponent->BindAxis("CameraPitch", this, &UCMovement::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("CameraYaw", this, &UCMovement::AddControllerYawInput);

	PlayerInputComponent->BindAction("ToggleCrouch", IE_Pressed, this, &UCMovement::ToggleCrouch);
	PlayerInputComponent->BindAction("ToggleCrouch", IE_Released, this, &UCMovement::ToggleCrouch);

}

void UCMovement::SetMovable(bool bCanMove)
{
	this->bCanMove = bCanMove;
}
