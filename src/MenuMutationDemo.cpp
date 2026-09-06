#include "MenuMutationDemo.h"

#include <SFSEMCP/SFSEMenuFramework.hpp>

#include <cstdint>

namespace SFSEMenuFrameworkExample::MenuMutationDemo
{
	namespace
	{
		constexpr char controllerPath[]{
			"Test Plugin Diagnostics/Menu mutations"
		};
		constexpr char originalPath[]{
			"Test Plugin Diagnostics/Literal\\/slash mutation"
		};
		constexpr char renamedPath[]{
			"Test Plugin Diagnostics/Renamed\\/slash mutation"
		};
		constexpr char renamedSegment[]{ "Renamed\\/slash mutation" };

		const char* currentPath{ originalPath };
		const char* currentDisplayName{ "Literal/slash mutation" };
		const char* lastOperation{ "Initial duplicate registration" };
		bool targetExpected{ true };
		bool lastPrimaryResult{ true };
		bool lastSecondaryResult{};
		bool hasSecondaryResult{};
		bool oldPathChecked{};
		bool oldPathRejected{};
		std::uint64_t staleRendererFrames{};
		std::uint64_t replacementRendererFrames{};

		void __stdcall RenderStaleTarget() noexcept
		{
			++staleRendererFrames;
			ImGuiMCP::TextColored(
				ImGuiMCP::ImVec4{ 1.0F, 0.35F, 0.35F, 1.0F },
				"ERROR: the replaced renderer was called.");
		}

		void __stdcall RenderReplacementTarget() noexcept
		{
			++replacementRendererFrames;
			ImGuiMCP::TextColored(
				ImGuiMCP::ImVec4{ 0.35F, 1.0F, 0.55F, 1.0F },
				"Replacement renderer active");
			ImGuiMCP::TextUnformatted(
				"The slash in this page name is one escaped path segment.");
		}

		void RegisterOriginalTarget() noexcept
		{
			SFSEMenuFramework::FullPathAddSectionItem(
				originalPath,
				&RenderStaleTarget);
			SFSEMenuFramework::FullPathAddSectionItem(
				originalPath,
				&RenderReplacementTarget);
			currentPath = originalPath;
			currentDisplayName = "Literal/slash mutation";
			targetExpected = true;
		}

		void RecreateTarget() noexcept
		{
			(void)SFSEMenuFramework::DeleteSection(originalPath);
			(void)SFSEMenuFramework::DeleteSection(renamedPath);
			RegisterOriginalTarget();
			lastOperation = "Recreate with duplicate registration";
			lastPrimaryResult = true;
			hasSecondaryResult = false;
			oldPathChecked = false;
			oldPathRejected = false;
		}

		void RenameTarget() noexcept
		{
			lastOperation = "Rename";
			lastPrimaryResult = targetExpected &&
				SFSEMenuFramework::RenameSection(currentPath, renamedSegment);
			hasSecondaryResult = false;
			oldPathChecked = lastPrimaryResult;
			oldPathRejected = oldPathChecked &&
				!SFSEMenuFramework::DeleteSection(originalPath);
			if (lastPrimaryResult) {
				currentPath = renamedPath;
				currentDisplayName = "Renamed/slash mutation";
			}
		}

		void DeleteTarget() noexcept
		{
			lastOperation = targetExpected ? "Delete" : "Delete again";
			lastPrimaryResult = SFSEMenuFramework::DeleteSection(currentPath);
			hasSecondaryResult = false;
			if (lastPrimaryResult) {
				targetExpected = false;
			}
		}

		void RenameThenDelete() noexcept
		{
			(void)SFSEMenuFramework::DeleteSection(originalPath);
			(void)SFSEMenuFramework::DeleteSection(renamedPath);
			RegisterOriginalTarget();

			lastOperation = "Rename then delete in one callback";
			lastPrimaryResult = SFSEMenuFramework::RenameSection(
				originalPath,
				renamedSegment);
			lastSecondaryResult = lastPrimaryResult &&
				SFSEMenuFramework::DeleteSection(renamedPath);
			hasSecondaryResult = true;
			currentPath = lastPrimaryResult ? renamedPath : originalPath;
			currentDisplayName = lastPrimaryResult ?
				"Renamed/slash mutation" :
				"Literal/slash mutation";
			targetExpected = !lastSecondaryResult;
			oldPathChecked = lastPrimaryResult;
			oldPathRejected = oldPathChecked &&
				!SFSEMenuFramework::DeleteSection(originalPath);
		}

		void __stdcall RenderController() noexcept
		{
			ImGuiMCP::SeparatorText("Merged 3.8 menu API");
			ImGuiMCP::Text(
				"Framework %.1f | API %u",
				SFSEMenuFramework::GetMenuFrameworkVersion(),
				SFSEMenuFramework::GetMenuFrameworkAPIVersion());
			ImGuiMCP::Text(
				"Target: %s (%s)",
				currentDisplayName,
				targetExpected ? "present" : "deleted");

			if (ImGuiMCP::Button("Rename target")) {
				RenameTarget();
			}
			ImGuiMCP::SameLine();
			if (ImGuiMCP::Button("Delete target")) {
				DeleteTarget();
			}
			ImGuiMCP::SameLine();
			if (ImGuiMCP::Button("Recreate target")) {
				RecreateTarget();
			}

			if (ImGuiMCP::Button("Rename then delete")) {
				RenameThenDelete();
			}

			ImGuiMCP::Text(
				"Last: %s -> %s",
				lastOperation,
				lastPrimaryResult ? "true" : "false");
			if (hasSecondaryResult) {
				ImGuiMCP::SameLine();
				ImGuiMCP::Text(
					"/ %s",
					lastSecondaryResult ? "true" : "false");
			}
			ImGuiMCP::Text(
				"Old path rejected after rename: %s",
				oldPathChecked ?
					(oldPathRejected ? "yes" : "NO") :
					"not checked");
			ImGuiMCP::Text(
				"Renderer frames - replacement: %llu, stale: %llu",
				static_cast<unsigned long long>(replacementRendererFrames),
				static_cast<unsigned long long>(staleRendererFrames));
			ImGuiMCP::TextDisabled(
				"Open the slash-named target once; stale must remain zero.");
		}
	}

	void Register()
	{
		SFSEMenuFramework::FullPathAddSectionItem(
			controllerPath,
			&RenderController);
		RegisterOriginalTarget();
	}
}
