modded class SCR_CommandActionsEditorComponent : SCR_BaseActionsEditorComponent
{
	bool GME_m_bSuppressMenu;
	SCR_BaseEditorAction GME_m_LastChosenAction;
	vector GME_m_vLastWaypointPos;

	void GME_ReplaceWaypointWithAction(SCR_BaseEditorAction action)
	{
		SCR_BaseCommandAction commandAction = SCR_BaseCommandAction.Cast(action);
		if (!commandAction)
		{
			Print("[GME] ReplaceWaypoint: action is not SCR_BaseCommandAction", LogLevel.WARNING);
			return;
		}

		Print(string.Format("[GME] ReplaceWaypoint: selectedEntities=%1", m_SelectedEntities.Count()), LogLevel.WARNING);

		foreach (SCR_EditableEntityComponent selected : m_SelectedEntities)
		{
			SCR_EditableGroupComponent group = SCR_EditableGroupComponent.Cast(selected);
			if (!group)
				group = SCR_EditableGroupComponent.Cast(selected.GetAIGroup());
			if (!group)
				continue;

			set<SCR_EditableEntityComponent> children = group.GetChildrenRef();
			int childCount = -1;
			if (children)
				childCount = children.Count();
			Print(string.Format("[GME] ReplaceWaypoint: group found, children=%1", childCount), LogLevel.WARNING);
			if (!children)
				continue;

			// Collect all waypoints and their positions
			array<vector> waypointPositions = {};
			foreach (SCR_EditableEntityComponent child : children)
			{
				SCR_EditableWaypointComponent waypoint = SCR_EditableWaypointComponent.Cast(child);
				if (!waypoint)
					continue;
				Print(string.Format("[GME] ReplaceWaypoint: waypoint found, IsCurrent=%1", waypoint.IsCurrent()), LogLevel.WARNING);
				waypointPositions.Insert(waypoint.GetOwner().GetOrigin());
			}

			Print(string.Format("[GME] ReplaceWaypoint: collected %1 waypoints", waypointPositions.Count()), LogLevel.WARNING);

			if (waypointPositions.IsEmpty())
				continue;

			// Replace each waypoint at its position with the new type
			SetCurrentAction(action);
			int evalFlags = ValidateSelection(true);
			for (int i = 0; i < waypointPositions.Count(); i++)
			{
				vector pos = waypointPositions[i];
				GME_m_vLastWaypointPos = pos;
				action.Perform(GetHoveredEntity(), m_SelectedEntities, pos, evalFlags);
			}

			GME_m_LastChosenAction = action;
			return;
		}

		Print(string.Format("[GME] ReplaceWaypoint: no group found in selectedEntities, using stored pos=%1", GME_m_vLastWaypointPos), LogLevel.WARNING);
		if (GME_m_vLastWaypointPos != vector.Zero)
		{
			SetCurrentAction(action);
			int evalFlags = ValidateSelection(true);
			action.Perform(GetHoveredEntity(), m_SelectedEntities, GME_m_vLastWaypointPos, evalFlags);
			GME_m_LastChosenAction = action;
		}
	}
	//------------------------------------------------------------------------------------------------
	override void EOnEditorActivate()
	{
		super.EOnEditorActivate();

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.AddActionListener("MouseRight", EActionTrigger.DOWN, OnRightMouseDown);
			Print("[GME] SCR_CommandActionsEditorComponent: registered MouseRight listener", LogLevel.DEBUG);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnEditorDeactivate()
	{
		super.EOnEditorDeactivate();

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.RemoveActionListener("MouseRight", EActionTrigger.DOWN, OnRightMouseDown);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRightMouseDown()
	{
		int flags = ValidateSelection(false);

		if (!SCR_Enum.HasFlag(flags, EEditorCommandActionFlags.WAYPOINT))
			return;

		SCR_EditableEntityComponent hovered = GetHoveredEntity();
		if (hovered && SCR_EditableWaypointComponent.Cast(hovered))
			return;

		bool isQueue = GetGame().GetInputManager().GetActionValue("EditorModifier") > 0;

		// Get cursor position and default action
		vector position;
		array<ref SCR_EditorActionData> actions = {};
		SCR_BaseEditorAction defaultAction = null;
		for (int i = 0, count = GetAndEvaluateActions(position, actions); i < count; i++)
		{
			SCR_BaseEditorAction action = actions[i].GetAction();
			if (m_DefaultActionGroups.IsEmpty() || m_DefaultActionGroups.Contains(action.GetActionGroup()))
			{
				defaultAction = action;
				break;
			}
		}

		// Use current action if set, otherwise use default
		SCR_BaseEditorAction actionToPerform = defaultAction;
		if (m_CurrentAction)
			actionToPerform = m_CurrentAction;
		if (!actionToPerform)
			return;

		GME_m_vLastWaypointPos = position;

		// Set current action so PerformDefaultAction uses it
		SetCurrentAction(actionToPerform);
		PerformDefaultAction(isQueue);

		// Restore after vanilla reset
		SetCurrentAction(actionToPerform);
		GME_m_LastChosenAction = actionToPerform;
		GME_m_bSuppressMenu = true;
	}
}
