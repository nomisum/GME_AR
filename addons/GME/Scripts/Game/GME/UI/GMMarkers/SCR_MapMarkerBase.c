/*!
GM marker extension — adds a "global" flag (visible to all factions) on top of
vanilla static markers.

Serialization mirrors the m_bIsTimestampVisible pattern: the bool is appended
after the fixed-byte blob and string in every snapshot method. Static methods
cannot call super, so the vanilla body is reproduced and the new field appended.
*/
modded class SCR_MapMarkerBase
{
	protected bool m_bGME_IsGlobal;

	//------------------------------------------------------------------------------------------------
	bool GME_IsGlobal()
	{
		return m_bGME_IsGlobal;
	}

	//------------------------------------------------------------------------------------------------
	void GME_SetGlobal(bool global)
	{
		m_bGME_IsGlobal = global;
	}

	//------------------------------------------------------------------------------------------------
	override static bool Extract(SCR_MapMarkerBase instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeInt(instance.m_iPosWorldX);
		snapshot.SerializeInt(instance.m_iPosWorldY);
		snapshot.SerializeInt(instance.m_iMarkerID);
		snapshot.SerializeInt(instance.m_iMarkerOwnerID);
		snapshot.SerializeInt(instance.m_iFlags);
		snapshot.SerializeInt(instance.m_iConfigID);
		snapshot.SerializeInt(instance.m_iFactionFlags);
		snapshot.SerializeBytes(instance.m_iRotation, 2);
		snapshot.SerializeBytes(instance.m_eType, 1);
		snapshot.SerializeBytes(instance.m_iColorEntry, 1);
		snapshot.SerializeBytes(instance.m_iIconEntry, 2);
		snapshot.SerializeString(instance.m_sCustomText);
		snapshot.SerializeBool(instance.m_bIsTimestampVisible);
		snapshot.SerializeBytes(instance.m_Timestamp, 8);
		snapshot.SerializeBool(instance.m_bGME_IsGlobal);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_MapMarkerBase instance)
	{
		snapshot.SerializeInt(instance.m_iPosWorldX);
		snapshot.SerializeInt(instance.m_iPosWorldY);
		snapshot.SerializeInt(instance.m_iMarkerID);
		snapshot.SerializeInt(instance.m_iMarkerOwnerID);
		snapshot.SerializeInt(instance.m_iFlags);
		snapshot.SerializeInt(instance.m_iConfigID);
		snapshot.SerializeInt(instance.m_iFactionFlags);
		snapshot.SerializeBytes(instance.m_iRotation, 2);
		snapshot.SerializeBytes(instance.m_eType, 1);
		snapshot.SerializeBytes(instance.m_iColorEntry, 1);
		snapshot.SerializeBytes(instance.m_iIconEntry, 2);
		snapshot.SerializeString(instance.m_sCustomText);
		snapshot.SerializeBool(instance.m_bIsTimestampVisible);
		snapshot.SerializeBytes(instance.m_Timestamp, 8);
		snapshot.SerializeBool(instance.m_bGME_IsGlobal);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.Serialize(packet, SERIALIZED_BYTES);
		snapshot.EncodeString(packet);
		snapshot.EncodeBool(packet); // m_bIsTimestampVisible
		snapshot.Serialize(packet, 8); // m_Timestamp
		snapshot.EncodeBool(packet); // m_bGME_IsGlobal
	}

	//------------------------------------------------------------------------------------------------
	override static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.Serialize(packet, SERIALIZED_BYTES);
		snapshot.DecodeString(packet);
		snapshot.DecodeBool(packet); // m_bIsTimestampVisible
		snapshot.Serialize(packet, 8); // m_Timestamp
		snapshot.DecodeBool(packet); // m_bGME_IsGlobal
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, SERIALIZED_BYTES)
			&& lhs.CompareStringSnapshots(rhs)
			&& lhs.CompareSnapshots(rhs, 4 + 8)  // m_bIsTimestampVisible + m_Timestamp
			&& lhs.CompareSnapshots(rhs, 4);      // m_bGME_IsGlobal
	}

	//------------------------------------------------------------------------------------------------
	override static bool PropCompare(SCR_MapMarkerBase instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareInt(instance.m_iPosWorldX)
			&& snapshot.CompareInt(instance.m_iPosWorldY)
			&& snapshot.CompareInt(instance.m_iMarkerID)
			&& snapshot.CompareInt(instance.m_iMarkerOwnerID)
			&& snapshot.CompareInt(instance.m_iFlags)
			&& snapshot.CompareInt(instance.m_iConfigID)
			&& snapshot.CompareInt(instance.m_iFactionFlags)
			&& snapshot.Compare(instance.m_iRotation, 2)
			&& snapshot.Compare(instance.m_eType, 1)
			&& snapshot.Compare(instance.m_iColorEntry, 1)
			&& snapshot.Compare(instance.m_iIconEntry, 2)
			&& snapshot.CompareString(instance.m_sCustomText)
			&& snapshot.CompareBool(instance.m_bIsTimestampVisible)
			&& snapshot.Compare(instance.m_Timestamp, 8)
			&& snapshot.CompareBool(instance.m_bGME_IsGlobal);
	}
};
