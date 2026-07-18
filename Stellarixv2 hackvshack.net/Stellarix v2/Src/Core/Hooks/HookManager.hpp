#pragma once

class CHookManager
{
public:
	CHookManager( ) = default;
	~CHookManager( ) = default;

public:
	auto OnInit( ) -> bool;
	auto OnDestroy( ) -> void;

private:
	bool m_bInitialized = false;
};

auto GetHookManager( ) -> CHookManager*;