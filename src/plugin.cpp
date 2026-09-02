#include "Menu.h"

namespace
{
	void OnSFSEMessage(SFSE::MessagingInterface::Message* a_message)
	{
		if (!a_message ||
			a_message->type != SFSE::MessagingInterface::kPostLoad) {
			return;
		}

		if (SFSEMenuFrameworkExample::Menu::Register()) {
			logger::info(
				"Registered the nested diagnostics, lifecycle, input, HUD, font, "
				"and standalone-window examples.");
		} else {
			logger::warn(
				"SFSE Menu Framework is unavailable or rejected a required callback; "
				"the example remains inactive.");
		}
	}
}

SFSE_PLUGIN_LOAD(const SFSE::LoadInterface* a_sfse)
{
	if (!a_sfse) {
		return false;
	}

	constexpr SFSE::InitInfo initInfo{
		.logPattern = "%Y-%m-%d %H:%M:%S.%e [%l] %v",
		.trampoline = false,
		.hook = false
	};
	SFSE::Init(a_sfse, initInfo);

	const auto* messaging = SFSE::GetMessagingInterface();
	if (!messaging || !messaging->RegisterListener(OnSFSEMessage)) {
		logger::critical("Could not register the SFSE lifecycle listener.");
		return false;
	}

	logger::info("Initialized; waiting for SFSE post-load.");
	return true;
}
