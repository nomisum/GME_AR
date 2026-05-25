/*!
Marker sync extension — intercepts client-side marker creation while the local
player is in the GM editor and routes it through a "global marker" RPC so the
resulting marker is visible to every faction.

STUB — concrete vanilla method signatures for marker creation must be looked up
before this can be implemented. The RPC pattern here mirrors the existing
modded SCR_EditorManagerEntity at:
  addons/GME/Scripts/Game/GME/Editor/Entities/SCR_EditorManagerEntity.c:28-78

TODOs:
1. Open vanilla SCR_MapMarkerSyncComponent.c (base addon 58D0FB3206B6F859) and
   identify the public client-side static marker creation method — likely
   `CreateStaticMarker(int type, int posX, int posZ, int rotation, int ownerID)`
   or similar. Override it.
2. Same for the dynamic-marker creation entry point.
3. In each override, gate on SCR_EditorManagerEntity.GetInstance().IsOpened().
   If true, route through GME_RpcAsk_CreateGlobalStaticMarker instead of super.
   If false (or no editor), call super unchanged.
4. Server-side RPC handler validates `mgr.IsPlayerEditor(playerID)` (or vanilla
   equivalent) before stamping global, to prevent non-GM clients from forging.
*/
modded class SCR_MapMarkerSyncComponent
{
	// TODO override the client-side create method once vanilla signature is confirmed:
	//
	// override void CreateStaticMarker(int type, int posX, int posZ, int rotation, int ownerID)
	// {
	//     SCR_EditorManagerEntity mgr = SCR_EditorManagerEntity.GetInstance();
	//     if (mgr && mgr.IsOpened())
	//     {
	//         if (Replication.IsServer())
	//             GME_DoCreateGlobalStaticMarker(type, posX, posZ, rotation, ownerID);
	//         else
	//             Rpc(GME_RpcAsk_CreateGlobalStaticMarker, type, posX, posZ, rotation, ownerID);
	//         return;
	//     }
	//     super.CreateStaticMarker(type, posX, posZ, rotation, ownerID);
	// }

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void GME_RpcAsk_CreateGlobalStaticMarker(int type, int posX, int posZ, int rotation, int ownerID)
	{
		if (!Replication.IsServer())
			return;

		// TODO server-side GM authorization check before stamping global.
		// SCR_EditorManagerCore core = SCR_EditorManagerCore.GetInstance();
		// if (!core || !core.IsPlayerEditor(ownerID)) return;

		GME_DoCreateGlobalStaticMarker(type, posX, posZ, rotation, ownerID);
	}

	//------------------------------------------------------------------------------------------------
	protected void GME_DoCreateGlobalStaticMarker(int type, int posX, int posZ, int rotation, int ownerID)
	{
		SCR_MapMarkerManagerComponent mgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!mgr)
			return;

		mgr.GME_CreateGlobalStaticMarker(type, posX, posZ, rotation, ownerID);
	}
};
