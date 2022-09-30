#pragma once

namespace db
{
	template<crtp Derived>
	class BasicJob
	{
	public:
		shared_ptr<db::Query> query;
	};
}
