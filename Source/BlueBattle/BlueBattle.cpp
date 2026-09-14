#include "BlueBattle.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
#include "IModelContextProtocolModule.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "ModelContextProtocolSettings.h"
#endif

class FBlueBattleModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();

#if WITH_EDITOR
		const TCHAR* Cmd = FCommandLine::Get();
		const bool bIsUATCook =
			IsRunningCommandlet()
			|| FCString::Stristr(Cmd, TEXT("-run=Cook")) != nullptr
			|| FCString::Stristr(Cmd, TEXT("-CrashForUAT")) != nullptr;

		if (bIsUATCook)
		{
			// Packaging launches UnrealEditor-Cmd while the interactive editor already
			// owns 127.0.0.1:8000. Starting MCP there logs Error and fails Cook.
			FModuleManager::Get().LoadModule(TEXT("ModelContextProtocolEngine"));
			if (UModelContextProtocolSettings* Settings = GetMutableDefault<UModelContextProtocolSettings>())
			{
				Settings->bAutoStartServer = false;
			}
			return;
		}

		FCoreDelegates::GetOnPostEngineInit().AddLambda([]()
		{
			if (IModelContextProtocolModule* MCP = IModelContextProtocolModule::Get())
			{
				MCP->StartServer();
			}
		});
#endif
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FBlueBattleModule, BlueBattle, "BlueBattle");
