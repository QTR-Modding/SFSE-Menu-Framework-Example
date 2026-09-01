#include "FontDemo.h"

#include <SFSEMenuFramework/SFSEMenuFramework.h>

#include <string>

namespace
{
	// The named-font and Font Awesome usage pattern adapts SKSE Menu Framework 3
	// Usage.md at commit 928e01ab459822a8d233ab99f0419ea1de23c775
	// (GPL-3.0). The checks exercise SFSE Menu Framework's safer V5 contract.
	const std::string solidIcon =
		SFSEMenuFramework::FontAwesome::UnicodeToUtf8(0xF0E9U);
	const std::string regularIcon =
		SFSEMenuFramework::FontAwesome::UnicodeToUtf8(0xF06EU);
	const std::string brandsIcon =
		SFSEMenuFramework::FontAwesome::UnicodeToUtf8(0xF392U);

	enum class CheckResult
	{
		NotRun,
		Passed,
		Failed
	};

	CheckResult missingFontCheck{ CheckResult::NotRun };
	CheckResult unmatchedPopCheck{ CheckResult::NotRun };

	void RenderCheckResult(const char* a_label, CheckResult a_result) noexcept
	{
		if (a_result == CheckResult::NotRun) {
			ImGui::TextDisabled("%s: not run", a_label);
			return;
		}

		const bool passed = a_result == CheckResult::Passed;
		ImGui::TextColored(
			passed ? ImVec4{ 0.35F, 0.85F, 0.55F, 1.0F } :
			         ImVec4{ 1.0F, 0.35F, 0.35F, 1.0F },
			"%s: %s",
			a_label,
			passed ? "rejected as expected" : "unexpectedly accepted");
	}

	void RenderNamedFonts() noexcept
	{
		ImGui::SeparatorText("Named text fonts");
		{
			const SFSEMenuFramework::ScopedFont font{ "jOsT-400-bOoK" };
			if (font) {
				ImGui::TextUnformatted(
					"Case-insensitive stem: jOsT-400-bOoK");
			} else {
				ImGui::TextDisabled("Jost stem alias is unavailable.");
			}
		}

		{
			const SFSEMenuFramework::ScopedFont font{
				"SpAcEgRoTeSk-MeDiUm.TtF"
			};
			if (font) {
				ImGui::TextUnformatted(
					"Case-insensitive filename: SpAcEgRoTeSk-MeDiUm.TtF");
			} else {
				ImGui::TextDisabled("Space Grotesk filename alias is unavailable.");
			}
		}
	}

	void RenderFontAwesome() noexcept
	{
		ImGui::SeparatorText("Font Awesome 6 Free");

		if (SFSEMenuFramework::FontAwesome::PushSolid()) {
			ImGui::TextUnformatted(solidIcon.c_str());
			static_cast<void>(SFSEMenuFramework::FontAwesome::Pop());
		} else {
			ImGui::TextDisabled("Solid icon font unavailable (U+F0E9).");
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("Solid U+F0E9");

		if (SFSEMenuFramework::FontAwesome::PushRegular()) {
			ImGui::TextUnformatted(regularIcon.c_str());
			static_cast<void>(SFSEMenuFramework::FontAwesome::Pop());
		} else {
			ImGui::TextDisabled("Regular icon font unavailable (U+F06E).");
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("Regular U+F06E");

		if (SFSEMenuFramework::FontAwesome::PushBrands()) {
			ImGui::TextUnformatted(brandsIcon.c_str());
			static_cast<void>(SFSEMenuFramework::FontAwesome::Pop());
		} else {
			ImGui::TextDisabled("Brands icon font unavailable (U+F392).");
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("Brands U+F392");
	}

	void RenderContractChecks() noexcept
	{
		ImGui::SeparatorText("Stack contract");
		if (ImGui::Button("Test missing-font rejection")) {
			const bool pushed =
				SFSEMenuFramework::PushFont("font-that-does-not-exist.ttf");
			missingFontCheck = pushed ? CheckResult::Failed : CheckResult::Passed;
			if (pushed) {
				static_cast<void>(SFSEMenuFramework::PopFont());
			}
		}
		RenderCheckResult("Missing font", missingFontCheck);

		if (ImGui::Button("Test unmatched Pop rejection")) {
			unmatchedPopCheck = SFSEMenuFramework::PopFont() ?
				CheckResult::Failed :
				CheckResult::Passed;
		}
		RenderCheckResult("Unmatched Pop", unmatchedPopCheck);
	}

	void __stdcall RenderFontAPI() noexcept
	{
		ImGui::TextWrapped(
			"V5 resolves font names only while this consumer callback is active. "
			"ScopedFont balances each successful text-font push automatically.");
		RenderNamedFonts();
		RenderFontAwesome();
		RenderContractChecks();

		ImGui::SeparatorText("Optional glyph coverage");
		ImGui::TextUnformatted(
			"Greek: Αθήνα  Cyrillic: Жизнь  Vietnamese: Tiếng Việt");
		ImGui::TextUnformatted(
			"Turkish: İstanbul  Thai: ภาษาไทย  Korean: 한글");
		ImGui::TextUnformatted("Japanese: 日本語  Chinese: 中文");
		ImGui::TextWrapped(
			"Each sample requires both its Framework > Settings glyph option and "
			"a selected font containing those glyphs.");
	}
}

SFSEMenuFramework::Model::RegistrationResult
SFSEMenuFrameworkExample::FontDemo::Register()
{
	using Result = SFSEMenuFramework::Model::RegistrationResult;
	if (!SFSEMenuFramework::IsFontAPIAvailable()) {
		return Result::UnsupportedVersion;
	}

	return SFSEMenuFramework::AddSectionItem("Fonts/API", &RenderFontAPI);
}
