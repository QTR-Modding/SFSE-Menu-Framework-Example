#include "Menu.h"

#include <SFSEMenuFramework/SFSEMenuFramework.h>

#include <atomic>
#include <cstdint>

namespace
{
	std::uint32_t count{};
	SFSEMenuFramework::Model::WindowInterface* standaloneWindow{};
	SFSEMenuFramework::Model::Event* lowPriorityEvent{};
	SFSEMenuFramework::Model::Event* highPriorityEvent{};
	std::atomic<std::uint64_t> openEventCount{};
	std::atomic<std::uint64_t> closeEventCount{};
	std::atomic<std::uint64_t> beforeRenderEventCount{};
	std::atomic<std::uint64_t> afterRenderEventCount{};
	std::atomic<std::uint64_t> highPriorityInvocationCount{};
	std::atomic<std::uint64_t> lowPriorityInvocationCount{};
	std::atomic<std::uint64_t> priorityOrderFailures{};
	std::atomic<std::uint32_t> lastEventType{};
	std::atomic<bool> priorityCheckArmed{};
	thread_local bool highPriorityObserved{};
	thread_local std::uint32_t highPriorityEventType{};
	bool eventRegistrationFailed{};

	[[nodiscard]] const char* GetEventName(
		SFSEMenuFramework::Model::EventType a_type) noexcept
	{
		using EventType = SFSEMenuFramework::Model::EventType;
		switch (a_type) {
		case EventType::kOpenMenu:
			return "OpenMenu";
		case EventType::kCloseMenu:
			return "CloseMenu";
		case EventType::kBeforeRender:
			return "BeforeRender";
		case EventType::kAfterRender:
			return "AfterRender";
		default:
			return "None";
		}
	}

	void __stdcall OnHighPriorityEvent(
		SFSEMenuFramework::Model::EventType a_type) noexcept
	{
		highPriorityObserved = true;
		highPriorityEventType = static_cast<std::uint32_t>(a_type);
		if (priorityCheckArmed.load(std::memory_order_acquire)) {
			highPriorityInvocationCount.fetch_add(
				1,
				std::memory_order_relaxed);
		}
		lastEventType.store(
			static_cast<std::uint32_t>(a_type),
			std::memory_order_release);

		using EventType = SFSEMenuFramework::Model::EventType;
		switch (a_type) {
		case EventType::kOpenMenu:
			openEventCount.fetch_add(1, std::memory_order_relaxed);
			break;
		case EventType::kCloseMenu:
			closeEventCount.fetch_add(1, std::memory_order_relaxed);
			break;
		case EventType::kBeforeRender:
			beforeRenderEventCount.fetch_add(1, std::memory_order_relaxed);
			break;
		case EventType::kAfterRender:
			afterRenderEventCount.fetch_add(1, std::memory_order_relaxed);
			break;
		default:
			break;
		}
	}

	void __stdcall OnLowPriorityEvent(
		SFSEMenuFramework::Model::EventType a_type) noexcept
	{
		if (!priorityCheckArmed.load(std::memory_order_acquire)) {
			if (highPriorityObserved &&
				highPriorityEventType == static_cast<std::uint32_t>(a_type)) {
				highPriorityInvocationCount.fetch_add(
					1,
					std::memory_order_relaxed);
				lowPriorityInvocationCount.fetch_add(
					1,
					std::memory_order_relaxed);
				priorityCheckArmed.store(true, std::memory_order_release);
			}
			highPriorityObserved = false;
			highPriorityEventType = 0;
			return;
		}

		lowPriorityInvocationCount.fetch_add(1, std::memory_order_relaxed);
		if (!highPriorityObserved ||
			highPriorityEventType != static_cast<std::uint32_t>(a_type)) {
			priorityOrderFailures.fetch_add(1, std::memory_order_relaxed);
		}
		highPriorityObserved = false;
		highPriorityEventType = 0;
	}

	[[nodiscard]] bool AreLifecycleEventsActive() noexcept
	{
		return lowPriorityEvent && highPriorityEvent;
	}

	void UnregisterLifecycleEvents() noexcept
	{
		priorityCheckArmed.store(false, std::memory_order_release);
		delete lowPriorityEvent;
		lowPriorityEvent = nullptr;
		delete highPriorityEvent;
		highPriorityEvent = nullptr;
		highPriorityObserved = false;
		highPriorityEventType = 0;
	}

	[[nodiscard]] bool RegisterLifecycleEvents() noexcept
	{
		if (AreLifecycleEventsActive()) {
			return true;
		}

		UnregisterLifecycleEvents();
		lowPriorityEvent = SFSEMenuFramework::AddEvent(
			&OnLowPriorityEvent,
			-100.0F);
		if (!lowPriorityEvent) {
			return false;
		}

		highPriorityEvent = SFSEMenuFramework::AddEvent(
			&OnHighPriorityEvent,
			100.0F);
		if (!highPriorityEvent) {
			UnregisterLifecycleEvents();
			return false;
		}
		highPriorityObserved = false;
		highPriorityEventType = 0;
		return true;
	}

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

	void RenderLifecycleEvents() noexcept
	{
		ImGui::SeparatorText("Lifecycle events");
		bool active = AreLifecycleEventsActive();
		if (ImGui::Checkbox("Lifecycle listeners active", &active)) {
			if (active) {
				eventRegistrationFailed = !RegisterLifecycleEvents();
			} else {
				UnregisterLifecycleEvents();
				eventRegistrationFailed = false;
			}
		}

		const auto openCount = openEventCount.load(std::memory_order_relaxed);
		const auto closeCount = closeEventCount.load(std::memory_order_relaxed);
		const auto beforeCount =
			beforeRenderEventCount.load(std::memory_order_relaxed);
		const auto afterCount =
			afterRenderEventCount.load(std::memory_order_relaxed);
		const auto orderFailures =
			priorityOrderFailures.load(std::memory_order_relaxed);
		const auto highPriorityInvocations =
			highPriorityInvocationCount.load(std::memory_order_relaxed);
		const auto lowPriorityInvocations =
			lowPriorityInvocationCount.load(std::memory_order_relaxed);
		const auto lastEvent = static_cast<SFSEMenuFramework::Model::EventType>(
			lastEventType.load(std::memory_order_acquire));

		ImGui::Text(
			"Open / close: %llu / %llu",
			static_cast<unsigned long long>(openCount),
			static_cast<unsigned long long>(closeCount));
		ImGui::Text(
			"Before / after render: %llu / %llu",
			static_cast<unsigned long long>(beforeCount),
			static_cast<unsigned long long>(afterCount));
		ImGui::Text("Last event: %s", GetEventName(lastEvent));
		if (highPriorityInvocations == 0 && lowPriorityInvocations == 0) {
			ImGui::TextDisabled(
				"Priority checks: waiting for the first complete callback pair");
		} else if (
			orderFailures == 0 &&
			highPriorityInvocations == lowPriorityInvocations) {
			ImGui::Text(
				"Priority checks: %llu paired, 0 failures",
				static_cast<unsigned long long>(highPriorityInvocations));
		} else {
			ImGui::TextColored(
				ImVec4{ 1.0F, 0.35F, 0.35F, 1.0F },
				"Priority mismatch - high: %llu, low: %llu, failures: %llu",
				static_cast<unsigned long long>(highPriorityInvocations),
				static_cast<unsigned long long>(lowPriorityInvocations),
				static_cast<unsigned long long>(orderFailures));
		}
		if (eventRegistrationFailed) {
			ImGui::TextColored(
				ImVec4{ 1.0F, 0.35F, 0.35F, 1.0F },
				"Could not register the lifecycle listeners.");
		}
		ImGui::TextDisabled(
			"Open/Close cover the main MCP only; standalone windows do not emit them.");
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
		RenderLifecycleEvents();
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
	if (!SFSEMenuFramework::IsEventAPIAvailable()) {
		return SFSEMenuFramework::Model::RegistrationResult::UnsupportedVersion;
	}
	if (!SFSEMenuFramework::GetMainWindow()) {
		return SFSEMenuFramework::Model::RegistrationResult::UnsupportedVersion;
	}
	if (!SFSEMenuFramework::SetSection("Test Plugin")) {
		return SFSEMenuFramework::Model::RegistrationResult::OutOfMemory;
	}
	if (!RegisterLifecycleEvents()) {
		return SFSEMenuFramework::Model::RegistrationResult::InternalError;
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
