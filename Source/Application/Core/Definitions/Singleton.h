#pragma once

template <typename T>
class Singleton
{
public:
	Singleton() = default;
	~Singleton() = default;

	static T& Get() 
	{
		static T instance;
		return instance;
	}
};