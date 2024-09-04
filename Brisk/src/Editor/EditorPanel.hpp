#pragma once

namespace Brisk {
	class IEditorPanel {
	public:
		virtual void OnCreate() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnDestroy() = 0;
	protected:
		bool m_IsHidden;
		bool m_IsDisabled;
	};

	class HeirarchyPanel : IEditorPanel {
	public:
		virtual void OnCreate() override;
		virtual void OnUpdate() override;
		virtual void OnDestroy() override;
	};

	class ViewportPanel : IEditorPanel {
	public:
		virtual void OnCreate() override;
		virtual void OnUpdate() override;
		virtual void OnDestroy() override;
	};
}