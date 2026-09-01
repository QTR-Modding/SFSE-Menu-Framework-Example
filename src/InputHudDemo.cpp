#include "InputHudDemo.h"

#include <SFSEMenuFramework/SFSEMenuFramework.h>

#include <RE/B/BSInputEventUser.h>

#include <Windows.h>

#include <atomic>
#include <cstdio>

namespace
{
	using InputRegistration = SFSEMenuFramework::Model::InputEvent;
	using HudRegistration = SFSEMenuFramework::Model::HudElement;

	InputRegistration* consumerInput{};
	InputRegistration* observerInput{};
	HudRegistration*   hudElement{};

	std::atomic<std::uint64_t> consumerEventCount{};
	std::atomic<std::uint64_t> observerEventCount{};
	std::atomic<std::uint64_t> consumedEscapeCount{};
	std::atomic<std::uint64_t> observedConsumedEscapeCount{};
	std::atomic<std::uint64_t> hudFrameCount{};
	std::atomic<bool>          consumeNextEscape{};
	thread_local RE::InputEvent* consumedEventAwaitingObserver{};

	bool inputRegistrationFailed{};
	bool hudRegistrationFailed{};

	[[nodiscard]] bool IsInitialEscapePress(const RE::InputEvent& a_event) noexcept
	{
		if (a_event.eventType != RE::InputEvent::EventType::kButton) {
			return false;
		}

		const auto& button = static_cast<const RE::ButtonEvent&>(a_event);
		// Starfield 1.16.244 stores normalized Win32 virtual-key IDs here.
		return button.deviceType == RE::InputEvent::DeviceType::kKeyboard &&
		       button.idCode == VK_ESCAPE && button.value != 0.0F &&
		       button.heldDownSecs == 0.0F;
	}

	bool __stdcall ConsumeInput(RE::InputEvent* a_event) noexcept
	{
		consumerEventCount.fetch_add(1, std::memory_order_relaxed);
		consumedEventAwaitingObserver = nullptr;
		if (!a_event || !IsInitialEscapePress(*a_event) ||
			!consumeNextEscape.exchange(false, std::memory_order_acq_rel)) {
			return false;
		}

		consumedEventAwaitingObserver = a_event;
		consumedEscapeCount.fetch_add(1, std::memory_order_relaxed);
		return true;
	}

	bool __stdcall ObserveInput(RE::InputEvent* a_event) noexcept
	{
		observerEventCount.fetch_add(1, std::memory_order_relaxed);
		if (a_event && consumedEventAwaitingObserver == a_event) {
			observedConsumedEscapeCount.fetch_add(1, std::memory_order_relaxed);
			consumedEventAwaitingObserver = nullptr;
		}
		return false;
	}

	[[nodiscard]] bool AreInputListenersActive() noexcept
	{
		return consumerInput && observerInput;
	}

	void UnregisterInputListeners() noexcept
	{
		consumeNextEscape.store(false, std::memory_order_release);
		delete consumerInput;
		consumerInput = nullptr;
		delete observerInput;
		observerInput = nullptr;
	}

	[[nodiscard]] bool RegisterInputListeners() noexcept
	{
		if (AreInputListenersActive()) {
			return true;
		}

		UnregisterInputListeners();
		consumerInput = SFSEMenuFramework::AddInputEvent(&ConsumeInput);
		if (!consumerInput) {
			return false;
		}

		observerInput = SFSEMenuFramework::AddInputEvent(&ObserveInput);
		if (!observerInput) {
			UnregisterInputListeners();
			return false;
		}
		return true;
	}

	void __stdcall RenderHud() noexcept
	{
		const auto frames =
			hudFrameCount.fetch_add(1, std::memory_order_relaxed) + 1;
		const auto input = consumerEventCount.load(std::memory_order_relaxed);
		const auto consumed = consumedEscapeCount.load(std::memory_order_relaxed);
		const auto observed =
			observedConsumedEscapeCount.load(std::memory_order_relaxed);

		char text[192]{};
		std::snprintf(
			text,
			sizeof(text),
			"SFSE-MF HUD  |  frames %llu  |  input %llu  |  Escape %llu/%llu",
			static_cast<unsigned long long>(frames),
			static_cast<unsigned long long>(input),
			static_cast<unsigned long long>(observed),
			static_cast<unsigned long long>(consumed));

		const auto& io = ImGui::GetIO();
		const auto size = ImGui::CalcTextSize(text);
		const float padding = ImGui::GetFontSize() * 0.5F;
		const ImVec2 position{
			io.DisplaySize.x - size.x - padding * 3.0F,
			padding * 2.0F
		};
		const ImVec2 backgroundMinimum{
			position.x - padding,
			position.y - padding
		};
		const ImVec2 backgroundMaximum{
			position.x + size.x + padding,
			position.y + size.y + padding
		};
		auto* const drawList = ImGui::GetForegroundDrawList();
		drawList->AddRectFilled(
			backgroundMinimum,
			backgroundMaximum,
			IM_COL32(8, 15, 26, 220),
			3.0F);
		drawList->AddRect(
			backgroundMinimum,
			backgroundMaximum,
			IM_COL32(150, 175, 200, 210),
			3.0F);
		drawList->AddText(position, IM_COL32(235, 241, 247, 255), text);
	}

	[[nodiscard]] bool RegisterHud() noexcept
	{
		if (hudElement) {
			return true;
		}
		hudElement = SFSEMenuFramework::AddHudElement(&RenderHud);
		return hudElement != nullptr;
	}

	void UnregisterHud() noexcept
	{
		delete hudElement;
		hudElement = nullptr;
	}

	void ResetCounters() noexcept
	{
		consumeNextEscape.store(false, std::memory_order_release);
		consumerEventCount.store(0, std::memory_order_relaxed);
		observerEventCount.store(0, std::memory_order_relaxed);
		consumedEscapeCount.store(0, std::memory_order_relaxed);
		observedConsumedEscapeCount.store(0, std::memory_order_relaxed);
		hudFrameCount.store(0, std::memory_order_relaxed);
	}

	void __stdcall RenderDiagnostics() noexcept
	{
		bool inputActive = AreInputListenersActive();
		if (ImGui::Checkbox("Input listeners active", &inputActive)) {
			if (inputActive) {
				inputRegistrationFailed = !RegisterInputListeners();
			} else {
				UnregisterInputListeners();
				inputRegistrationFailed = false;
			}
		}

		bool hudActive = hudElement != nullptr;
		if (ImGui::Checkbox("Persistent HUD active", &hudActive)) {
			if (hudActive) {
				hudRegistrationFailed = !RegisterHud();
			} else {
				UnregisterHud();
				hudRegistrationFailed = false;
			}
		}

		if (ImGui::Button("Arm next Escape")) {
			consumeNextEscape.store(true, std::memory_order_release);
		}
		ImGui::SameLine();
		ImGui::TextUnformatted(
			consumeNextEscape.load(std::memory_order_acquire) ? "armed" : "idle");
		ImGui::SameLine();
		if (ImGui::Button("Reset counters")) {
			ResetCounters();
		}

		const auto consumer = consumerEventCount.load(std::memory_order_relaxed);
		const auto observer = observerEventCount.load(std::memory_order_relaxed);
		const auto consumed = consumedEscapeCount.load(std::memory_order_relaxed);
		const auto observed =
			observedConsumedEscapeCount.load(std::memory_order_relaxed);
		ImGui::Text(
			"Consumer / observer events: %llu / %llu",
			static_cast<unsigned long long>(consumer),
			static_cast<unsigned long long>(observer));
		ImGui::Text(
			"Consumed Escape seen by observer: %llu / %llu",
			static_cast<unsigned long long>(observed),
			static_cast<unsigned long long>(consumed));
		ImGui::Text(
			"HUD frames: %llu",
			static_cast<unsigned long long>(
				hudFrameCount.load(std::memory_order_relaxed)));

		if (inputRegistrationFailed || hudRegistrationFailed) {
			ImGui::TextColored(
				ImVec4{ 1.0F, 0.35F, 0.35F, 1.0F },
				"Could not register the %s callback.",
				inputRegistrationFailed ? "input" : "HUD");
		}
		ImGui::TextWrapped(
			"To test consumption: arm Escape, close the MCP with F1, then press "
			"Escape after the HUD updates once. Starfield should stay in the game "
			"and the HUD should report matching observed/consumed counts.");
	}
}

SFSEMenuFramework::Model::RegistrationResult
SFSEMenuFrameworkExample::InputHudDemo::Register()
{
	using Result = SFSEMenuFramework::Model::RegistrationResult;
	if (!SFSEMenuFramework::IsInputEventAPIAvailable() ||
		!SFSEMenuFramework::IsHudElementAPIAvailable()) {
		return Result::UnsupportedVersion;
	}
	if (!RegisterInputListeners() || !RegisterHud()) {
		UnregisterHud();
		UnregisterInputListeners();
		return Result::InternalError;
	}

	const auto result = SFSEMenuFramework::AddSectionItem(
		"Input and HUD",
		&RenderDiagnostics);
	if (result != Result::Success) {
		UnregisterHud();
		UnregisterInputListeners();
	}
	return result;
}
