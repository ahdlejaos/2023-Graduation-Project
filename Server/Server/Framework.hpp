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

	void ProceedAsync(Asynchron*, DWORD bytes);
	void ProceedConnect(Asynchron*);
	void ProceedSent(Asynchron*);
	void ProceedRecv(Asynchron*);
	
	void Listen();
	void Accept();
	void Dispose();

private:
	unique_ptr<Asynchron> connectWorker;
	DWORD connectBytes;
	char connectBuffer[BUFSIZ];
	atomic<SOCKET> connectNewbie;
	std::condition_variable connectFlag;

	std::array<shared_ptr<Room>, MAX_ROOMS> everyRooms;
	std::array<shared_ptr<Session>, MAX_ENTITIES> everySessions;

	std::priority_queue<int> timerQueue;
};
