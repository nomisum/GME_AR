/*!
Marker sync extension — when a GM places a marker while the editor is open,
route it through the server-marker path (ownerID = -1, no per-player limit)
so it broadcasts as global.

The intercept happens in AskAddStaticMarker (the client->server gateway called
by the manager). If the local player is in GM editor mode, we send a separate
RPC that stamps the marker global and inserts it via isServerMarker=true,
bypassing the per-player limit and owner-ID overwrite in RPC_AskAddStaticMarker.
The server-side handler validates GM authorization before accepting.
*/
modded class SCR_MapMarkerSyncComponent
{
	//------------------------------------------------------------------------------------------------
	override void AskAddStaticMarker(notnull SCR_MapMarkerBase marker)
	{
		if (GME_GMMapOverlayBase.IsGmModeActive())
		{
			Rpc(GME_RpcAsk_CreateGlobalStaticMarker, marker);
			return;
		}

		super.AskAddStaticMarker(marker);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void GME_RpcAsk_CreateGlobalStaticMarker(SCR_MapMarkerBase markerData)
	{
		int playerID = SCR_PlayerController.Cast(GetOwner()).GetPlayerId();
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core || !core.GetEditorManager(playerID))
			return;

		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!markerMgr)
			return;

		markerData.GME_SetGlobal(true);
		markerData.SetMarkerOwnerID(-1);
		markerMgr.AssignMarkerUID(markerData);
		markerMgr.OnAddSynchedMarker(markerData);
		markerMgr.OnAskAddStaticMarker(markerData);
	}
};
