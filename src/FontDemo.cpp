#include "FontDemo.h"

#include <SFSEMCP/SFSEMenuFramework.hpp>

#include <string>

namespace
{
	// The named-font and Font Awesome usage pattern adapts SKSE Menu Framework 3
	// Usage.md at commit 928e01ab459822a8d233ab99f0419ea1de23c775
	// (GPL-3.0) and exercises the same public push/pop contract through SFSE-MCP.
	const std::string solidIcon =
		FontAwesome::UnicodeToUtf8(0xF0E9U);
	const std::string regularIcon =
		FontAwesome::UnicodeToUtf8(0xF06EU);
	const std::string brandsIcon =
		FontAwesome::UnicodeToUtf8(0xF392U);

	void RenderNamedFonts() noexcept
	{
		ImGuiMCP::SeparatorText("Named text fonts");
		SFSEMenuFramework::PushFont("jOsT-400-bOoK");
		ImGuiMCP::TextUnformatted("Case-insensitive stem: jOsT-400-bOoK");
		FontAwesome::Pop();

		SFSEMenuFramework::PushFont("SpAcEgRoTeSk-MeDiUm.TtF");
		ImGuiMCP::TextUnformatted(
			"Case-insensitive filename: SpAcEgRoTeSk-MeDiUm.TtF");
		FontAwesome::Pop();
	}

	void RenderFontAwesome() noexcept
	{
		ImGuiMCP::SeparatorText("Font Awesome 6 Free");

		FontAwesome::PushSolid();
		ImGuiMCP::TextUnformatted(solidIcon.c_str());
		FontAwesome::Pop();
		ImGuiMCP::SameLine();
		ImGuiMCP::TextUnformatted("Solid U+F0E9");

		FontAwesome::PushRegular();
		ImGuiMCP::TextUnformatted(regularIcon.c_str());
		FontAwesome::Pop();
		ImGuiMCP::SameLine();
		ImGuiMCP::TextUnformatted("Regular U+F06E");

		FontAwesome::PushBrands();
		ImGuiMCP::TextUnformatted(brandsIcon.c_str());
		FontAwesome::Pop();
		ImGuiMCP::SameLine();
		ImGuiMCP::TextUnformatted("Brands U+F392");
	}

	void __stdcall RenderFontAPI() noexcept
	{
		ImGuiMCP::TextWrapped(
			"Font names and Font Awesome styles are selected through the same "
			"push/pop API exposed by SFSE-MCP.");
		RenderNamedFonts();
		RenderFontAwesome();

		ImGuiMCP::SeparatorText("Optional glyph coverage");
		ImGuiMCP::TextUnformatted(
			"Greek: Αθήνα  Cyrillic: Жизнь  Vietnamese: Tiếng Việt");
		ImGuiMCP::TextUnformatted(
			"Turkish: İstanbul  Thai: ภาษาไทย  Korean: 한글");
		ImGuiMCP::TextUnformatted("Japanese: 日本語  Chinese: 中文");
		ImGuiMCP::TextWrapped(
			"Each sample requires both its Framework > Settings glyph option and "
			"a selected font containing those glyphs.");
	}
}

void SFSEMenuFrameworkExample::FontDemo::Register()
{
	SFSEMenuFramework::AddSectionItem("Fonts/API", &RenderFontAPI);
}
