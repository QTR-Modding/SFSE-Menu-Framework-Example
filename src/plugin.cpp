#include "Menu.h"

#include <cstdint>

namespace
{
	void OnSFSEMessage(SFSE::MessagingInterface::Message* a_message)
	{
		if (!a_message ||
			a_message->type != SFSE::MessagingInterface::kPostLoad) {
			return;
		}

		const auto result = SFSEMenuFrameworkExample::Menu::Register();
		if (result == SFSEMenuFramework::Model::RegistrationResult::Success) {
			logger::info(
				"Registered Test Plugin / Settings and the standalone consumer window.");
		} else if (
			result ==
			SFSEMenuFramework::Model::RegistrationResult::InterfaceUnavailable) {
			logger::warn(
				"SFSE Menu Framework is unavailable; the example remains inactive.");
		} else {
			logger::error(
				"Example panel registration failed with result {}.",
				static_cast<std::uint32_t>(result));
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
