#include "UInteraction.h"
#include "PIDController3D.h"

// Constructor
PIDController3D::PIDController3D(float ProportionalVal, float IntegralVal, float DerivativeVal,
	float OutMaxVal, float OutMinVal) :
	P(ProportionalVal),
	I(IntegralVal),
	D(DerivativeVal),
	OutMax(OutMaxVal),
	OutMin(OutMinVal)
{
	IErr = FVector(0.f);
	PrevErr = FVector(0.f);
}

// Destructor
PIDController3D::~PIDController3D()
{
}

// Set all PID values
void PIDController3D::SetValues(float ProportionalVal, float IntegralVal, float DerivativeVal,
	float OutMaxVal, float OutMinVal)
{
	P = ProportionalVal;
	I = IntegralVal;
	D = DerivativeVal;
	OutMax = OutMaxVal;
	OutMin = OutMinVal;

	PIDController3D::Reset();
};

// Update the PID loop
FVector PIDController3D::Update(const FVector Error, const float DeltaTime)
{
	if (DeltaTime == 0.0f)
	{
		return FVector(0.f);
	}

	// Calculate proportional output
	const FVector POut = P * Error;

	// Calculate integral error / output
	IErr += DeltaTime * Error;
	const FVector IOut = I * IErr;

	// Calculate the derivative error / output
	const FVector DErr = (Error - PrevErr) / DeltaTime;
	const FVector DOut = D * DErr;

	// Set previous error
	PrevErr = Error;

	// Calculate the output
	const FVector Out = POut + IOut + DOut;


	// If Out>Outmax --> return Outmax, else if Out<Outmin --> return Outmin, else return Out
	if (OutMax > 0.f || OutMin < 0.f)
	{
		return FVector(
			(Out.X > OutMax) ? (OutMax) : (Out.X < OutMin) ? (OutMin) : (Out.X),
			(Out.Y > OutMax) ? (OutMax) : (Out.X < OutMin) ? (OutMin) : (Out.Y),
			(Out.Z > OutMax) ? (OutMax) : (Out.X < OutMin) ? (OutMin) : (Out.Z)
		);
	}
	else
	{
		return Out;
	}
};

// Update the PID loop
FVector PIDController3D::UpdatePD(const FVector Error, const float DeltaTime)
{
	if (DeltaTime == 0.0f)
	{
		return FVector(0.f);
	}

	// Calculate proportional output
	const FVector POut = P * Error;

	// Calculate the derivative error / output
	const FVector DErr = (Error - PrevErr) / DeltaTime;
	const FVector DOut = D * DErr;

	// Set previous error
	PrevErr = Error;

	// Calculate the output
	const FVector Out = POut + DOut;

	// If Out>Outmax --> return Outmax, else if Out<Outmin --> return Outmin, else return Out
	if (OutMax > 0.f || OutMin < 0.f)
	{
		return FVector(
			(Out.X > OutMax) ? (OutMax) : (Out.X < OutMin) ? (OutMin) : (Out.X),
			(Out.Y > OutMax) ? (OutMax) : (Out.X < OutMin) ? (OutMin) : (Out.Y),
			(Out.Z > OutMax) ? (OutMax) : (Out.X < OutMin) ? (OutMin) : (Out.Z)
		);
	}
	else
	{
		return Out;
	}
};

// Reset error values of the PID
void PIDController3D::Reset()
{
	PrevErr = FVector(0.f);
	IErr = FVector(0.f);
};