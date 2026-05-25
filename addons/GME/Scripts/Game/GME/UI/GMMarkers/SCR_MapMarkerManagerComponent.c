/*!
Map marker manager extension — provides a server-side helper to insert a
marker that is pre-tagged as global (visible to all factions).

STUB — concrete vanilla create-marker entry points (names + signatures) must
be looked up before this can be implemented.

TODOs:
1. Open vanilla SCR_MapMarkerManagerComponent.c in the base editor addon
   (58D0FB3206B6F859) and identify the server-side static-marker creation
   method (likely `CreateStaticMarker(...)` or `InsertStaticMarker(...)`).
   Note its signature.
2. Implement `GME_CreateGlobalStaticMarker(...)` here that delegates to the
   vanilla create method, then calls `GME_SetGlobal(true)` on the returned
   marker BEFORE the broadcast that announces the new marker. If the broadcast
   happens inside the vanilla create method, the order must be preserved
   (either re-broadcast or insert the global tag pre-broadcast).
3. Same for dynamic markers — confirm whether dynamic markers go through this
   manager or are spawned directly as entities, and add a parallel
   `GME_CreateGlobalDynamicMarker(...)` if needed.
*/
modded class SCR_MapMarkerManagerComponent
{
	//------------------------------------------------------------------------------------------------
	//! Server-only. Inserts a static marker pre-tagged as global so the modded widget
	//! visibility predicate renders it for every faction.
	void GME_CreateGlobalStaticMarker(int type, int posX, int posZ, int rotation, int ownerID)
	{
		if (!Replication.IsServer())
			return;

		// TODO: replace with the actual vanilla server-side create call once located.
		// SCR_MapMarkerBase marker = CreateStaticMarker(type, posX, posZ, rotation, ownerID);
		// if (marker)
		//     marker.GME_SetGlobal(true);
		Print("[GME] GME_CreateGlobalStaticMarker stub — wire to vanilla create method.", LogLevel.WARNING);
	}
};
