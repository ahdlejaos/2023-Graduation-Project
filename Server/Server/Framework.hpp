#pragma once
#include "Runnable.hpp"

class Framework : public Runnable
{
public:
	Framework();
	~Framework();

	void Awake() override;
	void Start() override;
	void Update(float delta_time) override;
	void Release() override;

	friend void ProceedListen();
	friend void ProceedAccept();
	friend void ProceedOverlapped();
	
	std::priority_queue<int> timerQueue;
};
