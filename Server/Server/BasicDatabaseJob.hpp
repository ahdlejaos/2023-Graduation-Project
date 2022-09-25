#pragma once

template<typename Ty>
struct BasicDatabaseJob
{
	shared_ptr<DatabaseQuery> query;
};
