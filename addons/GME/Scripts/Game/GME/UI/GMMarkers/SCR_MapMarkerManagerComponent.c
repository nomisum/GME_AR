/*!
Map marker manager extension — three concerns:
1. GME_CreateGlobalStaticMarker: server-side helper that stamps a marker global
   and inserts it as a server marker (no owner, broadcast to all factions).
2. OnAddSynchedMarker override: global markers skip the faction filter so they
   show on all clients regardless of faction, and apply faction tint via the
   widget component.
3. RplSave/RplLoad overrides: the vanilla manual serialization path must also
   carry m_bGME_IsGlobal so late-joining clients receive the correct state.
   The snapshot pipeline (Extract/Inject/Encode/Decode) handles live replication;
   RplSave/RplLoad handles initial session join.
*/
modded class SCR_MapMarkerManagerComponent
{
	//------------------------------------------------------------------------------------------------
	//! Server-only. Inserts a static marker pre-tagged as global so the modded
	//! visibility predicate renders it for every faction.
	void GME_CreateGlobalStaticMarker(int type, int posX, int posZ, int rotation, int ownerID)
	{
		if (!Replication.IsServer())
			return;

		SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
		marker.SetType(type);
		marker.SetWorldPos(posX, posZ);
		marker.SetRotation(rotation);
		marker.SetMarkerOwnerID(ownerID);
		marker.GME_SetGlobal(true);

		InsertStaticMarker(marker, false, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Override: global markers bypass the faction filter and show on all clients.
	override void OnAddSynchedMarker(SCR_MapMarkerBase marker)
	{
		Print(string.Format("[GME][MarkerMgr] OnAddSynchedMarker | isGlobal=%1 ownerID=%2 uid=%3", marker.GME_IsGlobal(), marker.GetMarkerOwnerID(), marker.GetMarkerID()), LogLevel.WARNING);

		if (!marker.GME_IsGlobal())
		{
			super.OnAddSynchedMarker(marker);
			return;
		}

		// Global markers are always visible — skip faction check, never server-disable.
		m_aStaticMarkers.Insert(marker);

		if (System.IsConsoleApp())
			return;

		if (marker.GetMarkerOwnerID() > -1)
			marker.RequestProfanityFilter();

		SCR_MapEntity mapEnt = SCR_MapEntity.GetMapInstance();
		bool mapOpen = mapEnt && mapEnt.IsOpen();
		bool hasMarkersUI = mapOpen && mapEnt.GetMapUIComponent(SCR_MapMarkersUI) != null;
		Print(string.Format("[GME][MarkerMgr] OnAddSynchedMarker global path | mapOpen=%1 hasMarkersUI=%2", mapOpen, hasMarkersUI), LogLevel.WARNING);

		if (mapOpen && hasMarkersUI)
		{
			marker.OnCreateMarker(true);
			SCR_MapMarkerWidgetComponent widgetComp = marker.GetMarkerComponent();
			Print(string.Format("[GME][MarkerMgr] widgetComp=%1", widgetComp != null), LogLevel.WARNING);
			if (widgetComp)
				widgetComp.GME_ApplyFactionTint();
		}
	}

	//------------------------------------------------------------------------------------------------
	override event protected bool RplSave(ScriptBitWriter writer)
	{
		int count = 0;

		array<SCR_MapMarkerBase> markersSimple = GetStaticMarkers();
		foreach (SCR_MapMarkerBase markerDis : GetDisabledMarkers())
			markersSimple.Insert(markerDis);

		if (markersSimple.IsEmpty())
		{
			writer.WriteInt(count);
			return true;
		}

		foreach (SCR_MapMarkerBase marker : markersSimple)
		{
			if (marker.GetMarkerID() != -1)
				count++;
		}

		writer.WriteInt(count);

		WorldTimestamp timestamp;

		foreach (SCR_MapMarkerBase marker : markersSimple)
		{
			if (marker.GetMarkerID() == -1)
				continue;

			int pos[2];
			marker.GetWorldPos(pos);

			writer.WriteInt(pos[0]);
			writer.WriteInt(pos[1]);
			writer.WriteInt(marker.GetMarkerID());
			writer.WriteInt(marker.GetMarkerOwnerID());
			writer.WriteInt(marker.GetFlags());
			writer.WriteInt(marker.GetMarkerConfigID());
			writer.WriteInt(marker.GetMarkerFactionFlags());
			writer.Write(marker.GetRotation(), 16);
			writer.Write(marker.GetType(), 8);
			writer.Write(marker.GetColorEntry(), 8);
			writer.Write(marker.GetIconEntry(), 16);
			writer.WriteString(marker.GetCustomText());
			writer.WriteBool(marker.IsTimestampVisible());
			if (marker.IsTimestampVisible())
			{
				timestamp = marker.GetTimestamp();
				writer.Write(timestamp, 64);
			}
			writer.WriteBool(marker.GME_IsGlobal());
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override event protected bool RplLoad(ScriptBitReader reader)
	{
		int count;
		reader.ReadInt(count);
		if (count == 0)
			return true;

		int posX, posY, markerID, markerOwnerID, flags, markerConfigID, factionFlags, markerType, colorID, iconID, rotation;
		string customText;
		bool isTimestampVisible;
		bool isGlobal;
		WorldTimestamp timestamp;
		SCR_MapMarkerBase marker;
		array<string> textsToFilter = {};

		for (int i; i < count; i++)
		{
			reader.ReadInt(posX);
			reader.ReadInt(posY);
			reader.ReadInt(markerID);
			reader.ReadInt(markerOwnerID);
			reader.ReadInt(flags);
			reader.ReadInt(markerConfigID);
			reader.ReadInt(factionFlags);
			reader.Read(rotation, 16);
			reader.Read(markerType, 8);
			reader.Read(colorID, 8);
			reader.Read(iconID, 16);
			reader.ReadString(customText);
			reader.ReadBool(isTimestampVisible);
			if (isTimestampVisible)
				reader.Read(timestamp, 64);
			reader.ReadBool(isGlobal);

			marker = new SCR_MapMarkerBase();
			marker.SetType(markerType);
			marker.SetWorldPos(posX, posY);
			marker.SetMarkerID(markerID);
			marker.SetMarkerOwnerID(markerOwnerID);
			marker.SetFlags(flags);
			marker.SetMarkerConfigID(markerConfigID);
			marker.SetMarkerFactionFlags(factionFlags);
			marker.SetRotation(rotation);
			marker.SetColorEntry(colorID);
			marker.SetIconEntry(iconID);
			marker.SetCustomText(customText);
			marker.SetTimestampVisibility(isTimestampVisible);
			if (isTimestampVisible)
				marker.SetTimestamp(timestamp);
			marker.GME_SetGlobal(isGlobal);

			m_aStaticMarkers.Insert(marker);
			if (marker.GetMarkerOwnerID() > -1)
				textsToFilter.Insert(marker.GetCustomText());
		}

		if (!textsToFilter.IsEmpty())
		{
			SCR_ScriptProfanityFilterRequestCallback cb = RequestProfanityFilter(textsToFilter);
			if (cb)
				cb.m_OnResult.Insert(OnFilteredCallback);
		}

		return true;
	}
};
