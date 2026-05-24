modded class SCR_CommandToolbarEditorUIComponent : SCR_BaseToolbarEditorUIComponent
{
	protected bool m_bHighlightSubscribed;

	//------------------------------------------------------------------------------------------------
	override protected void Refresh()
	{
		super.Refresh();

		if (!m_bHighlightSubscribed && m_EditorActionsComponent)
		{
			m_EditorActionsComponent.GetOnCurrentActionChanged().Insert(RefreshHighlights);
			m_bHighlightSubscribed = true;
		}

		RefreshHighlights();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_bHighlightSubscribed && m_EditorActionsComponent)
		{
			m_EditorActionsComponent.GetOnCurrentActionChanged().Remove(RefreshHighlights);
			m_bHighlightSubscribed = false;
		}

		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	// Clicking a waypoint type button changes type and re-issues at the current waypoint's position.
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button == 0 && m_EditorActionsComponent)
		{
			SCR_BaseEditorAction action;
			if (m_Actions.Find(w, action) && action)
			{
				SCR_CommandActionsEditorComponent commandComp = SCR_CommandActionsEditorComponent.Cast(m_EditorActionsComponent);
				if (commandComp)
					commandComp.GME_ReplaceWaypointWithAction(action);
				else
					m_EditorActionsComponent.SetCurrentAction(action);

				return false;
			}
		}

		return super.OnClick(w, x, y, button);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshHighlights()
	{
		if (!m_EditorActionsComponent || !m_ItemsWidget)
			return;

		SCR_CommandActionsEditorComponent commandComp = SCR_CommandActionsEditorComponent.Cast(m_EditorActionsComponent);
		SCR_BaseEditorAction lastChosen = null;
		if (commandComp)
			lastChosen = commandComp.GME_m_LastChosenAction;

		int itemIndex;
		Widget child = m_ItemsWidget.GetChildren();
		while (child)
		{
			bool isCurrent = false;
			if (itemIndex < m_aActionData.Count())
			{
				SCR_EditorActionData actionData = m_aActionData[itemIndex];
				if (actionData)
				{
					SCR_BaseEditorAction action = actionData.GetAction();
					if (action)
						isCurrent = m_EditorActionsComponent.IsActionCurrent(action) || action == lastChosen;
				}
			}

			Widget bg = child.FindAnyWidget("Background");
			if (bg)
				bg.SetVisible(isCurrent);

			itemIndex++;
			child = child.GetSibling();
		}
	}
}
