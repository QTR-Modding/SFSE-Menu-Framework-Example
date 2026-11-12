#include "Menu.h"

#include <spdlog/sinks/basic_file_sink.h>

namespace
{
	void InitializeLog()
	{
		const auto directory = SFSE::GetLogDirectory();
		if (!directory) {
			return;
		}
		std::error_code error;
		std::filesystem::create_directories(*directory, error);
		if (error) {
			logger::warn("Could not create the SFSE log directory: {}", error.message());
			return;
		}
		const auto path = *directory / (std::string(PLUGIN_NAME) + ".log");
		std::shared_ptr<spdlog::logger> output;
		try {
			output = spdlog::basic_logger_mt(PLUGIN_DLL_NAME, path.string(), true);
		} catch (const spdlog::spdlog_ex& failure) {
			logger::warn("Could not open the plugin log: {}", failure.what());
			return;
		}
#ifdef NDEBUG
		output->set_level(spdlog::level::info);
		output->flush_on(spdlog::level::info);
#else
		output->set_level(spdlog::level::debug);
		output->flush_on(spdlog::level::debug);
#endif
		output->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] %v");
		spdlog::set_default_logger(std::move(output));
	}

	void OnSFSEMessage(SFSE::MessagingInterface::Message* a_message)
	{
		if (!a_message ||
			a_message->type != SFSE::MessagingInterface::kPostLoad) {
			return;
		}

		if (SFSEMenuFrameworkExample::Menu::Register()) {
			logger::info(
				"Registered the nested diagnostics, lifecycle, input, HUD, font, "
				"menu-mutation, and standalone-window examples.");
		} else {
			logger::warn(
				"SFSE Menu Framework is unavailable or rejected a required callback; "
				"the example remains inactive.");
		}
	}
}

extern "C" __declspec(dllexport) constinit SFSE::PluginVersionData SFSEPlugin_Version{
	1,
	PLUGIN_PACKED_VERSION,
	PLUGIN_NAME,
	PLUGIN_AUTHOR,
	0,
	1U,
	{ SFSE::RUNTIME_SF_1_16_244 },
	0, 0, 0
};

extern "C" __declspec(dllexport) bool SFSEPlugin_Load(const SFSE::LoadInterface* a_sfse)
{
	if (!a_sfse) {
		return false;
	}

	InitializeLog();
	if (!SFSE::Init(*a_sfse)) {
		return false;
	}

	const auto* messaging = SFSE::GetMessagingInterface();
	if (!messaging || !messaging->RegisterListener(OnSFSEMessage)) {
		logger::critical("Could not register the SFSE lifecycle listener.");
		return false;
	}

	logger::info("Initialized; waiting for SFSE post-load.");
	return true;
}
