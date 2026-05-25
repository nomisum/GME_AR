/*!
Dynamic marker entity extension — adds a replicated "global" flag on top of
vanilla dynamic markers.

STUB — vanilla `SCR_MapMarkerEntity` is a regular entity, so we can use
`[RplProp]` directly. The TODOs below cover wiring it into the entity's normal
RplComponent and ensuring all spawn paths can stamp the flag.

TODOs:
1. Confirm vanilla SCR_MapMarkerEntity exposes an RplComponent and that
   `[RplProp]` will replicate this flag without further bookkeeping.
2. If vanilla declares the entity with custom serialization (BitSerializationSave
   / similar), update those overrides to include the flag.
3. Server-side spawn must call `GME_SetGlobal(true)` AND `Replication.BumpMe()`
   (or the relevant Rpl-bump entrypoint) so the flag propagates immediately.
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

		// TODO: confirm BumpMe is the correct propagation call for this entity's RplComponent.
		RplComponent rpl = RplComponent.Cast(FindComponent(RplComponent));
		if (rpl)
			Replication.BumpMe();
	}
};
