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
		bool gmActive = GME_GMMapOverlayBase.IsGmModeActive();
		Print(string.Format("[GME][SyncComp] AskAddStaticMarker | gmActive=%1", gmActive), LogLevel.WARNING);

		if (gmActive)
		{
			Print("[GME][SyncComp] Routing to GME_RpcAsk_CreateGlobalStaticMarker", LogLevel.WARNING);
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
		Print(string.Format("[GME][SyncComp][Server] GME_RpcAsk_CreateGlobalStaticMarker | playerID=%1", playerID), LogLevel.WARNING);

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core || !core.GetEditorManager(playerID))
		{
			Print(string.Format("[GME][SyncComp][Server] GM auth failed | core=%1 editorMgr=%2", core != null, core && core.GetEditorManager(playerID) != null), LogLevel.WARNING);
			return;
		}

		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!markerMgr)
		{
			Print("[GME][SyncComp][Server] markerMgr is null", LogLevel.WARNING);
			return;
		}

		markerData.GME_SetGlobal(true);
		markerData.SetMarkerOwnerID(-1);
		markerMgr.AssignMarkerUID(markerData);
		Print(string.Format("[GME][SyncComp][Server] Calling OnAddSynchedMarker | isGlobal=%1 ownerID=%2 uid=%3", markerData.GME_IsGlobal(), markerData.GetMarkerOwnerID(), markerData.GetMarkerID()), LogLevel.WARNING);
		markerMgr.OnAddSynchedMarker(markerData);
		markerMgr.OnAskAddStaticMarker(markerData);
	}
};
