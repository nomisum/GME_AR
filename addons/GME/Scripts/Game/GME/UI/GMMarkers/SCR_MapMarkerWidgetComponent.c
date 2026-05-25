/*!
Marker widget extension — applies a faction tint to global markers so the GM
can read affiliation at a glance. There is no visibility predicate to override;
visibility is driven by SetUpdateDisabled on the marker object. Global marker
visibility is handled in SCR_MapMarkerManagerComponent.OnAddSynchedMarker.
*/
modded class SCR_MapMarkerWidgetComponent
{
	//------------------------------------------------------------------------------------------------
	//! Called from SCR_MapMarkerManagerComponent config init after SetMarkerObject.
	//! Tints the icon by the owner's faction color when the marker is global.
	void GME_ApplyFactionTint()
	{
		if (!m_MarkerObject || !m_MarkerObject.GME_IsGlobal())
			return;

		int ownerID = m_MarkerObject.GetMarkerOwnerID();
		if (ownerID < 0)
			return;

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		Faction faction = SCR_FactionManager.SGetPlayerFaction(ownerID);
		if (!faction)
			return;

		SetColor(faction.GetFactionColor());
	}
};
