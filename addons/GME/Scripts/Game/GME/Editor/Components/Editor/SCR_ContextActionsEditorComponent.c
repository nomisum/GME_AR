modded class SCR_ContextActionsEditorComponent : SCR_BaseActionsEditorComponent
{
	override void EvaluateActions(notnull array<SCR_BaseEditorAction> actions, vector cursorWorldPosition, out notnull array<ref SCR_EditorActionData> filteredActions, out int flags = 0)
	{
		SCR_CommandActionsEditorComponent commandComp = SCR_CommandActionsEditorComponent.Cast(
			SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent, true));

		if (commandComp && commandComp.GME_m_bSuppressMenu)
		{
			commandComp.GME_m_bSuppressMenu = false;
			return;
		}

		super.EvaluateActions(actions, cursorWorldPosition, filteredActions, flags);
	}
}
