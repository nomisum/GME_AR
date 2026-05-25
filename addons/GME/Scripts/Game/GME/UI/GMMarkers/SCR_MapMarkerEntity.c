/*!
Dynamic marker entity extension — adds a replicated "global" flag.
[RplProp] + Replication.BumpMe() confirmed correct from vanilla SCR_MapMarkerEntity
which uses the identical pattern for m_eType, m_vPos, m_bIsGlobalVisible, etc.
No custom BitSerializationSave/Load exists, so no additional overrides needed.
*/
modded class SCR_MapMarkerEntity
{
	[RplProp()]
	protected bool m_bGME_IsGlobal;

	//------------------------------------------------------------------------------------------------
	bool GME_IsGlobal()
	{
		return m_bGME_IsGlobal;
	}

	//------------------------------------------------------------------------------------------------
	void GME_SetGlobal(bool global)
	{
		if (m_bGME_IsGlobal == global)
			return;

		m_bGME_IsGlobal = global;
		Replication.BumpMe();
	}
};
