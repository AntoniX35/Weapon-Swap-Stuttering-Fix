/* 
 * https://github.com/Starfield-Reverse-Engineering/CommonLibSF
 * This plugin template links against CommonLibSF
 */

#include "DKUtil/Hook.hpp"
ptrdiff_t offset;
namespace WeaponSwapStutteringFix
{
	void Install()
	{
		REL::Relocation<std::uintptr_t> FixAddress1{ REL::ID(135088), offset };
		
		std::uint8_t NOP5[] = { 0x0F, 0x1F, 0x44, 0x00, 0x00 };

		REL::safe_write(FixAddress1.address(), NOP5, sizeof(NOP5));
		
		{				
			REL::Relocation<std::uintptr_t> target{ REL::ID(135015), 0x19A }; 
			struct Patch1_Code : Xbyak::CodeGenerator
				{
					Patch1_Code(std::uintptr_t a_address)
						{
							Xbyak::Label retnLabel;

							mov(rdx, rsi);
							mov(rcx, rbx);
							mov(dl, 1);
	
							jmp(ptr[rip + retnLabel]);

							L(retnLabel);
							dq(a_address + 0x6);
						
						}
				};
			Patch1_Code code(target.address());
			code.ready();

			auto& trampoline = SFSE::GetTrampoline();
			trampoline.write_branch<6>(
				target.address(),
				trampoline.allocate(code));
		}
		// {				
			// REL::Relocation<std::uintptr_t> target{ REL::ID(144857), -0x28C }; 
			// struct Patch2_Code : Xbyak::CodeGenerator
				// {
					// Patch2_Code(std::uintptr_t a_address, std::uintptr_t a_call_address)
						// {
							// Xbyak::Label retnLabel;
							// Xbyak::Label callLabel;

							// mov(dl, 1);
							// call(ptr[rip + callLabel]);
	
							// jmp(ptr[rip + retnLabel]);
							
							// L(callLabel);
							// dq(a_call_address);

							// L(retnLabel);
							// dq(a_address + 0x5);
						
						// }
				// };
			// REL::Relocation<std::uintptr_t> callTarget{ REL::ID(151202) };
			// Patch2_Code code(target.address(), callTarget.address());
			// code.ready();

			// auto& trampoline = SFSE::GetTrampoline();
			// trampoline.write_branch<5>(
				// target.address(),
				// trampoline.allocate(code));
		// }

		INFO("Installed");
	}
}

namespace
{
	void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type) {
		case SFSE::MessagingInterface::kPostLoad:
			{
				WeaponSwapStutteringFix::Install();
			}
			break;
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
	MessageBoxA(NULL, "Loaded. You can now attach the debugger or continue execution.", Plugin::NAME.data(), NULL);
#endif

	SFSE::Init(a_sfse, false);
	DKUtil::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));
	INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);
	const REL::Version version = a_sfse->RuntimeVersion();
	if (version == SFSE::RUNTIME_SF_1_7_29 || version == SFSE::RUNTIME_SF_1_7_23 ) {
		offset = 0x48;
	}
	if (version >= SFSE::RUNTIME_SF_1_7_33) {
		offset = 0x4A;
	}
	// do stuff
	// this allocates 1024 bytes for development builds, you can
	// adjust the value accordingly with the log result for release builds
	SFSE::AllocTrampoline(1 << 7);

	SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
