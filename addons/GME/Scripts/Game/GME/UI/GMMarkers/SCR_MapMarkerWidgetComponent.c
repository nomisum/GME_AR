/*!
Marker widget extension — bypasses the per-faction visibility filter when the
local player is in GM editor mode, AND when a marker is tagged global. Also
applies a faction tint so the GM can tell at a glance who placed each marker.

STUB — vanilla `SCR_MapMarkerWidgetComponent` exposes a visibility predicate
(or wraps it in Update / OnMarkerVisibilityUpdate). The exact name must be
confirmed before this is functional.

TODOs:
1. Open vanilla SCR_MapMarkerWidgetComponent.c in the base editor addon
   (58D0FB3206B6F859) and identify the visibility predicate. Likely candidates:
     - `bool IsVisibleForPlayer(int playerID)`
     - `void OnMarkerVisibilityUpdate(...)`
     - direct branch inside `Update(float timeSlice)` based on faction match
2. Override that method; short-circuit to "visible" when:
     - SCR_EditorManagerEntity.GetInstance().IsOpened() is true (GM bypass), OR
     - the underlying marker.GME_IsGlobal() returns true.
3. Apply faction tint to the marker icon via GME_ApplyFactionTint() during
   the same update path so the GM can distinguish faction-A markers from
   faction-B markers visually.
4. The marker reference is fetched via FindHandler/marker accessor — confirm
   the method to retrieve `SCR_MapMarkerBase` from this widget component.
*/
modded class SCR_MapMarkerWidgetComponent
{
	// TODO override the visibility predicate once its name is confirmed. Sketch:
	//
	// override bool IsVisibleForPlayer(int playerID)
	// {
	//     if (GME_GMMapOverlayBase.IsGmModeActive())
	//         return true;
	//
	//     SCR_MapMarkerBase marker = GetMarker();
	//     if (marker && marker.GME_IsGlobal())
	//         return true;
	//
	//     return super.IsVisibleForPlayer(playerID);
	// }

	//------------------------------------------------------------------------------------------------
	//! Helper: tint a marker widget by its owner faction so the GM can read affiliation at a glance.
	protected void GME_ApplyFactionTint(SCR_MapMarkerBase marker)
	{
		if (!marker)
			return;

		// TODO retrieve the marker's owner-faction the way vanilla does:
		// int ownerID = marker.GetOwnerID();
		// PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(ownerID);
		// IEntity char = pc.GetControlledEntity();
		// FactionAffiliationComponent aff = FactionAffiliationComponent.Cast(char.FindComponent(FactionAffiliationComponent));
		// Faction f = aff.GetAffiliatedFaction();
		// if (f) <find icon widget>.SetColor(f.GetFactionColor());
	}
};
