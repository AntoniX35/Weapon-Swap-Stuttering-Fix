#include "DKUtil/Hook.hpp"

namespace Fix
{
	void Install()
	{
		auto patchAddress = reinterpret_cast<uintptr_t>(DKUtil::Hook::search_pattern<"48 83 C1 10 E9 93 FF FF FF">());
		if (patchAddress)
		{
			patchAddress += 4;
			INFO("Found the patch address: {:x}", patchAddress);
			REL::safe_write(patchAddress, REL::NOP6, sizeof(REL::NOP6));
			INFO("Installed");
		}
		else
		{
			ERROR("Couldn't find the patch address");
		}
	}
}

DLLEXPORT constinit auto SFSEPlugin_Version = []() noexcept {
	SFSE::PluginVersionData data{};

	data.PluginVersion(Plugin::Version);
	data.PluginName(Plugin::NAME);
	data.AuthorName(Plugin::AUTHOR);
	data.UsesSigScanning(true);
	//data.UsesAddressLibrary(true);
	data.HasNoStructUse(true);
	//data.IsLayoutDependent(true);
	data.CompatibleVersions({
		SFSE::RUNTIME_SF_1_7_29,
		SFSE::RUNTIME_LATEST
	});

	return data;
}();

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type) {
		case SFSE::MessagingInterface::kPostLoad:
			{
				Fix::Install();
				break;
			}
		default:
			break;
		}
	}
}

/**
// for preload plugins
void SFSEPlugin_Preload(SFSE::LoadInterface* a_sfse);
/**/

DLLEXPORT bool SFSEAPI SFSEPlugin_Load(const SFSE::LoadInterface* a_sfse)
{
#ifndef NDEBUG
	MessageBoxA(NULL, "Loaded. You can attach the debugger now or continue", "WeaponSwapStutteringFix SFSE Plugin", NULL);
	// while (!IsDebuggerPresent()) {
	// 	Sleep(100);
	// }
#endif

	SFSE::Init(a_sfse, false);

	DKUtil::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));

	INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);

	// do stuff
	SFSE::AllocTrampoline(1 << 10);

	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
