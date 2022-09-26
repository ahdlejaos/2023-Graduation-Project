#pragma once

template<typename Ty>
class BasicDatabaseJob
{
public:
	shared_ptr<DatabaseQuery> query;
};
