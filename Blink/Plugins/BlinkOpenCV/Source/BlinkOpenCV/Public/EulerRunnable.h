// Copyright 2022 Liam Hall. All Rights Reserved.
// Created on 18/12/2022.
// NHE2422 Advanced Computer Games Development Assignment 2.

#pragma once

/**
 * @brief Acts like a functional thread, but is also thread-safe when used with TSharedPtr.
 *
 * Ideally, this thread type should be created as a TSharedPtr, and passed around as a TWeakPtr. Only the owning
 * object (the object which created this thread) should have a TSharedPtr of this thread.
 * 
 * When it is time to kill this thread, the owner should call the StopThread method and reset the TSharedPtr.
 * 
 * StopThread only stops the internal thread, but the object will still exist until no more references to the thread
 * exists (the TSharedPtr is reset and all of the TWeakPtr Pins are out-of-scope). This is intentional as it allows
 * other threads which are currently accessing this thread to do so without issue.
 *
 * If the thread isn't going to be accessed by more than one thread, then you can use raw C++ pointers with delete
 * instead (no need to call StopThread).
 */
class FEulerThread : public FRunnable
{
public:
	FEulerThread(FString InThreadName, double InTickRate = 1.f/60.f, EThreadPriority InThreadPriority = TPri_Normal);
	
	// Overriden from FRunnable
	
	// Do not call or inherit!
	virtual bool Init() override;
	// Do not call or inherit!
	virtual uint32 Run() override;
	// Do not call or inherit!
	virtual void Exit() override;
	// Do not call or inherit!
	virtual void Stop() override;
	

	// Only call from one thread.
	void SetPriority(EThreadPriority InThreadPriority);

	// Only call from one thread.
	void SetTickRate(double InTickRate);

	// Only call from one thread.
	void SetTickEnabled(bool bInTickEnabled);

	
	// Only call from owning thread.
	void StartThread();

	// Only call from owning thread.
	void StopThread();

	
	// Called by owning thread just after construction.
	virtual void OnConstruction();
	
	// Called by owning thread just before the thread is started.
	virtual void OnPreStart();
	
	// Called by this worker thread just after it starts.
	virtual void OnStart();
	
	// Called by this worker thread once every tick.
	virtual void OnTick(const double& DeltaTime);
	
	// Called by owning thread after this thread has been requested to stop. 
	virtual void OnPreStop();
	
	/**
	 * @brief Called by this worker thread just before the thread stops.
	 *
	 * This method should contain stopping current tasks and cleaning-up of data that is entirely self-contained within
	 * this thread. For data that is shared between threads, clean it up in OnDestroy instead.
	 */
	virtual void OnStop();

	/**
	 * @brief Called by whichever thread deconstructed this object.
	 *
	 * This method should contain the remaining clean-up that couldn't be done in OnStop due to thread-safety.
	 */
	virtual void OnDestroy();

	
	bool IsActive() const { return bThreadActive; }
	const FString& GetName() const { return ThreadName; }
	double GetTickRate() const { return ThreadTickRate; }

	
	// Only call from one thread.
	void AddChildThread(FEulerThread* Runnable);
	
	// Only call from one thread.
	void RemoveChildThread(FEulerThread* Runnable);
	
	void ContainsChildThread(FEulerThread* Runnable);
	
	
	virtual ~FEulerThread() override;

private:
	FString ThreadName;
	double ThreadTickRate;
	bool bThreadActive;
	FRunnableThread* Thread;
	TArray<TSharedPtr<FEulerThread>> ChildRunnables;
};
