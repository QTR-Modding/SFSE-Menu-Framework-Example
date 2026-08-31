#include "Menu.h"

#include <SFSEMenuFramework/SFSEMenuFramework.h>

#include <atomic>
#include <cstdint>

namespace
{
	std::uint32_t count{};
	SFSEMenuFramework::Model::WindowInterface* standaloneWindow{};

	[[nodiscard]] bool IsStandaloneOpen() noexcept
	{
		return standaloneWindow &&
		       standaloneWindow->IsOpen.load(std::memory_order_acquire);
	}

	[[nodiscard]] bool IsStandaloneBlocking() noexcept
	{
		return standaloneWindow &&
		       standaloneWindow->BlockUserInput.load(std::memory_order_acquire);
	}

	void SetStandaloneOpen(bool a_open) noexcept
	{
		if (!standaloneWindow) {
			return;
		}
		if (!a_open && !SFSEMenuFramework::IsHotkeyEnabled()) {
			SFSEMenuFramework::SetHotkeyEnabled(true);
		}
		standaloneWindow->IsOpen.store(a_open, std::memory_order_release);
	}

	void SetStandaloneBlocking(bool a_blocking) noexcept
	{
		if (!standaloneWindow) {
			return;
		}
		if (!a_blocking && !SFSEMenuFramework::IsHotkeyEnabled()) {
			SFSEMenuFramework::SetHotkeyEnabled(true);
		}
		standaloneWindow->BlockUserInput.store(
			a_blocking,
			std::memory_order_release);
	}

	void RenderCounter() noexcept
	{
		ImGui::Text("Count: %u", count);
		if (ImGui::Button("Increment")) {
			++count;
		}
	}

	void RenderHotkeyControl() noexcept
	{
		bool hotkeyEnabled = SFSEMenuFramework::IsHotkeyEnabled();
		const bool safeToDisable = IsStandaloneOpen() && IsStandaloneBlocking();
		const bool disableControl = hotkeyEnabled && !safeToDisable;
		ImGui::BeginDisabled(disableControl);
		if (ImGui::Checkbox("Framework hotkey enabled", &hotkeyEnabled)) {
			SFSEMenuFramework::SetHotkeyEnabled(hotkeyEnabled);
		}
		ImGui::EndDisabled();

		if (disableControl) {
			ImGui::TextDisabled(
				"Open the blocking standalone window before disabling F1.");
		} else if (!hotkeyEnabled) {
			ImGui::TextDisabled(
				"F1 is disabled. Re-enable it here; closing this window also restores it.");
		}
	}

	void __stdcall RenderStandaloneWindow() noexcept
	{
		bool keepOpen = true;
		ImGui::SetNextWindowSize(ImVec2(640.0F, 360.0F), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(
				"SFSE Menu Framework Example Window",
				&keepOpen)) {
			ImGui::TextWrapped(
				"This resizable window is rendered by the example plugin through "
				"SFSE Menu Framework's public AddWindow API.");
			ImGui::Separator();
			RenderCounter();
			ImGui::Separator();
			ImGui::Text(
				"Blocks game input: %s",
				IsStandaloneBlocking() ? "yes" : "no");
			ImGui::Text(
				"Any blocking framework window: %s",
				SFSEMenuFramework::IsAnyBlockingWindowOpened() ? "yes" : "no");
			RenderHotkeyControl();
		}
		ImGui::End();

		if (!keepOpen) {
			SetStandaloneOpen(false);
		}
	}

	void __stdcall RenderSettings() noexcept
	{
		RenderCounter();
		ImGui::Separator();

		bool standaloneOpen = IsStandaloneOpen();
		if (ImGui::Checkbox("Standalone example window open", &standaloneOpen)) {
			SetStandaloneOpen(standaloneOpen);
		}

		bool standaloneBlocking = IsStandaloneBlocking();
		if (ImGui::Checkbox(
				"Standalone window blocks game input",
				&standaloneBlocking)) {
			SetStandaloneBlocking(standaloneBlocking);
		}

		if (standaloneOpen && !standaloneBlocking) {
			ImGui::TextDisabled(
				"Nonblocking mode stays visible after F1 closes this menu, but "
				"intentionally releases mouse and keyboard ownership.");
		}

		const auto* mainWindow = SFSEMenuFramework::GetMainWindow();
		ImGui::Text(
			"Main framework window: %s",
			mainWindow &&
					mainWindow->IsOpen.load(std::memory_order_acquire) ?
				"open" :
				"closed");
		ImGui::Text(
			"Any blocking framework window: %s",
			SFSEMenuFramework::IsAnyBlockingWindowOpened() ? "yes" : "no");
		ImGui::Separator();
		RenderHotkeyControl();
	}
}

SFSEMenuFramework::Model::RegistrationResult
SFSEMenuFrameworkExample::Menu::Register()
{
	if (!SFSEMenuFramework::IsInstalled()) {
		return SFSEMenuFramework::Model::RegistrationResult::InterfaceUnavailable;
	}
	if (!SFSEMenuFramework::GetMainWindow()) {
		return SFSEMenuFramework::Model::RegistrationResult::UnsupportedVersion;
	}
	if (!SFSEMenuFramework::SetSection("Test Plugin")) {
		return SFSEMenuFramework::Model::RegistrationResult::OutOfMemory;
	}
	if (!standaloneWindow) {
		standaloneWindow =
			SFSEMenuFramework::AddWindow(&RenderStandaloneWindow, true);
		if (!standaloneWindow) {
			return SFSEMenuFramework::Model::RegistrationResult::InternalError;
		}
	}
	return SFSEMenuFramework::AddSectionItem(
		"Settings",
		&RenderSettings);
}
