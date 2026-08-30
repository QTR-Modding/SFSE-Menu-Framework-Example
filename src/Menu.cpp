#include "Menu.h"

#include <SFSEMenuFramework/SFSEMenuFramework.h>

#include <cstdint>

namespace
{
	std::uint32_t count{};

	void __stdcall RenderSettings() noexcept
	{
		ImGui::Text("Count: %u", count);
		if (ImGui::Button("Increment")) {
			++count;
		}
	}
}

SFSEMenuFramework::Model::RegistrationResult
SFSEMenuFrameworkExample::Menu::Register()
{
	if (!SFSEMenuFramework::IsInstalled()) {
		return SFSEMenuFramework::Model::RegistrationResult::InterfaceUnavailable;
	}
	if (!SFSEMenuFramework::SetSection("Test Plugin")) {
		return SFSEMenuFramework::Model::RegistrationResult::OutOfMemory;
	}

	return SFSEMenuFramework::AddSectionItem(
		"Settings",
		&RenderSettings);
}
